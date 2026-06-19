#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int shell_loop()
{
    while (1) {}
    return 0;
}

int main(int argc, char *argv[])
{
    execl("./config.sh", "config.sh", (char *)NULL); 

    if (errno) {
        printf("FAIL\n");
        return EXIT_FAILURE;
    }
    
    printf("Herro\n");

    int status = shell_loop();

    return status;
}
