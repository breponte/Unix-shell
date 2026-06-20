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
    while (argc-- > 0 && **argv == '-') {
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
        flags = tempFlags;
        argv++;
    }

print_text:
    printf("Flags: %d\n", flags);
    

    /**
     * TODO: echo behavior notes
     * 1. flags can be repeated
     * 2. flags can be separate or together (i.e. -n -e vs. -ne)
     * 3. the moment a non-flag argument appears, the rest is text argument
     * 4. single spaces between space-broken text argument
     * 5. invalid flags are considered text argument by default
     * 6. flags are processed left to right (i.e. -eE vs -Ee)
     */
    return EXIT_SUCCESS;
}
