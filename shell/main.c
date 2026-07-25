#include "utilities.h"

int shell_loop() {
    uint8_t status = EXIT_SUCCESS;
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
            if (isFirstArg > 0) {
                while (*command == ' ') command++;
                isFirstArg = 0;
            }
            status = parseCommand(&command, &newCommand, &state, &redirections,
                                &i, &iRedirect, &argv, &argc);
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

        // create a child to execute specified coreutil command
        pid_t pid = fork();
        // fork failure
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAIL_FORK);

        // child's execution path
        } else if (pid == 0) {

            status = interpretRedirections(redirections, iRedirect);

            // TODO: corrupted infinite loop
            char* binPath = malloc(
                (strlen("./bin/") * sizeof(char)) +
                (strlen(*argv) * sizeof(char)) +
                sizeof(char)	
            );
            strcpy(binPath, "./bin/");
            strcat(binPath, *argv);
            execv(binPath, argv);
            // TODO: file path broken, attempts `cmd` not `./cmd`
            //execv(*argv, argv);

            // execl failure
            fprintf(stderr, "Failed execv(), exit number %d with command %s\n",
                    errno, binPath);
            perror("execv");
            _exit(127);
        }

        // parent execution from this point on...
        status = join(pid);

        // terminate child if not main thread
        if (pidMain != getpid()) _exit(0); 

        // reset file descriptors
        dup2(fd_stdin, STDIN);
        close(fd_stdin);
    }
    return status;
}

int main(void) {

    // set main thread
    pidMain = getpid();

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
   
    uint8_t status = 0;
    status = join(pid); 

    // primary shell loop
    status = shell_loop();

    return status;
}
