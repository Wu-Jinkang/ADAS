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
    char componentName[] = "forwardFacingRadar";
    clientFd = connectToServer();
    sendComponentName(clientFd, componentName);

    char buffer[1024];
    int logFd, urandomFd;

    logFd = open(RADAR_LOG, O_WRONLY);
    if (logFd == -1)
    {
        perror("open radar log");
        exit(EXIT_FAILURE);
    }

    urandomFd = open(getDataSrcUrandom(argv[1]), O_RDONLY);
    if (urandomFd == -1)
    {
        perror("open urandom");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        memset(buffer, 0, sizeof buffer);

        int byteRead = read8(urandomFd, buffer);
        if (byteRead == 8)
        {
            if (writeln(logFd, buffer) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
            sendC(clientFd, buffer);
        }
        sleep(1);
    }

    close(logFd);
    close(urandomFd);
    close(clientFd);

    return 0;
}