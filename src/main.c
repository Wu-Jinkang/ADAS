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

#define BRAKE_BY_WIRE "bin/brake_by_wire"
#define FORWARD_FACING_RADAR "bin/forward_facing_radar"
#define STEER_BY_WIRE "bin/steer_by_wire"
#define THROTTLE_CONTROL "bin/throttle_control"
#define FRONT_WINDSHIELD_CAMERA "bin/front_windshield_camera"
#define HMI_INPUT "bin/hmi_input"
#define HMI_OUTPUT "bin/hmi_output"

#define ECU_LOG "log/ECU.log"
#define STEER_LOG "log/steer.log"
#define THROTTLE_LOG "log/throttle.log"
#define BRAKE_LOG "log/brake.log"
#define CAMERA_LOG "log/camera.log"
#define RADAR_LOG "log/radar.log"
#define CAR_SPEED "log/car_speed"
#define ASSIST_LOG "log/assist.log"

#define FRONT_CAMERA_DATA "res/frontCamera.data"
#define URANDOM "/dev/urandom"
#define NORMAL "NORMALE"
#define ARTIFICIAL "ARTIFICIALE"
#define READ 0
#define WRITE 1

// int CAR_SPEED = 0;
int pid_central, pid_steer, pid_throttle, pid_brake, pid_front_camera, pid_radar, pid_park;
void createPidFile(char *path);
void createPipe(char *path);
int createLog(char *path);
int getMode(char * inputString);
void initLogFiles(void);
void execComponents(char* mode);

int main(int argc, char *argv[])
{
    int mode; //launch mode
    mode = getMode(argv[1]);
    initLogFiles();

    int centralFd, clientFd;
    socklen_t centralLen, clientLen;

    struct sockaddr_un centralAddr, clientAddr;
    centralLen = sizeof(centralAddr);
    clientLen = sizeof(clientAddr);
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
    while (1)
    {
        clientFd = accept(centralFd, (struct sockaddr *)&clientAddr, &clientLen);
        printf("%d\n", clientFd);
        if (clientFd < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if (fork() == 0)
        {  
            
        }
        else
            close(clientFd);
    }

    execComponents(argv[1]);


    // int status;
    // for (int i = 1; i <= 6; i++)
    //     wait(&status);

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
        return 0;
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
        perror("Modality invalid.\n");
        exit(EXIT_FAILURE);
    }
}

void initLogFiles (void)
{
    int state = createLog(ECU_LOG) &&
        createLog(STEER_LOG) &&
        createLog(THROTTLE_LOG) &&
        createLog(BRAKE_LOG) &&
        createLog(CAMERA_LOG) &&
        createLog(RADAR_LOG) &&
        createLog(ASSIST_LOG);
    if (!state)
    {
        remove(ECU_LOG);
        remove(STEER_LOG);
        remove(THROTTLE_LOG);
        remove(BRAKE_LOG);
        remove(CAMERA_LOG);
        remove(RADAR_LOG);
        remove(ASSIST_LOG);
        perror("Log files creation failed.");
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
                char* args[] = {BRAKE_BY_WIRE, mode, NULL};
                if (execvp(BRAKE_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 2)
            {
                char* args[] = {FORWARD_FACING_RADAR, mode, NULL};
                if (execvp(BRAKE_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 3)
            {
                char *args[] = {FRONT_WINDSHIELD_CAMERA, mode, NULL};
                if (execvp(BRAKE_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 4)
            {
                char *args[] = {HMI_INPUT, mode, NULL};
                if (execvp(BRAKE_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 5)
            {
                char *args[] = {STEER_BY_WIRE, mode, NULL};
                if (execvp(BRAKE_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 6)
            {
                char *args[] = {THROTTLE_CONTROL, mode, NULL};
                if (execvp(BRAKE_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}