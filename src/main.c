#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <signal.h>

#include "conn.h"
#include "def.h"
#include "util.h"

struct Component {
    char *name;
    int fd;
};

void createPipe(char *path);
int createLog(char *path);
int getMode(char * inputString);
void initLogFiles(void);
void removeLogFiles(void);
void execComponents(char *mode);

int main(int argc, char *argv[])
{
    int mode; //launch mode
    mode = getMode(argv[1]);
    initLogFiles();
    execComponents(argv[1]);

    unsigned int speed = 0;

    int centralFd, clientFd;
    centralFd = initServerSocket();
    printf("Start central ecu, waiting for connections...\n");

    struct Component components[6];
    for (int i = 0; i < 6; ++i)
    {
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
        char ok[] = "ok";
        result = write(clientFd, ok, strlen(ok) + 1);
        if (result < 0)
        {
            perror("write");
            exit(1);
        }

        printf("%s connected\n", buffer);
        components[i].name = malloc(strlen(buffer) + 1);
        strcpy(components[i].name, buffer);
        components[i].fd = clientFd;
    }

    for (int i = 0; i < 6; ++i)
    {
        printf("%s:%d\n", components[i].name, components[i].fd);
    }

    int status;
    for (int i = 1; i <= 6; i++)
        wait(&status);

    return 0;
} 

void createPidFile (char *path) {
    int fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    char pid[8];
    if (fd == -1)
    {
        perror("open() error");
        exit(-1);
    }
    sprintf(pid, "%d", getpid());
    write(fd, pid, strlen(pid));
}

void createPipe (char *path)
{
    unlink(path);
    mknod(path, S_IFIFO, 0);
    chmod(path, 0660);
}

int createLog (char *path) 
{
    int fd = open(path, O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("create log");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return 1;
}

int getMode (char* inputString)
{
    // Check input
    if (inputString == NULL)
    {
        printf("Doesn't exist an argument after Main.\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(NORMAL, inputString) == 0)
    {
        return 0;
    }
    else if (strcmp(ARTIFICIAL, inputString) == 0)
    {
        return 1;
    }
    else
    {
        printf("Modality invalid.\n");
        exit(EXIT_FAILURE);
    }
}

void removeLogFiles (void)
{
    remove(ECU_LOG);
    remove(STEER_LOG);
    remove(THROTTLE_LOG);
    remove(BRAKE_LOG);
    remove(CAMERA_LOG);
    remove(RADAR_LOG);
    remove(ASSIST_LOG);
}

void initLogFiles (void)
{
    printf("Init log files\n");
    int state = createLog(ECU_LOG) &&
                createLog(STEER_LOG) &&
                createLog(THROTTLE_LOG) &&
                createLog(BRAKE_LOG) &&
                createLog(CAMERA_LOG) &&
                createLog(RADAR_LOG) &&
                createLog(ASSIST_LOG);
    if (!state)
    {
        removeLogFiles();
        printf("Log files creation failed.\n");
        exit(EXIT_FAILURE);
    }
}

void execComponents (char* mode)
{
    int processes = 6, i;

    for (i = 1; i <= processes; i++)
    {
        if (fork() == 0)
        {
            if (i == 1)
            {
                printf("Start brake by wire\n");
                char* args[] = {BRAKE_BY_WIRE, mode, NULL};
                if (execvp(BRAKE_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 2)
            {
                printf("Start forward facing radar\n");
                char* args[] = {FORWARD_FACING_RADAR, mode, NULL};
                if (execvp(FORWARD_FACING_RADAR, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 3)
            {
                printf("Start front windshield camera\n");
                char *args[] = {FRONT_WINDSHIELD_CAMERA, mode, NULL};
                if (execvp(FRONT_WINDSHIELD_CAMERA, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 4)
            {
                printf("Start hmi input\n");
                char *args[] = {HMI_INPUT, mode, NULL};
                if (execvp(HMI_INPUT, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 5)
            {
                printf("Start steer by wire\n");
                char *args[] = {STEER_BY_WIRE, mode, NULL};
                if (execvp(STEER_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 6)
            {
                printf("Start throttle control\n");
                char *args[] = {THROTTLE_CONTROL, mode, NULL};
                if (execvp(THROTTLE_CONTROL, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

void handler(int sig)
{
    if (sig == SIGINT)
        printf("PERICOLO!\n");
}