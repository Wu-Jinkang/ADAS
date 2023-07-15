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
#include <limits.h>

#include "conn.h"
#include "def.h"
#include "util.h"

int throttleBreaks(int fd);

int main(int argc, char *argv[])
{
    int clientFd;
    char componentName[] = "throttleControl";
    clientFd = connectToServer("central");
    sendComponentName(clientFd, componentName); // Send component info to central ECU

    char str[100], printStr[100];
    int logFd, randomFd;

    logFd = open(THROTTLE_LOG, O_WRONLY); // Open log file
    if (logFd == -1)
    {
        perror("open throttle log");
        exit(EXIT_FAILURE);
    }
    randomFd = open(getDataSrcRandom(argv[1]), O_RDONLY); // Open random file
    if (randomFd == -1)
    {
        perror("open urandom");
        exit(EXIT_FAILURE);
    }

    pid_t ppid;
    ppid = getppid(); // Get parent pid

    while (1)
    {
        memset(str, 0, sizeof str);
        sendOk(clientFd); // Synchronize central ECU
        readLine(clientFd, str);

        if (throttleBreaks(randomFd))
        {
            kill(ppid, SIGUSR1); // Send SIGUSR1 to central ECU if throttle control break
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
    close(logFd);
    close(randomFd);
    close(clientFd);
   
    return 0;
}

int throttleBreaks(int fd)
{
    unsigned int randomNumber;
    int result = read(fd, &randomNumber, sizeof randomNumber);
    if (result < 0)
    {
        perror("read random");
        exit(EXIT_FAILURE);
    }
    double probability = (double)randomNumber / (double)UINT_MAX;
    return probability <= 0.00001;
    /*
        DA PENSARE NUOVAMENTE COME SIMULARE
    */
}