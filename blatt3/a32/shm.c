#include "../../common/common.h"

void child(int count, int *block) {
    for (int i = 0; i < count; ++i) {
        printf("Kind liest:     %d\n", *block);
        sleep(1);
    }

    shm_detach(block);
}

void parent(int shm, int count, int *block) {
    for (int i = 0; i < count; ++i) {
        unsigned int n = random_between(0, 0);
        printf("Vater schreibt: %d\n", n);
        *block = (int) n;
        sleep(1);
    }

    sleep(5);

    shm_detach(block);
    del_shm(shm);
}

int main(int argc, char *argv[]) {
    printf("+----------------------------------------------------+\n");
    printf("| A32: Shared Memory - Einstieg                      |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    if (argc != 3) {
        printf("Es werden zwei Parameter benötigt!\n");
        printf("Verwendung: ./shm <anzahl> <seed>\n");
        return EX_USAGE;
    }

    int count = str_to_int(argv[1]);
    int seed = str_to_int(argv[2]);

    int shm = new_shm(IPC_PRIVATE, sizeof(int));
    int *block = shm_attach(shm);
    srand(seed);

    pid_t pid = new_process_or_error();

    if (pid == 0) {
        child(count, block);
    } else {
        parent(shm, count, block);
    }

    return EX_OK;
}