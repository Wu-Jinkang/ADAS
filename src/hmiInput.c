#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "conn.h"
#include "def.h"
#include "util.h"

int main(void)
{

    int clientFd;
    char componentName[] = "hmiInput";
    clientFd = connectToServer();
    sendComponentName(clientFd, componentName);

    printf("Waiting components initialize\n");
    sleep(3);

    char input[1024];
    do
    {
        printf("Enter a command: ");

        if (scanf("%s", input) == EOF)
        {
            break;
        }

        if (strcmp(input, "INIZIO") == 0 || strcmp(input, "PARCHEGGIO") == 0 || strcmp(input, "ARRESTO") == 0)
        {
            size_t len = strlen(input);
            if (write(clientFd, input, len) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
            memset(input, 0, sizeof(input));
        }
        else
            printf("Invalid command\n");
    } while (1);

    close(clientFd);

    return 0;
}