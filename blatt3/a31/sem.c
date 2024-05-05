#include "../../common/common.h"

#define BUFFER_SIZE 1024

void parent(int sem) {
    print_process_info("Vater");

    char buffer[BUFFER_SIZE] = {0};

    printf("Vater-Benutzereingabe: ");
    read_string_from_stdin_or_error(buffer, BUFFER_SIZE);

    printf("Vater: Kind benachrichtigen\n");
    sem_signal(sem, 0);

    printf("Vater: Auf Kind warten\n");
    sem_wait(sem, 1);
    printf("Vater: Von Kind benachrichtigt\n");

    del_sem(sem);
}

void child(int sem) {
    print_process_info("Kind");

    char buffer[BUFFER_SIZE] = {0};

    printf("Kind: Auf Vater warten\n");
    sem_wait(sem, 0);
    printf("Kind: Vom Vater benachrichtigt\n");

    printf("Kind-Benutzereingabe: ");
    read_string_from_stdin_or_error(buffer, BUFFER_SIZE);

    printf("Kind: Vater benachrichtigen\n");
    sem_signal(sem, 1);
}

int main(void) {
    printf("+----------------------------------------------------+\n");
    printf("| A31: Semaphore - Einstieg                          |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    int sem = new_sem(IPC_PRIVATE, 2, 0, 0);

    pid_t pid = new_process_or_error();

    if (pid == 0) {
        child(sem);
    } else {
        parent(sem);
    }

    return EX_OK;
}