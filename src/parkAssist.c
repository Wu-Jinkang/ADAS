#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include "conn.h"
#include "def.h"
#include "util.h"

void execSurroundViewCameras(char *mode);
void termHandler(int sig);
struct Component surroundViewCameras;

int main(int argc, char *argv[])
{
    signal(SIGTERM, termHandler);
    execSurroundViewCameras(argv[1]); // Start surround view cameras proccess

    int clientFd, serverFd;
    char componentName[] = "parkAssist";
    clientFd = connectToServer("central"); // Connect to central ECU
    sendComponentName(clientFd, componentName);

    serverFd = initServerSocket("parkAssist");
    surroundViewCameras = connectToComponent(serverFd); // Connect to surround view cameras
    int flags = fcntl(surroundViewCameras.fd, F_GETFL, 0);
    fcntl(surroundViewCameras.fd, F_SETFL, flags | O_NONBLOCK);

    char str[100];
    int logFd, urandomFd;

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

    for (int i = 0; i < 30; ++i)
    {
        memset(str, 0, sizeof str);
        int byteRead = read8(urandomFd, str);

        memset(surroundViewCameras.buffer, 0, sizeof surroundViewCameras.buffer);
        if (readLine(surroundViewCameras.fd, surroundViewCameras.buffer) > 0)
        {
            sendOk(surroundViewCameras.fd);
            sendMsg(clientFd, surroundViewCameras.buffer);
        }
        if (byteRead == 8)
        {
            if (writeln(logFd, str) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
            sendMsg(clientFd, str);
        }

        sleep(1);
    }

    kill(surroundViewCameras.pid, SIGTERM);
    sendMsg(clientFd, "END");
    close(urandomFd);
    close(logFd);

    return 0;
}

void execSurroundViewCameras(char *mode)
{
    if (fork() == 0)
    {
        printf("Start surround view cameras\n");
        char *args[] = {SURROUND_VIEW_CAMERAS, mode, NULL};
        if (execvp(SURROUND_VIEW_CAMERAS, args) < 0)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
}

void termHandler(int sig)
{
    kill(surroundViewCameras.pid, SIGTERM);
    exit(EXIT_SUCCESS);
}