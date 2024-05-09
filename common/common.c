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

void unregister_handler_or_error(int signal) {
    handle_signal_or_error(signal, SIG_DFL);
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

int str_to_int(char *input) {
    errno = 0;

    char *end;
    long output = strtol(input, &end, 10);

    if (errno != 0) {
        printf("Fehler bei der Konvertierung von String zu Integer!\n");
        error_and_exit();
    }

    if (end == input) {
        printf("Fehler bei der Konvertierung von String zu Integer!\n");
        errno = EINVAL;
        error_and_exit();
    }

    if (output < INT_MIN) {
        output = INT_MIN;
    }

    if (output > INT_MAX) {
        output = INT_MAX;
    }

    return (int) output;
}

int random_between(int from, int to) {
    if (from >= to) {
        return 0;
    }

    return (rand() % (to - from)) + from;
}

int new_sem(key_t key, int count, ...) {
    int sem = semget(key, count, 0666 | IPC_CREAT);

    if (sem == -1) {
        printf("Fehler beim Erzeugen einer Semaphore!\n");
        error_and_exit();
    }

    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; ++i) {
        int result = semctl(sem, i, SETVAL, va_arg(args, int));

        if (result == -1) {
            printf("Fehler beim Initialisieren einer Semaphore!\n");
            error_and_exit();
        }
    }

    va_end(args);

    return sem;
}

void sem_wait(int sem, int index) {
    struct sembuf op;
    op.sem_num = index;
    op.sem_op = -1;
    op.sem_flg = 0;

    int result = semop(sem, &op, 1);

    if (result == -1) {
        printf("Fehler beim Dekrementieren einer Semaphore!\n");
        error_and_exit();
    }
}

bool sem_wait_nowait(int sem, int index) {
    struct sembuf op;
    op.sem_num = index;
    op.sem_op = -1;
    op.sem_flg = IPC_NOWAIT;

    int result = semop(sem, &op, 1);

    if (result == -1 && errno != EAGAIN) {
        printf("Fehler beim Dekrementieren einer Semaphore mit IPC_NOWAIT!\n");
        error_and_exit();
    }

    return !(result == -1 && errno == EAGAIN);
}

void sem_signal(int sem, int index) {
    struct sembuf op;
    op.sem_num = index;
    op.sem_op = 1;
    op.sem_flg = 0;

    int result = semop(sem, &op, 1);

    if (result == -1) {
        printf("Fehler beim Inkrementieren einer Semaphore!\n");
        error_and_exit();
    }
}

void del_sem(int sem) {
    int result = semctl(sem, 0, IPC_RMID);

    if (result == -1) {
        printf("Fehler beim Löschen einer Semaphore!\n");
        error_and_exit();
    }
}

int new_shm(key_t key, size_t size) {
    int shm = shmget(key, size, 0666 | IPC_CREAT);

    if (shm == -1) {
        printf("Fehler beim Erzeugen eines Shared Memory Blocks!\n");
        error_and_exit();
    }

    return shm;
}

void *shm_attach(int shm) {
    void *block = shmat(shm, NULL, 0);

    if (block == (void *) -1) {
        printf("Fehler beim Laden eines Shared Memory Blocks!\n");
        error_and_exit();
    }

    return block;
}

void shm_detach(void *block) {
    int result = shmdt(block);

    if (result == -1) {
        printf("Fehler beim Zurückgeben eines Shared Memory Blocks!\n");
        error_and_exit();
    }
}

void del_shm(int shm) {
    int result = shmctl(shm, IPC_RMID, NULL);

    if (result == -1) {
        printf("Fehler beim Löschen eines Shared Memory Blocks!\n");
        error_and_exit();
    }
}
