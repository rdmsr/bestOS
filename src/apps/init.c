#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{

    puts("Welcome to bestOS, starting shell..");

    char hostname[10];
    gethostname(hostname, 10);

    while (1)
    {
        char *command = malloc(256);

        printf("user@%s$ ", hostname);

        fflush(stdout);

        read(0, command, 256);

        printf("running: %s\n", command);

        free(command);
    }

    return 0;
}
