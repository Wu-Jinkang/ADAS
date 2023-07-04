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
    char componentName[] = "frontWindshieldCamera";
    clientFd = connectToServer();
    sendComponentName(clientFd, componentName);

    char str[100];
    int logFd, cameraFd;

    logFd = open(CAMERA_LOG, O_WRONLY);
    if (logFd == -1)
    {
        perror("open camera log");
        exit(EXIT_FAILURE);
    }

    cameraFd = open(FRONT_CAMERA_DATA, O_RDONLY);
    if (cameraFd == -1)
    {
        perror("open front camera data");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        memset(str, 0, sizeof(str));

        readLine(cameraFd, str);
        if (writeln(logFd, str) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        if (writeln(clientFd, str) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        sleep(1);
    }

    close(cameraFd);
    close(logFd);

    return 0;
}