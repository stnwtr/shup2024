#include "../../common/common.h"

void without_param(void) {
    print_process_info("Kind");

    execlp("ls", "ls", NULL);

    printf("Fehler beim Starten eines anderen Programms!\n");
    error_and_exit();
}

void non_existent(void) {
    print_process_info("Kind");

    execlp("this program should not exist", "this program should not exist", NULL);

    printf("Fehler beim Starten eines anderen Programms!\n");
    error_and_exit();
}

void with_param(char *args[]) {
    print_process_info("Kind");

    execvp("ls", args);

    printf("Fehler beim Starten eines anderen Programms!\n");
    error_and_exit();
}

int main(int argc, char *argv[]) {
    printf("+----------------------------------------------------+\n");
    printf("| A24: Anderes Programm starten                      |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    if (argc < 2) {
        printf("Es werden mehrere Parameter benötigt!\n");
        printf("Verwendung: ./exec <o|x|n> <parameter>\n");
        return EX_USAGE;
    }

    print_process_info("Vater");

    pid_t pid = new_process_or_error();

    if (pid == 0) {
        if (str_starts_with(argv[1], "o")) {
            without_param();
        } else if (str_starts_with(argv[1], "x")) {
            non_existent();
        } else if (str_starts_with(argv[1], "n")) {
            char *args[argc];
            for (int i = 0; i < argc - 1; ++i) {
                args[i] = argv[i + 1];
            }
            args[argc - 1] = NULL;
            with_param(args);
        }
    }

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
