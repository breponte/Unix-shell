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
#define EXIT_FAIL_REDIRECT_MALLOC       8
#define EXIT_FAIL_REDIRECT_PARSE        9

void processRedirectTarget(char** command, char** redirectionsPtr, int* i)
{
    while (**command == ' ' && **command != '\0') {
        (*command)++;
    }

    if (**command == '\0') {
        fprintf(stderr, "Redirect has broken target\n");
        exit(EXIT_FAIL_REDIRECT_PARSE);
    }

    uint8_t state = STATE_DEFAULT;
    while (**command != ' ' && **command != '\0') {
        switch (state) {
            case STATE_DEFAULT:
                // escape character prints next character, ignoring self
                if (**command == '\\') {
                    state = STATE_BACKSLASH;
                    (*i)--;
                // start of double quote, searching for ending quote
                } else if (**command == '\"') {
                    state = STATE_DOUBLEQUOTE; 
                    *(*redirectionsPtr + *i) = '\"';
                // start of single quote, searching for ending quote
                } else if (**command == '\'') {
                    state = STATE_SINGLEQUOTE; 
                    *(*redirectionsPtr + *i) = '\'';
                // process two character redirection
                } else if ((**command == '>' && *(*command+1) == '>') ||
                        (**command == '2' && *(*command+1) == '>') ||
                        (**command == '&' && *(*command+1) == '>')) {
                    *(*redirectionsPtr + *i) = '\0';
                    (*i)++;
                    (*command)--;
                    return;
                // process single character redirection
                } else if (**command == '>' || **command == '<') {
                    *(*redirectionsPtr + *i) = '\0';
                    (*i)++;
                    (*command)--;
                    return;
                // otherwise, print character normally
                } else {
                    *(*redirectionsPtr + *i) = **command;
                }
                break;
            case STATE_BACKSLASH:
                // prints next character, regardless
                state = STATE_DEFAULT;
                *(*redirectionsPtr + *i) = **command;
                break;
            case STATE_DOUBLEQUOTE:
                // found ending quote
                if (**command == '\"') {
                    state = STATE_DEFAULT;
                    *(*redirectionsPtr + *i) = '\"';
                // otherwise, print character normally, even backslashes
                } else {
                    *(*redirectionsPtr + *i) = **command;
                }
                break;
            case STATE_SINGLEQUOTE:
                // found ending quote
                if (**command == '\'') {
                    state = STATE_DEFAULT;
                    *(*redirectionsPtr + *i) = '\'';
                // otherwise, print character normally, even backslashes
                } else {
                    *(*redirectionsPtr + *i) = **command;
                }
                break;
            default:
                // unrecognized state
                fprintf(stderr, "Unrecognized state, %d\n", state);
                exit(EXIT_FAIL_UNRECOGNIZED_STATE);
        }
        (*i)++;
        (*command)++;
    }
    *(*redirectionsPtr + *i) = '\0';
    (*i)++;
}

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
        char** argv = malloc(bufferSize * sizeof(char *) + sizeof(char *));
        char* newCommand = malloc(bufferSize); 
        char* redirections = malloc(bufferSize);
        if (argv == NULL) {
            fprintf(stderr, "argv malloc failed.\n");
            exit(EXIT_FAIL_ARGV_MALLOC);
        }
        if (newCommand == NULL) {
            fprintf(stderr, "Command buffer malloc failed.\n");
            exit(EXIT_FAIL_CMD_BUFFER_MALLOC);
        }
        if (redirections == NULL) {
            fprintf(stderr, "Redirections buffer malloc failed.\n");
            exit(EXIT_FAIL_REDIRECT_MALLOC);
        }
        // set first argument
        *argv = newCommand;

        uint8_t state = STATE_DEFAULT;
        int i = 0;
        int iRedirect = 0;
        // iterate through user input, parsing arguments by ' ' space delimiter
        while (*command != '\0') {
            switch (state) {
                case STATE_DEFAULT:
state_default:
                    // set null delimiter between arguments
                    if (*command == ' ' || *command == '\n') {
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
                    // process two character redirection
                    } else if ((*command == '>' && *(command+1) == '>') ||
                            (*command == '2' && *(command+1) == '>') ||
                            (*command == '&' && *(command+1) == '>')) {
                        *(redirections + iRedirect) = *command;
                        command++;
                        *(redirections + iRedirect + 1) = *command;
                        *(redirections + iRedirect + 2) = '\0';
                        iRedirect += 3;
                        command++;
                        i--;
                        processRedirectTarget(&command, &redirections, &iRedirect);
                    // process single character redirection
                    } else if (*command == '>' || *command == '<') {
                        *(redirections + iRedirect) = *command;
                        *(redirections + iRedirect + 1) = '\0';
                        iRedirect += 2;
                        command++;
                        i--;
                        processRedirectTarget(&command, &redirections, &iRedirect);
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
        argv = realloc(argv, ((size_t)argc * sizeof(char *)) + sizeof(char *));
        // realloc command buffer to how many was written
        newCommand = realloc(newCommand, (size_t)i * sizeof(char) + sizeof(char));
        if (argv == NULL) {
            fprintf(stderr, "argv realloc failed.\n");
            exit(EXIT_FAIL_ARGV_REALLOC);
        }
        if (newCommand == NULL) {
            fprintf(stderr, "Command buffer realloc failed.\n");
            exit(EXIT_FAIL_CMD_BUFFER_REALLOC);
        }
        // execv expects last element to be NULL to prevent reading forever
        *(argv + argc) = NULL;
        // terminate when received exit
        if (strcmp(*argv, "exit") == 0) break;

        // redirection parsing
        for (int j = 0; j < iRedirect; j++) {
            if (*(redirections + j) != '\0')
                putchar(*(redirections + j));
            else
                putchar(' ');
        }
        putchar('\n');

        // create a child to execute specified coreutil command
        pid_t pid = fork();
        // fork failure
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAIL_FORK);

        // child's execution path
        } else if (pid == 0) {

            char* binPath = malloc(
                (strlen("./bin/") * sizeof(char)) +
                (strlen(*argv) * sizeof(char)) +
                sizeof(char)	
            );
            strcpy(binPath, "./bin/");
            strcat(binPath, *argv);
            execv(binPath, argv);
            execv(*argv, argv);

            // execl failure
            fprintf(stderr, "Failed execv(), exit number %d\n", errno);
            perror("execv");
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
    }
    return EXIT_SUCCESS;
}

int main(void)
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
