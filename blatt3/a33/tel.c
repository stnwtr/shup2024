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

Item pop_item(Queue *queue) {
    int out = queue->out;
    Item item = queue->inner[out];

    queue->inner[out].id = 0;
    queue->inner[out].pid = 0;
    queue->inner[out].time = 0;

    queue->out = (out + 1) % QUEUE_SIZE;

    return item;
}

void push_item(Queue *queue, Item item) {
    int in = queue->in;

    queue->inner[in].id = item.id;
    queue->inner[in].pid = item.pid;
    queue->inner[in].time = item.time;

    queue->in = (in + 1) % QUEUE_SIZE;
}

void hang_up(Item item) {
    hang_up_item->id = item.id;
    hang_up_item->pid = item.pid;
    hang_up_item->time = item.time;
}

void remove_hang_up(void) {
    hang_up_item->id = 0;
    hang_up_item->pid = 0;
    hang_up_item->time = 0;
}

void debug_print_queues() {
    printf("%lu - ACCEPT [ %d %d %d %d ] - HANG UP [ %d ] - ",
           now(),
           accept_queue->inner[0].id, accept_queue->inner[1].id,
           accept_queue->inner[2].id, accept_queue->inner[3].id,
           hang_up_item->id);
}

void advisor(int id) {
    while (true) {
        debug_print_queues();
        printf("Berater %d: Warte auf anruf\n", id);

        sem_wait(accept_sem, FULL);
        sem_wait(accept_sem, MUTEX);

        Item item = pop_item(accept_queue);
        debug_print_queues();
        printf("Berater %d: Nehme Anruf %d entgegen\n", id, item.id);

        sem_signal(accept_sem, MUTEX);
        sem_signal(accept_sem, EMPTY);

        int time = random_between(0, 60*5);
        debug_print_queues();
        printf("Berater %d: Gespräch dauert %d Sekunden\n", id, time);
        safe_sleep(time);

        sem_wait(hang_up_sem, EMPTY);
        sem_wait(hang_up_sem, MUTEX);

        debug_print_queues();
        printf("Berater %d: Anruf mit %d beendet, lege auf\n", id, item.id);
        hang_up(item);

        sem_signal(hang_up_sem, MUTEX);
        sem_signal(hang_up_sem, FULL);
    }
}

void caller(int id) {
    if (!sem_wait_nowait(accept_sem, EMPTY)) {
        debug_print_queues();
        printf("Anrufer %d: Warteschlange voll, beenden\n", id);
        exit(EAGAIN);
    }

    sem_wait(accept_sem, MUTEX);

    Item item = {id, getpid(), now()};
    push_item(accept_queue, item);
    debug_print_queues();
    printf("Anrufer %d: In Warteschlange\n", id);

    sem_signal(accept_sem, MUTEX);
    sem_signal(accept_sem, FULL);

    bool found = false;
    while (true) {
        sem_wait(hang_up_sem, FULL);
        sem_wait(hang_up_sem, MUTEX);

        for (int i = 0; i < QUEUE_SIZE; ++i) {
            if (hang_up_item->id == id) {
                debug_print_queues();
                printf("Anrufer %d: Es wurde aufgelegt, beenden\n", id);
                remove_hang_up();
                debug_print_queues();
                printf("Anrufer %d: Nach auflegen\n", id);
                found = true;
                break;
            }
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
                debug_print_queues();
                printf("NOTIFICATION: Anrufer %d wartet schon seit %lu\n", accept_queue->inner[i].id,
                       (now() - accept_queue->inner[i].time));
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

    debug_print_queues();
    printf("Beenden.\n");

    shm_detach(hang_up_item);
    del_shm(hang_up_shm);

    shm_detach(accept_queue);
    del_shm(accept_shm);

    del_sem(hang_up_sem);
    del_sem(accept_sem);

    exit(EX_OK);
}

int main(int argc, char *argv[]) {
    p=getpid();
    printf("+----------------------------------------------------+\n");
    printf("| A33: Telefon-Hotline                               |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    handle_signal_or_error(SIGINT, shutdown_handler);

    accept_sem = new_sem(IPC_PRIVATE, 3, 1, 0, QUEUE_SIZE);
    hang_up_sem = new_sem(IPC_PRIVATE, 3, 1, 0, QUEUE_SIZE);

    accept_shm = new_shm(IPC_PRIVATE, sizeof(Queue));
    accept_queue = shm_attach(accept_shm);
    accept_queue->in = 0;
    accept_queue->out = 0;

//    srand(now());

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
        int time = random_between(0, 30);
        debug_print_queues();
        printf("Anrufer %d kommt in %d Sekunden\n", i + 1, time);
        safe_sleep(time);
        if (new_process_or_error() == 0) {
            unregister_handler_or_error(SIGINT);
            caller(i + 1);
        }
    }
}
