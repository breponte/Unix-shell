#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define STATE_DEFAULT       0
#define STATE_BACKSLASH     1
#define STATE_DOUBLEQUOTE   2
#define STATE_SINGLEQUOTE   3
#define STATE_SPACES        4

int shell_loop()
{
    while (1) {
        char* command = "";
        size_t bufferSize = 0;
        getline(&command, &bufferSize, stdin);
        
        int argc = 0;
        char* newCommand = malloc(bufferSize);
        uint8_t state = STATE_DEFAULT;
        int i = 0;
        while (*command != '\0') {
            switch (state) {
                case STATE_DEFAULT:
state_default:
                    // set null delimiter between arguments
                    if (*command == ' ') {
                        state = STATE_SPACES;
                        *(newCommand + i) = '#';
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
                        goto state_default;
                    }
                    break;
                default:
                    break;
            }
            i++;
            command++;
        }
        if (newCommand) printf("Command:%s\n", newCommand);
        if (strcmp(newCommand, "exit\n") == 0) break;
    }
    return 0;
}

int main(int argc, char **argv)
{
    // create a child to execute config file
    pid_t pid = fork();

    // fork failure
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE; 
    
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
        return EXIT_FAILURE;
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
