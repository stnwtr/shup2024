#include "../../common/common.h"

#define ADVISOR_COUNT 3
#define QUEUE_SIZE 4

int sem, mutex = 0, full = 1, empty = 2;
int shm;
void *memory;

typedef struct {
    pid_t queue[QUEUE_SIZE];
    int in;
    int out;
} Queue;

void advisor(int n) {
    while (true) {
        sem_wait(sem, full);
        sem_wait(sem, mutex);

        Queue *queue = memory;
        int out = queue->out;
        pid_t item = queue->queue[out];
        queue->queue[out] = 0;
        queue->out = (out + 1) % QUEUE_SIZE;
        printf("Berater %d: Anrufer mit id %d angenommen\n", n, item);

        sem_signal(sem, mutex);
        sem_signal(sem, empty);

        safe_sleep(random_between(0, 6));
    }
}

void caller(int n) {
    if (sem_wait_nowait(sem, empty)) {
        sem_wait(sem, mutex);

        Queue *queue = memory;
        int in = queue->in;
        pid_t item = n; // n = getpid();
        queue->queue[in] = item;
        queue->in = (in + 1) % QUEUE_SIZE;
        printf("Anrufer %d: In Warteschlange eingereiht!\n", n);

        sem_signal(sem, mutex);
        sem_signal(sem, full);
    } else {
        printf("Anrufer %d: Warteschlange voll, beenden!\n", n);
        exit(EAGAIN);
    }

    endless();
}

void shutdown_handler() {
    printf("SIGINT-Handler!\n");

    del_sem(sem);
    shm_detach(memory);
    del_shm(shm);

    exit(0);
}

int main(int argc, char *argv[]) {
    printf("+----------------------------------------------------+\n");
    printf("| A33: Telefon-Hotline                               |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    srand(time(NULL));

    handle_signal_or_error(SIGINT, shutdown_handler);

    shm = new_shm(IPC_PRIVATE, QUEUE_SIZE * sizeof(pid_t));
    memory = shm_attach(shm);
    sem = new_sem(IPC_PRIVATE, 3, 1, 0, QUEUE_SIZE);

    // init memory
    Queue *queue = memory;
    queue->queue[0] = 0;
    queue->queue[1] = 0;
    queue->queue[2] = 0;
    queue->queue[3] = 0;
    queue->in = 0;
    queue->out = 0;

    printf("\n");

    for (int i = 0; i < ADVISOR_COUNT; ++i) {
        if (new_process_or_error() == 0) {
            unregister_handler_or_error(SIGINT);
            advisor(i + 1);
        }
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
