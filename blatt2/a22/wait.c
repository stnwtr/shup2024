#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void endless(void) {
    while (true) {
        // run forever
    }
}

void print_process_information(char *name) {
    printf("%s: PID = %d, PPID = %d\n", name, getpid(), getppid());
}

void print_process_information_with_arg(char *name, char *arg) {
    printf("%s: PID = %d, PPID = %d and ARG = %s\n", name, getpid(), getppid(), arg);
}

int main(int argc, char **argv) {
    printf("+----------------------------------------------------------------+\n");
    printf("| A22: Warten und beenden von Kindprozessen                      |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner             |\n");
    printf("+----------------------------------------------------------------+\n\n");

    if (argc != 5) {
        printf("Es werden vier Parameter benötigt!\n");
        printf("Verwendung: ./wait <eins> <zwei> <drei> <vier>\n");
        return EX_USAGE;
    }

    print_process_information_with_arg("Vater", argv[1]);

    pid_t a = fork();
    if (a == -1) {
        printf("Fehler beim erzeugen eines neuen Prozesses!\n");
        printf("[%d]: %s\n", errno, strerror(errno));
        return errno;
    } else if (a == 0) {
        print_process_information_with_arg("Kind A", argv[2]);
        endless();
    }

    pid_t b = fork();
    if (b == -1) {
        printf("Fehler beim erzeugen eines neuen Prozesses!\n");
        printf("[%d]: %s\n", errno, strerror(errno));
        return errno;
    } else if (b == 0) {
        print_process_information_with_arg("Kind B", argv[3]);
        endless();
    }

    pid_t c = fork();
    if (c == -1) {
        printf("Fehler beim erzeugen eines neuen Prozesses!\n");
        printf("[%d]: %s\n", errno, strerror(errno));
        return errno;
    } else if (c == 0) {
        print_process_information_with_arg("Kind C", argv[4]);
        sleep(1);
        exit(2);
    }

    sleep(2);

    kill(a, SIGTERM);
    kill(b, SIGKILL);
    kill(c, SIGKILL);

    pid_t child;
    int status;
    while ((child = wait(&status)) != -1) {
        printf("Kind mit PID = %d erwartet - ", child);
        if (WIFEXITED(status)) {
            printf("normal beendet mit Code: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("beendet durch Signal: %d\n", WTERMSIG(status));
        }
    }


    return EX_OK;
}
