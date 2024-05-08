#include "../../common/common.h"

#define BUFFER_SIZE 4
int item_loc = BUFFER_SIZE, in_loc = BUFFER_SIZE + 1, out_loc = BUFFER_SIZE + 2;

int sem, mutex, full, empty;

int shm, *buffer;

void producer(int n) {
    printf("Spawned Producer %d\n", n);

    while (true) {
        safe_sleep(1);

        sem_wait(sem, empty);
        sem_wait(sem, mutex);

        int in = buffer[in_loc];
        int item = ++buffer[item_loc];
        buffer[in] = item;
        buffer[in_loc] = (in + 1) % BUFFER_SIZE;

        printf("Producer %d produced %d\n", n, item);

        sem_signal(sem, mutex);
        sem_signal(sem, full);
    }
}

void consumer(int n) {
    printf("Spawned Consumer %d\n", n);

    while (true) {
        safe_sleep(1);

        if (sem_wait_nowait(sem, full)) {
            sem_wait(sem, mutex);

            int out = buffer[out_loc];
            int item = buffer[out];
            buffer[out_loc] = (out + 1) % BUFFER_SIZE;
            printf("Consumer %d consumed %d\n", n, item);

            sem_signal(sem, mutex);
            sem_signal(sem, empty);
        } else {
            printf("Consumer %d could not consume\n", n);
        }
    }
}

void handler() {
    printf("SIGINT handler called\n");

    del_sem(sem);
    shm_detach(buffer);
    del_shm(shm);

    exit(0);
}

int main(int argc, char *argv[]) {
    printf("+----------------------------------------------------+\n");
    printf("| A33: Telefon-Hotline                               |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    if (argc != 3) {
        printf("Usage: ./tel <prod> <cons>\n");
        exit(64);
    }

    int producer_count = str_to_int(argv[1]);
    int consumer_count = str_to_int(argv[2]);

    shm = new_shm(IPC_PRIVATE, BUFFER_SIZE * sizeof(int) + 3 * sizeof(int));
    buffer = shm_attach(shm);
    sem = new_sem(IPC_PRIVATE, 3, 1, 0, BUFFER_SIZE);
    mutex = 0;
    full = 1;
    empty = 2;

    for (int i = 0; i < producer_count; ++i) {
        if (new_process_or_error() == 0) {
            producer(i + 1);
        }
    }

    for (int i = 0; i < consumer_count; ++i) {
        if (new_process_or_error() == 0) {
            consumer(i + 1);
        }
    }

    handle_signal_or_error(SIGINT, handler);

    safe_sleep(60 * 60);

    return EX_OK;
}