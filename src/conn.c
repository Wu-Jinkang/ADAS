#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

int initServerSocket(void)
{
    int centralFd;
    socklen_t centralLen;
    struct sockaddr_un centralAddr;
    centralLen = sizeof(centralAddr);
    centralFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (centralFd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    centralAddr.sun_family = AF_UNIX;
    strcpy(centralAddr.sun_path, "central");
    unlink(centralAddr.sun_path);
    if (bind(centralFd, (struct sockaddr *)&centralAddr, centralLen) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(centralFd, 8) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return centralFd;
}

int connectToServer(void)
{
    int clientFd, serverLen, result;
    struct sockaddr_un serverUNIXAddress;
    struct sockaddr *serverSockAddrPtr;
    serverSockAddrPtr = (struct sockaddr *)&serverUNIXAddress;
    serverLen = sizeof(serverUNIXAddress);
    clientFd = socket(AF_UNIX, SOCK_STREAM, 0);
    serverUNIXAddress.sun_family = AF_UNIX;
    strcpy(serverUNIXAddress.sun_path, "central");
    do
    {
        result = connect(clientFd, serverSockAddrPtr, serverLen);
        if (result == -1)
            sleep(1);
    } while (result == -1);

    return clientFd;
}

int readLine(int fd, char *str)
{
    int n, i = 0;

    while (1)
    {
        n = read(fd, str + i, 1);
        if (n == 0 || *(str + i) == 10 || *(str + i) == '\n' || *(str + i) == '\0')
        {
            *(str+i) = '\0';
            break;
        }
        else if (n < 0)
        {
            perror("read");
            exit(1);
        }
        i++;
    }

    return i + 1;
}