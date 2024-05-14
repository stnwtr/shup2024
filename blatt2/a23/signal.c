#include "../../common/common.h"

void handler(int signal) {
    static int count = 0;

    if (signal == SIGTERM) {
        ++count;
        printf("%d. Signal: SIGTERM", count);
    } else if (signal == SIGINT) {
        ++count;
        printf("%d. Signal: SIGINT", count);
    }

    if (count == 3) {
        printf(" - beendet!\n");
        exit(5);
    } else {
        printf(" - mache weiter!\n");
    }
}

int main(void) {
    printf("+----------------------------------------------------+\n");
    printf("| A23: Im Programm auf Signale reagieren             |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    printf("Programm wurde gestartet. Höre auf Signale:\n");
    printf("- SIGINT (^C): Erhöhe Zähler um 1.\n");
    printf("- SIGTERM: Erhöhe Zähler um 1.\n\n");

    print_process_info("Prozess");

    handle_signal_or_error(SIGTERM, handler);
    handle_signal_or_error(SIGINT, handler);

    endless();
}
