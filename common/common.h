#ifndef COMMON_H
#define COMMON_H

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <unistd.h>

void error_and_exit(void);

pid_t new_process_or_error(void);

pid_t new_process_with_callback_or_error(void (*)(void));

[[noreturn]] void endless(void);

void print_process_info(char *);

void print_process_info_with_arg(char * ,char *);

void safe_sleep(unsigned int);

void kill_or_error(pid_t, int);

pid_t wait_or_error(int *);

void handle_signal_or_error(int signal, void (*)(int));

#endif // COMMON_H
