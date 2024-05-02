#include "../../common/common.h"

void start() {
    print_process_info("Kind");

    execlp("lsd", "lsd", "-alFh", "--color=auto", NULL);

    printf("Fehler beim Starten eines anderen Programms!\n");
    error_and_exit();
}

int main(void) {
    printf("+----------------------------------------------------+\n");
    printf("| A24: Anderes Programm starten                      |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    print_process_info("Vater");

    pid_t pid = new_process_with_callback_or_error(start);

    int status;
    pid_t child = wait_or_error(&status);

    printf("Kind mit PID = %d erwartet (%d) - ", pid, child);

    if (WIFEXITED(status)) {
        printf("normal beendet mit Code: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("beendet durch Signal %d\n", WTERMSIG(status));
    } else {
        printf("unerwarteter Status: %d\n", status);
    }

    return EX_OK;
}
