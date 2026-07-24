#pragma once

#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#define STATE_DEFAULT                   0
#define STATE_BACKSLASH                 1
#define STATE_DOUBLEQUOTE               2
#define STATE_SINGLEQUOTE               3
#define STATE_SPACES                    4

#define EXIT_SUCCESS                    0
#define EXIT_FAIL_FORK                  1
#define EXIT_FAIL_WAITPID               2
#define EXIT_FAIL_ARGV_MALLOC           3
#define EXIT_FAIL_CMD_BUFFER_MALLOC     4
#define EXIT_FAIL_UNRECOGNIZED_STATE    5
#define EXIT_FAIL_ARGV_REALLOC          6
#define EXIT_FAIL_CMD_BUFFER_REALLOC    7
#define EXIT_FAIL_REDIRECT_MALLOC       8
#define EXIT_FAIL_REDIRECT_PARSE        9
#define EXIT_FAIL_MISSING_REDIRECT      10
#define EXIT_FAIL_DUP2                  11
#define EXIT_FAIL_TEMP_FILE_CREATE	12


#define STDIN 0
#define STDOUT 1
#define STDERR 2

uint8_t parseCommand(char** command, char** newCommand, uint8_t* state, char** redirections, int* i, int* iRedirect, char*** argv, int* argc);
uint8_t parseRedirectTarget(char** command, char** redirectionsPtr, int* i);
uint8_t interpretRedirections(char* redirections, int redirectionsLength);
uint8_t join(pid_t pid);
