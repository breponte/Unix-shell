#include "utilities.h"

pid_t pidMain = -1;
uint8_t isFirstArg = 1;

uint8_t parseCommand(char** command, char** newCommand, uint8_t* state,
                    char** redirections, int* i, int* iRedirect, char*** argv,
                    int* argc) {
    uint8_t status = EXIT_SUCCESS;
    switch (*state) {
        case STATE_DEFAULT:
state_default:
            // set null delimiter between arguments
            if (**command == ' ' || **command == '\n') {
                *state = STATE_SPACES;
                *(*newCommand + *i) = '\0';
            // escape character prints next character, ignoring self
            } else if (**command == '\\') {
                *state = STATE_BACKSLASH;
                (*i)--;
            // start of double quote, searching for ending quote
            } else if (**command == '\"') {
                *state = STATE_DOUBLEQUOTE; 
                *(*newCommand + *i) = '\"';
            // start of single quote, searching for ending quote
            } else if (**command == '\'') {
                *state = STATE_SINGLEQUOTE; 
                *(*newCommand + *i) = '\'';
            // process two character redirection
            } else if ((**command == '>' && *(*command+1) == '>') ||
                    (**command == '2' && *(*command+1) == '>') ||
                    (**command == '&' && *(*command+1) == '>')) {
                *(*redirections + *iRedirect) = **command;
                (*command)++;
                *(*redirections + *iRedirect + 1) = **command;
                *(*redirections + *iRedirect + 2) = '\0';
                *iRedirect += 3;
                (*command)++;
                (*i)--;
                status = parseRedirectTarget(command, redirections, iRedirect);
            // process single character redirection
            } else if (**command == '>' || **command == '<') {
                *(*redirections + *iRedirect) = **command;
                *(*redirections + *iRedirect + 1) = '\0';
                *iRedirect += 2;
                (*command)++;
                (*i)--;
                status = parseRedirectTarget(command, redirections, iRedirect);
            // process pipe character
            } else if (**command == '|') {
                // create temp file
                FILE *temp_file = tmpfile();
                if (temp_file == NULL) {
                    fprintf(stderr, "Error making temp file for piping\n");
                    return EXIT_FAIL_TEMP_FILE_CREATE;
                }
                int fd_temp = fileno(temp_file);
                
                // fork a child with the current command
                pid_t pid = fork();
                // fork failure
                if (pid == -1) {
                    perror("fork");
                    exit(EXIT_FAIL_FORK);

                // child execution
                } else if (pid == 0) {
                    // set STDOUT to temp file
                    if (dup2(fd_temp, STDOUT) == -1) {
                        fprintf(stderr, "dup2 failed with fd:%d\n",
                            STDOUT);
                        return EXIT_FAIL_DUP2;
                    }
                    // end command parsing and move to execution
                    *(*command + 1) = '\0';
                // parent execution
                } else {
                    // set STDIN to temp file
                    if (dup2(fd_temp, STDIN) == -1) {
                        fprintf(stderr, "dup2 failed with fd:%d\n",
                            STDIN);
                        return EXIT_FAIL_DUP2;
                    }
                    // flush current command
                    *i = -1;
                    // wait on child
                    uint8_t status = join(pid);
                    if (status != EXIT_SUCCESS) {
                        fprintf(stderr, "Previous pipe command failed\n");
                        return status;
                    }
                    isFirstArg = 1;
                }
            // otherwise, print character normally
            } else {
                *(*newCommand + *i) = **command;
            }
            break;
        case STATE_BACKSLASH:
            // prints next character, regardless
            *state = STATE_DEFAULT;
            *(*newCommand + *i) = **command;
            break;
        case STATE_DOUBLEQUOTE:
            // found ending quote
            if (**command == '\"') {
                *state = STATE_DEFAULT;
                *(*newCommand + *i) = '\"';
            // otherwise, print character normally, even backslashes
            } else {
                *(*newCommand + *i) = **command;
            }
            break;
        case STATE_SINGLEQUOTE:
            // found ending quote
            if (**command == '\'') {
                *state = STATE_DEFAULT;
                *(*newCommand + *i) = '\'';
            // otherwise, print character normally, even backslashes
            } else {
                *(*newCommand + *i) = **command;
            }
            break;
        case STATE_SPACES:
            // ignore whitespace between arguments
            if (**command == ' ') {
                (*i)--;
            // new argument found, process as in default state
            } else {
                *state = STATE_DEFAULT;
                *(*argv + *argc) = *newCommand + *i;
                (*argc)++;
                goto state_default;
            }
            break;
        default:
            // unrecognized state
            fprintf(stderr, "Unrecognized state, %d\n", *state);
            return EXIT_FAIL_UNRECOGNIZED_STATE;
    }
    return status;
}

