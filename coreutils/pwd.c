#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/limits.h>

#define EXIT_SUCCESS        0
#define EXIT_FAIL_GETENV    1
#define EXIT_FAIL_GETCWD    2

int main(int argc, char** argv)
{
    // logical uses PWD with potential symlinks, physical resolves all symlinks
    // default physical
    uint8_t isLogical = 0;

    // skip program name
    argv++;
    argc--;

    // iterate through arguments until no flags are found
    while (argc > 0 && **argv == '-') {
        // iterate through this arguments flags
        uint8_t tempFlags = isLogical;      // do not commit bad flag argument
        char* flagArg = *argv;
        while (*(++flagArg) != '\0') {
            switch (*(flagArg)) {
                // print symlinks
                case 'L':
                    tempFlags = 1;
                    break;

                // resolve symlinks
                case 'P':
                    tempFlags = 0;
                    break;

                // invalid flag
                default:
                    goto print_working_dir;
            }
        }

        // commit temporary flags
        isLogical = tempFlags;

        argc--;
        argv++;
    }

print_working_dir:
    char* cwd = malloc(PATH_MAX * sizeof(char));
    // uses getenv() to get $PWD environment variable
    if (isLogical) {
        if ((cwd = getenv("PWD")) == NULL) {
            fprintf(stderr, "getenv() failed\n");
            exit(EXIT_FAIL_GETENV);
        }

    // uses getcwd to get physical path with resolved symlinks
    } else {
        if (getcwd(cwd, PATH_MAX * sizeof(char)) == NULL) {
            fprintf(stderr, "getcwd failed\n");
            exit(EXIT_FAIL_GETCWD);
        }
    }
    printf("%s\n", cwd);
    return EXIT_SUCCESS;
}
