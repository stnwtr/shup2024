#include "../../common/common.h"

#define BUFFER_SIZE 1024 // + \0
#define MAX_ARGS 32 // + self + NULL
#define MAX_PATH 64 // + NULL

void get_cwd(char *buffer, size_t len) {
    if (getcwd(buffer, len) == NULL) {
        printf("Fehler beim Lesen des aktuellen Arbeitsverzeichnisses!\n");
        error_and_exit();
    }
}

void get_path(char *buffer, size_t len) {
    char *path = getenv("PATH");
    if (path == NULL) {
        strncpy(buffer, "", len);
    } else {
        strncpy(buffer, path, len);
    }
}

void try_exec(char **args, char **path) {
    char *name = args[0];
    char executable[BUFFER_SIZE] = {0};
    bool can_exec = str_starts_with(args[0], "/") || str_starts_with(args[0], "./") || str_starts_with(args[0], "../");

    if (!can_exec) {
        int i = 0;
        char *part;

        while ((part = path[i++])) {
            sprintf(executable, "%s/%s", part, name);

            if (access(executable, F_OK | R_OK | X_OK) != -1) {
                can_exec = true;
                name = executable;
                break;
            }
        }
    }

    if (can_exec) {
        execv(name, args);
    } else {
        errno = ENOENT;
    }
}

int main(void) {
    printf("+----------------------------------------------------+\n");
    printf("| A25: Midi-Shell                                    |\n");
    printf("| Diese Lösung wurde erstellt von Simon Steinkellner |\n");
    printf("+----------------------------------------------------+\n\n");

    char cwd[BUFFER_SIZE] = {0};
    char buffer[BUFFER_SIZE] = {0};
    char raw_path[BUFFER_SIZE] = {0};
    char *args[MAX_ARGS] = {NULL};
    char *path[MAX_PATH] = {NULL};

    while (true) {
        get_cwd(cwd, BUFFER_SIZE);
        get_path(raw_path, BUFFER_SIZE);

        printf("%s> ", cwd);

        read_string_from_stdin_or_error(buffer, BUFFER_SIZE);

        if (str_is_blank(buffer)) {
            continue;
        }

        if (strcmp(buffer, "schluss") == 0) {
            printf("\nDie Shell wird beendet!\n");
            break;
        }

        if (!str_split(buffer, args, "\t\r\n\f\v ", MAX_ARGS)) {
            printf("\nDer Befehl konnte nicht ausgeführt werden: Zu viele Argumente!\n");
            continue;
        }

        if (!str_split(raw_path, path, ":", MAX_PATH)) {
            printf("\nDer Befehl konnte nicht ausgeführt werden: PATH-Variable ist zu groß!\n");
            continue;
        }

        pid_t pid = new_process_or_error();
        if (pid == 0) {
            printf("\n");
            try_exec(args, path);
            exit(errno);
        } else {
            int status;
            wait_or_error(&status);

            if (WIFEXITED(status)) {
                printf("Exit: %d - %s\n\n", WEXITSTATUS(status), strerror(WEXITSTATUS(status)));
            } else if (WIFSIGNALED(status)) {
                printf("Signal: %d\n\n", WTERMSIG(status));
            } else {
                printf("Unerwartet: %d\n\n", status);
            }
        }
    }

    return EX_OK;
}
