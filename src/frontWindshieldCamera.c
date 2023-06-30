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

    clientFd = connectToServer();

    return 0;
}