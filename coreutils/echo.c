#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ENABLE_NO_NEWLINE(flags)    (flags |= 0x2)      // 0b0010
#define ENABLE_ESCAPE(flags)        (flags |= 0x1)      // 0b0001
#define DISABLE_ESCAPE(flags)       (flags &= 0xE)      // 0b1110
#define IS_NO_NEWLINE(flags)        (flags &  0x2)
#define IS_ESCAPE(flags)            (flags &  0x1)

char handleEscape(char** textArg)
{
    char c = 0;
    // check next character following backslash
    switch (*(++(*textArg))) {
        case '\\': c = '\\';    break;
        case 'a' : c = '\a';    break;
        case 'b' : c = '\b';    break;
        case 'c' : exit(EXIT_SUCCESS);
        case 'e' : c = '\x1B';  break;
        case 'f' : c = '\f';    break;
        case 'n' : c = '\n';    break;
        case 'r' : c = '\r';    break;
        case 't' : c = '\t';    break;
        case 'v' : c = '\v';    break;
        case '0' :
            // process following 3 octal numbers if
            for (int i = 0; i < 3; i++) {
                c = c << 3;
                // check if following character is an octal number
                if (*((*textArg) + 1) >= '0' && *((*textArg) + 1) <= '7') {
                    // convert char to octal
                    c |= *((*textArg) + 1) - '0';
                    (*textArg)++;
                }
                else break;
            }
            break;
        case 'x' :
            // process following 2 hexadecimal numbers if any
            for (int i = 0; i < 2; i++) {
                c = c << 4;
                // check if following character is hexadecimal number
                // check if 0-9, decimal 0-9
                if (*((*textArg) + 1) >= '0' && *((*textArg) + 1) <= '9') {
                    // convert char to hex
                    c |= *((*textArg) + 1) - '0';
                    (*textArg)++;

                // check if A-F, decimal 10-15
                } else if (*((*textArg) + 1) >= 'A' && *((*textArg) + 1) <= 'F') {
                    // convert char to hex
                    c |= *((*textArg) + 1) - 'A' + (char)10;
                    (*textArg)++;

                // check if a-f, decimal 10-15
                } else if (*((*textArg) + 1) >= 'a' && *((*textArg) + 1) <= 'f') {
                    // convert char to hex 
                    c |= *((*textArg) + 1) - 'a' + (char)10;
                    (*textArg)++;
                } else break;
            }
            break;
        default:
            c = **textArg;
    }
    return c;
}

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
            char c = *textArg;
            if (IS_ESCAPE(flags) && *textArg == '\\') c = handleEscape(&textArg);
            putchar(c);
            textArg++;
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