uint8_t parseRedirectTarget(char** command, char** redirectionsPtr, int* i) {
    while (**command == ' ' && **command != '\0') {
        (*command)++;
    }

    if (**command == '\0') {
        fprintf(stderr, "Redirect has broken target\n");
        return EXIT_FAIL_REDIRECT_PARSE;
    }

    uint8_t state = STATE_DEFAULT;
    while (**command != ' ' && **command != '\n' && **command != '\0') {
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
                    return EXIT_SUCCESS;
                // process single character redirection
                } else if (**command == '>' || **command == '<') {
                    *(*redirectionsPtr + *i) = '\0';
                    (*i)++;
                    (*command)--;
                    return EXIT_SUCCESS;
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
                return EXIT_FAIL_UNRECOGNIZED_STATE;
        }
        (*i)++;
        (*command)++;
    }
    *(*redirectionsPtr + *i) = '\0';
    (*i)++;

    return EXIT_SUCCESS;
}

uint8_t interpretRedirections(char* redirections, int redirectionsLength) {
    int j = 0;
    while (j < redirectionsLength) {
        // process redirection token
        int flags = 0;
        int fd_io1 = -1;
        int fd_io2 = -1;
 
        // redirects standard output, overwrites file
        if (strcmp(redirections + j, ">") == 0) {
            flags = O_CREAT | O_RDWR | O_TRUNC;
            fd_io1 = STDOUT; 
        // redirects and appends output to file
        } else if (strcmp(redirections + j, ">>") == 0) {
            flags = O_CREAT | O_RDWR | O_APPEND;
            fd_io1 = STDOUT;
        // takes input from a file instead of the keyboard
        } else if (strcmp(redirections + j, "<") == 0) {
            flags = O_RDWR;
            fd_io1 = STDIN;
        // redirects error messages to a file
        } else if (strcmp(redirections + j, "2>") == 0) {
            flags = O_CREAT | O_RDWR | O_TRUNC;
            fd_io1 = STDERR;
        // redirects both standard output and error to a file
        } else if (strcmp(redirections + j, "&>") == 0) {
            flags = O_CREAT | O_RDWR | O_TRUNC | O_APPEND;
            fd_io1 = STDOUT;
            fd_io2 = STDERR;
        } else {
            fprintf(stderr, "Unrecognized I/O redirection token: %s\n",
                redirections + j);
        }

        // iterate to redirection target
        while (*(redirections + j) != '\0') j++;
        j++;

        // check if missing target after token
        if (*(redirections + j) == '\0') {
            fprintf(stderr, "Missing target after token");
            return EXIT_FAIL_MISSING_REDIRECT;
        }

        // redirect file descriptors of stdio
        int fd_redirect = open(redirections + j, flags, 00700);
        // redirect stdio
        if (dup2(fd_redirect, fd_io1) == -1) {
            fprintf(stderr, "dup2 failed with fd:%d, flags:%d\n",
                fd_io1, flags);
            return EXIT_FAIL_DUP2;
        }
        // if redirecting stdout and stderr
        if (fd_io2 >= 0) {
            if (dup2(fd_redirect, fd_io2) == -1) {
            fprintf(stderr, "dup2 failed with fd:%d, flags:%d\n",
                fd_io2, flags);
            return EXIT_FAIL_DUP2;
            }
        }

        // iterate to next token
        while (*(redirections + j) != '\0') j++;
        j++;
    }
    return EXIT_SUCCESS;
}

uint8_t join(pid_t pid) {
 	// wait on child's termination
    int configStatus = 0;
    if (waitpid(pid, &configStatus, 0) == -1) {
        // waitpid failure
        perror("waitpid");
       return EXIT_FAIL_WAITPID;
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
    return EXIT_SUCCESS;
}
