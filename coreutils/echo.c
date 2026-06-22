#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ENABLE_NO_NEWLINE(flags)    (flags |= 0x2)      // 0b0010
#define ENABLE_ESCAPE(flags)        (flags |= 0x1)      // 0b0001
#define DISABLE_ESCAPE(flags)       (flags &= 0xE)      // 0b1110
#define IS_NO_NEWLINE(flags)        (flags &  0x2)
#define IS_ESCAPE(flags)            (flags &  0x1)

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
                // no new line
                case 'n':
                    ENABLE_NO_NEWLINE(tempFlags);
                    break;

                // recognize escape characters
                case 'e':
                    ENABLE_ESCAPE(tempFlags);
                    break;

                // don't recognize escape characters
                case 'E':
                    DISABLE_ESCAPE(tempFlags);
                    break;

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
    while (argc > 0) {
        // iterate through current argument and character by character
        char* textArg = *argv;
        while (*textArg != '\0') {
            putchar(*textArg++);
        }
        // put space if between text arguments
        if (argc > 1) putchar(' ');

        argv++;
        argc--;
    }

    // put newline at the end according to the flag
    if (!IS_NO_NEWLINE(flags)) putchar('\n'); 

    return EXIT_SUCCESS;
}
