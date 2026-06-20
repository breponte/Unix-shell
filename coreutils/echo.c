#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
                    tempFlags |= 0x2;   // 0b0010
                    break;

                // recognize escape characters
                case 'e':
                    tempFlags |= 0x1;   // 0b0001
                    break;

                // don't recognize escape characters
                case 'E':
                    tempFlags &= 0xE;   // 0b1110
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
        putchar(' ');

        argv++;
        argc--;
    }

    return EXIT_SUCCESS;
}
