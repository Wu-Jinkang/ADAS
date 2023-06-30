#include <sys/socket.h>
#include <sys/un.h>

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