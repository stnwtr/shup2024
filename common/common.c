#include "common.h"

void error_and_exit(void) {
    printf("[%d]: %s\n", errno, strerror(errno));
    exit(errno);
}

pid_t new_process_or_error(void) {
    pid_t pid = fork();

    if (pid == -1) {
        printf("Fehler beim Erzeugen eines neuen Prozesses!\n");
        error_and_exit();
    }

    return pid;
}

void endless(void) {
    while (true) {
        // run forever
    }
}

void print_process_info(char *name) {
    printf("%s hat PID = %d und PPID = %d\n", name, getpid(), getppid());
}

void print_process_info_with_arg(char *name, char *arg) {
    printf("%s hat PID = %d und PPID = %d mit ARG = %s\n", name, getpid(), getppid(), arg);
}

void safe_sleep(unsigned int time) {
    do {
        time = sleep(time);
    } while (time != 0);
}

void kill_or_error(pid_t pid, int status) {
    if (kill(pid, status) == -1) {
        printf("Fehler beim Beenden eines Prozesses!\n");
        error_and_exit();
    }
}

pid_t wait_or_error(int *status) {
    pid_t child = wait(status);

    if (child == -1) {
        printf("Fehler beim Warten auf einen Kind-Prozess!\n");
        error_and_exit();
    }

    return child;
}
