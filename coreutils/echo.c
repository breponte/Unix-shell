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
    // check next character following backslash
    switch (*(++(*textArg))) {
        case '\\': c = '\\';    break;
        case 'a' : c = '\a';    break;
        case 'b' : c = '\b';    break;
        case 'c' :  goto exit_success;
        case 'e' : c = '\x1B';  break;
        case 'f' : c = '\f';    break;
        case 'n' : c = '\n';    break;
        case 'r' : c = '\r';    break;
        case 't' : c = '\t';    break;
        case 'v' : c = '\v';    break;
        case '0' : c = '';
            // TODO: Process following 3 octal numbers if any            
            break;
        case 'x' : c = '';
            // TODO: Process following 2 hexadecimal numbers if any
            break;
        default:
            putchar(**textArg);
    }
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
            if (*textArg == '\\') c = handleEscape(&textArg);
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

exit_success:
    return EXIT_SUCCESS;
}
