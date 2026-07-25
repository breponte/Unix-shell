#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define EXIT_SUCCESS                0
#define EXIT_FAILURE                1

#define BUFSIZE                     512

int main(int argc, char** argv)
{
    // list of flags
    // n = 2nd LSB, e/E = LSB
    uint8_t flags = 0;

    // skip program name
    argv++;
    argc--;

    // iterate through arguments until no flags are found
    while (argc > 0 && **argv == '-') {
        // iterate through this arguments flags
        uint8_t tempFlags = flags;      // do not commit bad flag argument
        char* flagArg = *argv;
        while (*(++flagArg) != '\0') {
            switch (*(flagArg)) {
                // invalid flag
                default:
                    goto print_text;
            }
        }

        // commit temporary flags
        flags = tempFlags;

        argc--;
        argv++;
    }

print_text:
    // iterate through remaining arguments and print them
    do {
        int fd = -1;
        // no program arguments, reads from STDIN
        if (argc == 0) {
            fd = dup(STDIN_FILENO);
        } else {
            fd = open(*argv, O_RDONLY);
        }

        if (fd == -1) {
            fprintf(stderr, "cat: unable to open file \"%s\"\n", *argv);
            argv++;
            argc--;
            continue;
        }

        char buf[BUFSIZE];
        ssize_t n;
        while ((n = read(fd, buf, BUFSIZE - 1)) > 0) {
            buf[n] = '\0';
            printf("%s", buf);
        }

        argv++;
        argc--;
    } while (argc > 0); 

    return EXIT_SUCCESS;
}
