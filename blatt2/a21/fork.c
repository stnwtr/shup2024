#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    pid_t pid = fork();

    if (pid == -1) {
        printf("Fehler beim erzeugen eines neuen Prozesses!\n");
        return 1;
    }

    if (pid == 0) {
        printf("Kind! pid = %d, ppid = %d\n", getpid(), getppid());
    } else {
        printf("Vater! pid = %d, ppid = %d\n", getpid(), getppid());
    }

    while (true);
}
