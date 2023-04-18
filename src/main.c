#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
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

int CAR_SPEED = 0;
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

    int processes = 6;
    int i;
    for (i = 1; i <= processes; i++)
    {
        if (fork() == 0)
        {
            if (i == 1)
            {
                initCentralECU();
            }
            else if (i == 2)
            {
                // initSteerByWire();
            }
            else if (i == 3) {
                // initThrottleControl();
            }
            else if (i == 3)
            {
                // initBrakeByWire();
            }
            else if (i == 4) {
                // initFrontWindshieldCamera();
            }
            else if (i == 5)
            {
                // initForwardFacingRadar();
            }
            else if (i == 6)
            {
                initParkAssist();
            }
            exit(0);
        }
    }
    int status;
    for (i = 1; i <= processes; i++)
        wait(&status);

    return 0;
}