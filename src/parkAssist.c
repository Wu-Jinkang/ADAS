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
    char componentName[] = "parkAssist";
    clientFd = connectToServer();
    sendComponentName(clientFd, componentName);

    char str[8];
    int logFd, clientFd, urandomFd, i, byteRead = 0;

    logFd = open(ASSIST_LOG, O_WRONLY);
    if (logFd == -1)
    {
        perror("open park log");
        exit(EXIT_FAILURE);
    }
    urandomFd = open(getDataSrcUrandom(argv[1]), O_RDONLY);
    if (urandomFd == -1)
    {
        perror("open urandom");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < 30; i++)
    {
        memset(str, 0, sizeof str);

        byteRead = read8(urandomFd, str);

        printf("%d, %d\n", byteRead, i);

        if (byteRead == 8)
        {
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
        }

        sleep(1);
    }

    close(urandomFd);
    close(logFd);

    return 0;
}