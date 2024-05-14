#ifndef COMMON_H
#define COMMON_H

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

void error_and_exit(void);

pid_t new_process_or_error(void);

pid_t new_process_with_callback_or_error(void (*)(void));

void endless(void);

void print_process_info(char *);

void print_process_info_with_arg(char *, char *);

void safe_sleep(unsigned int);

void kill_or_error(pid_t, int);

pid_t wait_or_error(int *);

void await_all_children(void);

void handle_signal_or_error(int, void (*)(int));

void unregister_handler_or_error(int);

void flush_stdout_or_error(void);

void flush_stderr_or_error(void);

void flush_stdin(void);

void read_string_from_stdin_or_error(char *, int);

bool char_is_whitespace(char);

bool str_starts_with(char *, char *);

bool str_is_blank(char *);

bool str_split(char *, char **, char *, size_t);

int str_to_int(char *);

unsigned int random_between(unsigned int, unsigned int);

time_t now(void);

// semaphores

int new_sem(key_t, int, ...);

void sem_wait(int, int);

bool sem_wait_nowait(int, int);

void sem_signal(int, int);

void del_sem(int);

// shared memory

int new_shm(key_t, size_t);

void *shm_attach(int);

void shm_detach(void *);

void del_shm(int);

#endif // COMMON_H
