#include "../../common/common.h"

#define BUFFER_SIZE 4
int item_loc = BUFFER_SIZE, in_loc = BUFFER_SIZE + 1, out_loc = BUFFER_SIZE + 2;

int mutex, full, empty;

int shm;
int *buffer;

void producer(int n) {
    printf("Spawned Producer %d\n", n);

    while (true) {
        safe_sleep(1);

        sem_wait(empty, 0);
        sem_wait(mutex, 0);

        int in = buffer[in_loc];
        int item = ++buffer[item_loc];
        buffer[in] = item;
        buffer[in_loc] = (in + 1) % BUFFER_SIZE;

        printf("Producer %d produced %d\n", n, item);

        sem_signal(mutex, 0);
        sem_signal(full, 0);
    }
}

void consumer(int n) {
    printf("Spawned Consumer %d\n", n);

    while (true) {
        safe_sleep(1);

        sem_wait(full, 0);
        sem_wait(mutex, 0);

        int out = buffer[out_loc];
        int item = buffer[out];
        buffer[out_loc] = (out + 1) % BUFFER_SIZE;
        printf("Consumer %d consumed %d\n", n, item);

        sem_signal(mutex, 0);
        sem_signal(empty, 0);
    }
}

void handler() {
    printf("SIGINT handler called\n");

    del_sem(empty);
    del_sem(full);
    del_sem(mutex);
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
    mutex = new_sem(IPC_PRIVATE, 1, 1);
    full = new_sem(IPC_PRIVATE, 1, 0);
    empty = new_sem(IPC_PRIVATE, 1, BUFFER_SIZE);

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
