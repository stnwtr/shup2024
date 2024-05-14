#include "../../common/common.h"

void child(int count, int *block) {
    for (int i = 0; i < count; ++i) {
        printf("Kind liest:     %d\n", *block);
    }
}

void parent(int count, int *block) {
    for (int i = 0; i < count; ++i) {
        unsigned int n = random_between(0, 0);
        printf("Vater schreibt: %d\n", n);
        *block = (int) n;
    }
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

    if (new_process_or_error() == 0) {
        child(count, block);
    } else {
        parent(count, block);

        wait_or_error(NULL);

        shm_detach(block);
        del_shm(shm);
    }

    return EX_OK;
}