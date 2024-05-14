#include "../../common/common.h"

int main(void) {
    printf("+----------------------------------------------------+\n");
    printf("| A21: Abspaltung von Prozessen                      |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    if (new_process_or_error() == 0) {
        print_process_info("Kind");
    } else {
        print_process_info("Vater");
    }

    endless();
}
