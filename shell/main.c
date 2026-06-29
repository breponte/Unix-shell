#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

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

int shell_loop()
{
    while (1) {
        /**
         * https://github.com/coreutils/gnulib/blob/master/lib/getdelim.c
         * getline() uses getdelim() for '\n', realloc used so memory handling
         * is specified via char * pointer and buffer size
         */
        char* command = NULL;
        size_t bufferSize = 0;
        getline(&command, &bufferSize, stdin);
        
        // build command line arguments, malloc maximum amount of arguments
        // argc starts at one, increments on space to default state transition
        int argc = 1;
        char** argv = malloc(bufferSize * sizeof(char*));
        char* newCommand = malloc(bufferSize);
        if (argv == NULL) {
            fprintf(stderr, "argv malloc failed.\n");
            exit(EXIT_FAIL_ARGV_MALLOC);
        }
        if (newCommand == NULL) {
            fprintf(stderr, "Command buffer malloc failed.\n");
            exit(EXIT_FAIL_CMD_BUFFER_MALLOC);
        }
        // set first argument
        *argv = newCommand;

        uint8_t state = STATE_DEFAULT;
        int i = 0;
        // iterate through user input, parsing arguments by ' ' space delimiter
        while (*command != '\0') {
            switch (state) {
                case STATE_DEFAULT:
state_default:
                    // set null delimiter between arguments
                    if (*command == ' ') {
                        state = STATE_SPACES;
                        *(newCommand + i) = '\0';
                    // escape character prints next character, ignoring self
                    } else if (*command == '\\') {
                        state = STATE_BACKSLASH;
                        i--;
                    // start of double quote, searching for ending quote
                    } else if (*command == '\"') {
                        state = STATE_DOUBLEQUOTE; 
                        *(newCommand + i) = '\"';
                    // start of single quote, searching for ending quote
                    } else if (*command == '\'') {
                        state = STATE_SINGLEQUOTE; 
                        *(newCommand + i) = '\'';
                    // otherwise, print character normally
                    } else {
                        *(newCommand + i) = *command;
                    }
                    break;
                case STATE_BACKSLASH:
                    // prints next character, regardless
                    state = STATE_DEFAULT;
                    *(newCommand + i) = *command;
                    break;
                case STATE_DOUBLEQUOTE:
                    // found ending quote
                    if (*command == '\"') {
                        state = STATE_DEFAULT;
                        *(newCommand + i) = '\"';
                    // otherwise, print character normally, even backslashes
                    } else {
                        *(newCommand + i) = *command;
                    }
                    break;
                case STATE_SINGLEQUOTE:
                    // found ending quote
                    if (*command == '\'') {
                        state = STATE_DEFAULT;
                        *(newCommand + i) = '\'';
                    // otherwise, print character normally, even backslashes
                    } else {
                        *(newCommand + i) = *command;
                    }
                    break;
                case STATE_SPACES:
                    // ignore whitespace between arguments
                    if (*command == ' ') {
                        i--;
                    // new argument found, process as in default state
                    } else {
                        state = STATE_DEFAULT;
                        *(argv + argc) = newCommand + i;
                        argc++;
                        goto state_default;
                    }
                    break;
                default:
                    // unrecognized state
                    fprintf(stderr, "Unrecognized state, %d\n", state);
                    exit(EXIT_FAIL_UNRECOGNIZED_STATE);
            }
            i++;
            command++;
        }
        // realloc argv to hold as many arguments was found via argc
        argv = realloc(argv, (size_t)argc * sizeof(char *));
        // realloc command buffer to how many was written
        newCommand = realloc(newCommand, (size_t)i * sizeof(char *));
        if (argv == NULL) {
            fprintf(stderr, "argv realloc failed.\n");
            exit(EXIT_FAIL_ARGV_REALLOC);
        }
        if (newCommand == NULL) {
            fprintf(stderr, "Command buffer realloc failed.\n");
            exit(EXIT_FAIL_CMD_BUFFER_REALLOC);
        }

        printf("Parsed Command: ");
        for (int j = 0; j < argc; j++) {
            printf("%s\t", *(argv + j));
        }
        putchar('\n');
        if (strcmp(*argv, "exit") == 0) break;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    // create a child to execute config file
    pid_t pid = fork();

    // fork failure
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAIL_FORK);
    
    // child's execution path
    } else if (pid == 0) {
        execl("./shell/config.sh", "config.sh", (char *)NULL);

        // execl failure
        fprintf(stderr, "Failed execl(), exit number %d\n", errno);
        perror("execl");
        _exit(127);
    }

    // parent execution from this point on...
    
    // wait on child's termination
    int configStatus = 0;
    if (waitpid(pid, &configStatus, 0) == -1) {
        // waitpid failure
        perror("waitpid");
       exit(EXIT_FAIL_WAITPID);
    }

    // check exit status of child process
    // child exited via exit()
    if (WIFEXITED(configStatus)) {
        int code = WEXITSTATUS(configStatus);
        // child's exit code is not successful
        if (code != 0)
            fprintf(stderr, "Child failed, exit code %d\n", code);
    // child exited via signal, not successful
    } else if (WIFSIGNALED(configStatus)) {
        int sig = WTERMSIG(configStatus);
        fprintf(stderr, "Child killed by signal %d\n", sig);
    }

    // primary shell loop
    int status = shell_loop();

    return status;
}
