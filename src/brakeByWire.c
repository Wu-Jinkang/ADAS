#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "conn.h"

int main(int argc, char *argv[]) 
{
    int clientFd;
    char processName[] = "brakeByWire";
    clientFd = connectToServer();

    while (1)
    {
        printf("start writing\n");
        int result = write(clientFd, processName, strlen(processName) + 1);
        if (result < 0)
        {
            perror("write");
            exit(1);
        }
        sleep(1);
        printf("end writing\n");
    }

    return 0;
}