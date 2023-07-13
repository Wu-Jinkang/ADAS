#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <time.h>

#include "conn.h"
#include "def.h"
#include "util.h"

void dangerHandler(int sig);

int main(int argc, char *argv[])
{
    int clientFd;
    char componentName[] = "brakeByWire";
    clientFd = connectToServer();
    sendComponentName(clientFd, componentName);

    char str[100], printStr[100];
    int logFd;

    logFd = open(BRAKE_LOG, O_WRONLY);
    if (logFd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, dangerHandler);

    while (1)
    {
        memset(str, 0, sizeof str);
        sendOk(clientFd);
        readLine(clientFd, str);

        if (strcmp(str, "FRENO 5") == 0)
        {
            sprintf(printStr, "%d:DECREMENTO 5", (int)time(NULL));
        }

        if (strcmp(str, "PERICOLO") == 0)
        {
            sprintf(printStr, "%d:ARRESTO AUTO", (int)time(NULL));
        }

        if (writeln(logFd, printStr) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        sleep(1);
    }
    close(logFd);
    close(clientFd);
   
    return 0;
}

void dangerHandler(int sig)
{
    int logFd;
    char printStr[100];
    sprintf(printStr, "%d:ARRESTO AUTO\n", (int)time(NULL));

    logFd = open(BRAKE_LOG, O_WRONLY);
    if (logFd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (writeln(logFd, printStr) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
}