#include "common.h"

void error_and_exit(void) {
    printf("Fehler %d: %s\n", errno, strerror(errno));
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

pid_t new_process_with_callback_or_error(void (*handler)(void)) {
    pid_t pid = new_process_or_error();

    if (pid == 0) {
        handler();
    }

    return pid;
}

[[noreturn]] void endless(void) {
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

void handle_signal_or_error(int signal, void (*handler)(int)) {
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    if (sigaction(signal, &action, NULL) == -1) {
        printf("Fehler beim Registrieren eines Signal-Handlers!\n");
        error_and_exit();
    }
}

void flush_stdout_or_error(void) {
    if (fflush(stdout) == EOF) {
        printf("Fehler beim Beschreiben von STDOUT!\n");
        error_and_exit();
    }
}

void flush_stderr_or_error(void) {
    if (fflush(stderr) == EOF) {
        printf("Fehler beim Beschreiben von STDERR!\n");
        error_and_exit();
    }
}

void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // "flush" read till end
    }
}

void read_string_from_stdin_or_error(char *buffer, int len) {
    if (fgets(buffer, len, stdin) == NULL) {
        printf("Fehler beim Lesen von STDIN!\n");
        exit(EX_IOERR);
    }

    bool exceeds = true;

    for (int i = 0; i < len; ++i) {
        if (buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == EOF) { // EOF?
            buffer[i] = 0;
            exceeds = false;
            break;
        }
    }

    if (exceeds) {
        flush_stdin();
    }
}

bool char_is_whitespace(char c) {
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '\v':
        case '\f':
            return true;
        default:
            return false;
    }
}

bool str_starts_with(char *buffer, char *prefix) {
    return strncmp(buffer, prefix, strlen(prefix)) == 0;
}

bool str_is_blank(char *buffer) {
    unsigned long len = strlen(buffer);

    for (int i = 0; i < len; ++i) {
        if (!char_is_whitespace(buffer[i])) {
            return false;
        }
    }

    return true;
}

bool str_split(char *source, char **destination, char *delimiter, size_t max) {
    char *token = strtok(source, delimiter);
    int count = 0;

    while (token) {
        if (count == max - 1) {
            return false;
        }

        destination[count++] = token;
        token = strtok(NULL, delimiter);
    }

    destination[count] = NULL;

    return true;
}
