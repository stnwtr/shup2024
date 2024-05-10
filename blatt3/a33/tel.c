#include "../../common/common.h"

#define ADVISOR_COUNT 3
#define QUEUE_SIZE 4

int sem, mutex = 0, full = 1, empty = 2;
int sem2, mutex2 = 0, full2 = 1, empty2 = 2;
int shm;
void *memory;
int shm2;
void *memory2;

typedef struct {
    pid_t pid;
    time_t time;
    time_t accessed;
} Item;

typedef struct {
    Item queue[QUEUE_SIZE];
    int in;
    int out;
} Queue;

void advisor(int n) {
    while (true) {
        sem_wait(sem, full);
        sem_wait(sem, mutex);

        Queue *queue = memory;
        int out = queue->out;
        Item item = queue->queue[out];
        queue->queue[out].time = 0;
        queue->queue[out].pid = 0;
        queue->queue[out].accessed = 0;
        queue->out = (out + 1) % QUEUE_SIZE;
        printf("Berater %d: Anrufer mit id %d angenommen\n", n, item.pid);

        sem_signal(sem, mutex);
        sem_signal(sem, empty);

        sem_wait(sem2, empty2);
        sem_wait(sem2, mutex2);

        Queue *queue2 = memory2;
        int in = queue2->in;
        queue2->queue[in].pid = item.pid;
        queue2->queue[in].time = now();
        queue2->queue[in].accessed = queue2->queue[in].time;
        queue2->in = (in + 1) % QUEUE_SIZE;

        sem_signal(sem2, mutex2);
        safe_sleep(random_between(0, 6));
        printf("Sending kill to id = %d\n", item.pid);
        sem_signal(sem2, full2);
    }
}

void call_finish_handler(int n){
    printf("Got killed with id = %d\n", n);
    exit(EX_OK);
}

void caller(int n) {
    if (sem_wait_nowait(sem, empty)) {
        sem_wait(sem, mutex);

        Queue *queue = memory;
        int in = queue->in;
        queue->queue[in].pid = n; // getpid();
        queue->queue[in].time = now();
        queue->queue[in].accessed = queue->queue[in].time;
        queue->in = (in + 1) % QUEUE_SIZE;
        printf("Anrufer %d: In Warteschlange eingereiht!\n", n);

        sem_signal(sem, mutex);
        sem_signal(sem, full);

        while (true) {
            printf("Wait for death check %d\n", n);
            sem_wait(sem2, full2);
            sem_wait(sem2, mutex2);
            Queue *queue2 = memory2;
            bool found = false;
            for (int i = 0; i < QUEUE_SIZE; ++i) {
                if (queue2->queue[i].pid == n) {
                    printf("found! %d\n", n);
                    found = true;
                    break;
                }
            }
            sem_signal(sem2, mutex2);
            sem_signal(sem2, empty2);

            if (found) {
                call_finish_handler(n);
            }
        }
    } else {
        printf("Anrufer %d: Warteschlange voll, beenden!\n", n);
        exit(EAGAIN);
    }
}

void broadcaster() {
    while (true) {
        sem_wait(sem, mutex);

        Queue *queue = memory;
        for (int i = 0; i < QUEUE_SIZE; ++i) {
            if (queue->queue[i].pid != 0 && (now() - queue->queue[i].accessed) >= 1) {
                printf("Anrufer mit ID %d wartet seit %lu (%lu Sekunden)!\n",
                       queue->queue[i].pid, queue->queue[i].time, now() - queue->queue[i].time);
                queue->queue[i].accessed = now();
            }
        }

        sem_signal(sem, mutex);

        safe_sleep(1);
    }
}

void shutdown_handler() {
    del_sem(sem);
    del_sem(sem2);
    shm_detach(memory);
    del_shm(shm);
    shm_detach(memory2);
    del_shm(shm2);

    exit(0);
}

int main(int argc, char *argv[]) {
    printf("+----------------------------------------------------+\n");
    printf("| A33: Telefon-Hotline                               |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    srand(now());

    handle_signal_or_error(SIGINT, shutdown_handler);

    shm = new_shm(IPC_PRIVATE, sizeof(Queue));
    memory = shm_attach(shm);
    shm2 = new_shm(IPC_PRIVATE, sizeof(Queue));
    memory2 = shm_attach(shm2);
    sem = new_sem(IPC_PRIVATE, 3, 1, 0, QUEUE_SIZE);
    sem2 = new_sem(IPC_PRIVATE, 3, 1, 0, QUEUE_SIZE);

    // init memory
    Queue *queue = memory;
    queue->in = 0;
    queue->out = 0;

    printf("\n");

    for (int i = 0; i < ADVISOR_COUNT; ++i) {
        if (new_process_or_error() == 0) {
            unregister_handler_or_error(SIGINT);
            advisor(i + 1);
        }
    }

    if (new_process_or_error() == 0) {
        unregister_handler_or_error(SIGINT);
        broadcaster();
    }

    int i = 0;
    while (true) {
        safe_sleep(random_between(0, 2));

        i++;
        if (new_process_or_error() == 0) {
            unregister_handler_or_error(SIGINT);
            caller(i);
        }
    }
}
