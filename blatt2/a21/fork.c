#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void endless(void) {
    while (true) {
        // run forever
    }
}

void print_process_information(char *name) {
    printf("%s: PID = %d, PPID = %d\n", name, getpid(), getppid());
}

int main(void) {
    printf("+----------------------------------------------------+\n");
    printf("| A21: Abspaltung von Prozessen                      |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    pid_t pid = fork();

    if (pid == -1) {
        printf("Fehler beim erzeugen eines neuen Prozesses!\n");
        printf("[%d]: %s\n", errno, strerror(errno));
        return 1;
    }

    if (pid == 0) {
        print_process_information("Kind");
    } else {
        print_process_information("Vater");
    }

    endless();
}
