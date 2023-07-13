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

int throttle_breaks(void);

int main(int argc, char *argv[])
{
    int clientFd;
    char componentName[] = "throttleControl";
    clientFd = connectToServer();
    sendComponentName(clientFd, componentName);

    char str[100], printStr[100];
    int logFd, throttleFd;

    logFd = open(THROTTLE_LOG, O_WRONLY);
    if (logFd == -1)
    {
        perror("open throttle log");
        exit(EXIT_FAILURE);
    }

    pid_t ppid;
    ppid = getppid();

    while (1)
    {
        memset(str, 0, sizeof str);
        sendOk(clientFd);
        readLine(clientFd, str);

        if (throttle_breaks())
        {
            kill(ppid, SIGINT);
            // printf("OPS! ACCELERATORE ROTTO!");
            break;
        }

        if (strcmp(str, "INCREMENTO 5") == 0)
        {
            sprintf(printStr, "%d:INCREMENTO 5", (int)time(NULL));

            if (writeln(logFd, printStr) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }

        sleep(1);
    }
    close(throttleFd);
    close(logFd);

    return 0;
}

int throttle_breaks(void)
{
    int probability = 100000;
    int random_number = rand() % probability;
    return random_number == 0;
}