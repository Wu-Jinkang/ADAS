#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#define CENTRAL_ECU "log/central_ecu"
#define ECU_LOG "log/ECU.log"
#define STEER_BY_WIRE "log/steer_by_wire"
#define STEER_LOG "log/steer.log"
#define THROTTLE_CONTROL "log/throttle_control"
#define THROTTLE_LOG "log/throttle.log"
#define BRAKE_BY_WIRE "log/brake_by_wire"
#define BRAKE_LOG "log/brake.log"
#define CAMERA_LOG "log/camera.log"
#define RADAR_LOG "log/radar.log"
#define FORWARD_FACING_RADAR "log/forward_facing_radar"
#define FRONT_CAMERA_DATA "res/frontCamera.data"
#define ASSIST_LOG "log/assist.log"
#define URANDOM "/dev/urandom"
#define NORMAL "NORMALE"
#define ARTIFICIAL "ARTIFICIALE"
#define READ 0
#define WRITE 1

int CAR_SPEED = 0;
int pid_central, pid_steer, pid_throttle, pid_brake, pid_front_camera, pid_radar, pid_park;
void createPidFile(char *path);
void createPipe(char *path);
int createLog(char *path);
#include "component.c"

int main(int argc, char *argv[])
{
    int mode;
    // Check input
    if (argv[1] == NULL)
    {
        printf("Doesn't exist an argument after Main.");
        exit(2);
    }
    if (strcmp(NORMAL, argv[1]) == 0)
    {
        mode = 0;
    }
    else if (strcmp(ARTIFICIAL, argv[1]) == 0)
    {
        mode = 1;
    }
    else
    {
        printf("Modality not valid.");
        exit(2);
    }

    createPipe(CENTRAL_ECU);
    createPipe(THROTTLE_CONTROL);
    createPipe(BRAKE_BY_WIRE);
    createPipe(STEER_BY_WIRE);

    int state = createLog(ECU_LOG) &&
    createLog(STEER_LOG) &&
    createLog(THROTTLE_LOG) &&
    createLog(BRAKE_LOG) &&
    createLog(CAMERA_LOG) &&
    createLog(RADAR_LOG) &&
    createLog(ASSIST_LOG);
    if (!state) 
    {
        unlink(CENTRAL_ECU);
        unlink(THROTTLE_CONTROL);
        unlink(BRAKE_BY_WIRE);
        unlink(STEER_BY_WIRE);
        exit(-1);
    }

    int processes = 6, i;

    for (i = 1; i <= processes; i++)
    {
        if (fork() == 0)
        {
            if (i == 1)
            {
                createPidFile("run/central.pid");
                initCentralECU();
            }
            else if (i == 2)
            {
                createPidFile("run/steer.pid");
                initSteerByWire();
            }
            else if (i == 3) {
                createPidFile("run/throttle.pid");
                initThrottleControl();
            }
            else if (i == 4)
            {
                createPidFile("run/brake.pid");
                initBrakeByWire();
            }
            else if (i == 5) {
                createPidFile("run/front_camera.pid");
                initFrontWindshieldCamera();
            }
            else if (i == 6)
            {
                // createPidFile("run/radar.pid");
                // initForwardFacingRadar();
            }
            else if (i == 7)
            {

            }
            exit(0);
        }
    }
    int status;
    for (i = 1; i <= processes; i++)
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
        return 0;
    }
    else {
        close(fd);
        return 1;
    }
}