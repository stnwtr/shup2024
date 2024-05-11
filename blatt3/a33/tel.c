#include "../../common/common.h"

#define QUEUE_SIZE 4

#define MUTEX 0
#define FULL 1
#define EMPTY 2

typedef struct {
    int id;
    pid_t pid;
    time_t time;
} Item;

typedef struct {
    Item inner[QUEUE_SIZE];
    int in;
    int out;
} Queue;

int accept_sem;
int hang_up_sem;

int accept_shm;
Queue *accept_queue;

int hang_up_shm;
Item *hang_up_item;

Item pop_item(void) {
    int out = accept_queue->out;
    Item item = accept_queue->inner[out];

    accept_queue->inner[out].id = 0;
    accept_queue->inner[out].pid = 0;
    accept_queue->inner[out].time = 0;
    accept_queue->out = (out + 1) % QUEUE_SIZE;

    return item;
}

void push_item(Item item) {
    int in = accept_queue->in;

    accept_queue->inner[in].id = item.id;
    accept_queue->inner[in].pid = item.pid;
    accept_queue->inner[in].time = item.time;
    accept_queue->in = (in + 1) % QUEUE_SIZE;
}

void hang_up(Item item) {
    hang_up_item->id = item.id;
    hang_up_item->pid = item.pid;
    hang_up_item->time = item.time;
}

void clear_hang_up(void) {
    hang_up_item->id = 0;
    hang_up_item->pid = 0;
    hang_up_item->time = 0;
}

void print_log_prefix(void) {
    time_t a = now();
    struct tm *b = localtime(&a);
    int size = 64;
    char buffer[size];
    strftime(buffer, size, "[%F %T] ", b);
    printf("%s", buffer);
}

void advisor(int id) {
    while (true) {
        print_log_prefix();
        printf("Berater %d: Warten auf Anruf.\n", id);
        sem_wait(accept_sem, FULL);
        sem_wait(accept_sem, MUTEX);

        Item item = pop_item();
        print_log_prefix();
        printf("Berater %d: Anruf mit %d angenommen.\n", id, item.id);

        sem_signal(accept_sem, MUTEX);
        sem_signal(accept_sem, EMPTY);

        unsigned int time = random_between(0, 60 * 5);
        print_log_prefix();
        printf("Berater %d: Anruf wird %d Sekunden dauern.\n", id, time);
        safe_sleep(time);

        sem_wait(hang_up_sem, EMPTY);
        sem_wait(hang_up_sem, MUTEX);

        print_log_prefix();
        printf("Berater %d: Anruf mit %d vorbei, lege auf.\n", id, item.id);
        hang_up(item);

        sem_signal(hang_up_sem, MUTEX);
        sem_signal(hang_up_sem, FULL);
    }
}

void caller(int id) {
    print_log_prefix();
    printf("Anrufer %d: Rufe an.\n", id);
    if (!sem_wait_nowait(accept_sem, EMPTY)) {
        print_log_prefix();
        printf("Anrufer %d: Die Hotline ist zur Zeit überlastet. Versuchen Sie es später noch einmal.\n", id);
        exit(EAGAIN);
    }
    sem_wait(accept_sem, MUTEX);

    Item item = {id, getpid(), now()};
    push_item(item);
    print_log_prefix();
    printf("Anrufer %d: In Warteschlange.\n", id);

    sem_signal(accept_sem, MUTEX);
    sem_signal(accept_sem, FULL);

    print_log_prefix();
    printf("Anrufer %d: Warte bis Gesprächsende.\n", id);
    bool found = false;
    while (true) {
        sem_wait(hang_up_sem, FULL);
        sem_wait(hang_up_sem, MUTEX);

        if (hang_up_item->id == id) {
            print_log_prefix();
            printf("Anrufer %d: Es wurde aufgelegt, beenden.\n", id);
            clear_hang_up();
            found = true;
        }

        sem_signal(hang_up_sem, MUTEX);

        if (found) {
            sem_signal(hang_up_sem, EMPTY);
            break;
        } else {
            sem_signal(hang_up_sem, FULL);
        }
    }

    exit(EX_OK);
}

void notificator() {
    while (true) {
        sem_wait(accept_sem, MUTEX);

        for (int i = 0; i < QUEUE_SIZE; ++i) {
            if (accept_queue->inner[i].id != 0 && (now() - accept_queue->inner[i].time) >= 1 * 60) {
                print_log_prefix();
                printf("Anrufer %d: Bitte warten Sie, bis ein Beratungsplatz frei ist.\n", accept_queue->inner[i].id);
                accept_queue->inner[i].time = now();
            }
        }

        sem_signal(accept_sem, MUTEX);

        safe_sleep(1);
    }
}

int p;

void shutdown_handler() {
    if (p != getpid()) {
        return;
    }

    print_log_prefix();
    printf("Beenden.\n");

    shm_detach(hang_up_item);
    del_shm(hang_up_shm);

    shm_detach(accept_queue);
    del_shm(accept_shm);

    del_sem(hang_up_sem);
    del_sem(accept_sem);

    exit(EX_OK);
}

int main(void) {
    p = getpid();
    print_log_prefix();
    printf("+----------------------------------------------------+\n");
    print_log_prefix();
    printf("| A33: Telefon-Hotline                               |\n");
    print_log_prefix();
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    print_log_prefix();
    printf("+----------------------------------------------------+\n\n");

    handle_signal_or_error(SIGINT, shutdown_handler);

    accept_sem = new_sem(IPC_PRIVATE, 3, 1, 0, QUEUE_SIZE);
    hang_up_sem = new_sem(IPC_PRIVATE, 3, 1, 0, QUEUE_SIZE);

    accept_shm = new_shm(IPC_PRIVATE, sizeof(Queue));
    accept_queue = shm_attach(accept_shm);
    accept_queue->in = 0;
    accept_queue->out = 0;

    hang_up_shm = new_shm(IPC_PRIVATE, sizeof(Item));
    hang_up_item = shm_attach(hang_up_shm);

    for (int i = 0; i < 3; ++i) {
        if (new_process_or_error() == 0) {
            unregister_handler_or_error(SIGINT);
            advisor(i + 1);
        }
    }

    if (new_process_or_error() == 0) {
        unregister_handler_or_error(SIGINT);
        notificator();
    }

    for (int i = 0; true; ++i) {
        unsigned int time = random_between(0, 2 * 60);
        print_log_prefix();
        printf("Anrufer %d: Kommt in %d Sekunden.\n", i + 1, time);
        safe_sleep(time);
        if (new_process_or_error() == 0) {
            unregister_handler_or_error(SIGINT);
            caller(i + 1);
        }
    }
}
