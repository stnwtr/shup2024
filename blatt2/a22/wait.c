#include "../../common/common.h"

int main(int argc, char **argv) {
    printf("+----------------------------------------------------+\n");
    printf("| A22: Warten und beenden von Kindprozessen          |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    if (argc != 5) {
        printf("Es werden vier Parameter benötigt!\n");
        printf("Verwendung: ./wait <eins> <zwei> <drei> <vier>\n");
        return EX_USAGE;
    }

    print_process_info_with_arg("Vater", argv[1]);

    pid_t a = new_process_or_error();
    if (a == 0) {
        print_process_info_with_arg("Kind A", argv[2]);
        endless();
    }

    pid_t b = new_process_or_error();
    if (b == 0) {
        print_process_info_with_arg("Kind B", argv[3]);
        endless();
    }

    pid_t c = new_process_or_error();
    if (c == 0) {
        print_process_info_with_arg("Kind C", argv[4]);
        safe_sleep(1);
        exit(2);
    }

    safe_sleep(2);

    kill_or_error(a, SIGTERM);
    kill_or_error(b, SIGKILL);
    kill_or_error(c, SIGKILL);

    for (int i = 0; i < 3; ++i) {
        int status;
        pid_t child = wait_or_error(&status);

        char *name;

        if (child == a) {
            name = "Kind A";
        } else if (child == b) {
            name = "Kind B";
        } else if (child == c) {
            name = "Kind C";
        } else {
            printf("Ungültige Kind-Prozess-ID von wait empfangen!\n");
            exit(EX_SOFTWARE);
        }

        printf("%s mit PID = %d erwartet - ", name, child);

        if (WIFEXITED(status)) {
            printf("normal beendet mit Code: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("beendet durch Signal: %d\n", WTERMSIG(status));
        } else {
            printf("ungültiger Status: %d\n", status);
        }
    }

    return EX_OK;
}
