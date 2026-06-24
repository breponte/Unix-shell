#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int shell_loop()
{
    while (1) {
        char* command = "";
        size_t bufferSize = 0;
        getline(&command, &bufferSize, stdin);
        
        int argc = 0;
        char* current = command;
        char prevState = '\0';
        while (*current != '\0') {
            // TODO: Parse command line arguments into an array of strings, removing whitespace
            // TODO: Handle quotes with spaces
            switch (*current) {
                case ' ':
                    if (prevState != '\0') *current = '\0';
                    break;
                case '\\':
                    if (prevState != '\0') *current = '\\';
                    break;
                case '\'':
                    if (prevState != '\'') prevState = '\'';
                    break;
                case '\"':
                    if (prevState != '\"') prevState = '\"';
                    break;
                default:
                    break;
            }

            current++;
        }
        if (command) printf("Command:%s\n", command);
        if (strcmp(command, "exit\n") == 0) break;
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
