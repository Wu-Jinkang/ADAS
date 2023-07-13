#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "conn.h"
#include "def.h"

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

struct Component connectToComponent(int centralFd)
{
    struct Component c1;
    int clientFd;
    socklen_t clientLen;
    struct sockaddr_un clientAddr;
    clientLen = sizeof(clientAddr);
    clientFd = accept(centralFd, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientFd < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int result;
    
    memset(buffer, 0, sizeof buffer);
    while (readLine(clientFd, buffer) <= 0)
    {
        sleep(1);
    }

    sendOk(clientFd);

    c1.name = malloc(strlen(buffer) + 1);
    strcpy(c1.name, buffer);

    int pid;
    while (read(clientFd, &pid, sizeof pid) <= 0)
    {
        sleep(1);
    }

    printf("%s[%d]: connected\n", buffer, pid);
    c1.fd = clientFd;
    c1.pid = pid;

    return c1;
}

void sendComponentName(int clientFd, char *name)
{
    int result, pid;
    sendC(clientFd, name);
    pid = getpid();
    result = write(clientFd, &pid, sizeof pid);
    if (result < 0)
    {
        perror("write");
        exit(1);
    }
}

void sendC(int clientFd, char *buffer)
{
    int result;
    result = write(clientFd, buffer, strlen(buffer) + 1);
    if (result < 0)
    {
        perror("write");
        exit(1);
    }

    waitOk(clientFd);
}

void sendOk(int clientFd)
{
    char ok[] = "ok";
    int result = write(clientFd, ok, strlen(ok) + 1);
    if (result < 0)
    {
        perror("write");
        exit(1);
    }
}

void waitOk(int clientFd)
{
    char res[3];
    memset(res, 0, sizeof res);
    while (!(read(clientFd, res, sizeof res) > 0 && strcmp(res, "ok") == 0))
    {
        memset(res, 0, sizeof res);
        printf("%s\n", res);
        sleep(1);
    }
}