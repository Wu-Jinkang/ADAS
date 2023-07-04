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

int main(int argc, char *argv[])
{
    int clientFd;
    char componentName[] = "steerByWire";
    clientFd = connectToServer();
    sendComponentName(clientFd, componentName);
    fcntl(clientFd, F_GETFL, O_NONBLOCK);

    char str[1024], printStr[1024];
    int logFd, c, repeat;

    logFd = open(STEER_LOG, O_WRONLY);
    if (logFd == -1)
    {
        perror("open steer log");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        memset(str, 0, sizeof str);

        readLine(clientFd, str);
        repeat = 1;

        if (strcmp(str, "SINISTRA") == 0)
        {
            repeat = 4;
            strcpy(printStr, "STO GIRANDO A SINISTRA");
        }
        else if (strcmp(str, "DESTRA") == 0)
        {
            repeat = 4;
            strcpy(printStr, "STO GIRANDO A DESTRA");
        }
        else
        {
            strcpy(printStr, "NO ACTION");
        }

        for (c = 0; c < repeat; ++c)
        {
            if (writeln(logFd, printStr) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            sleep(1);
        }
    }

    close(clientFd);

    return 0;
}