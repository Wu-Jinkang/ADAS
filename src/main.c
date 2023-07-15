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

struct Component components[7];
unsigned int speed = 0;

int createLog(char *path);
int getMode(char * inputString);
void initLogFiles(void);
void removeLogFiles(void);
void execComponents(char *mode);
void centralECU(int centralFd, struct Component *components, char *mode);
void execParkAssist(char *mode);
int checkHex(char *buffer);

int main(int argc, char *argv[])
{
    int mode; //launch mode
    mode = getMode(argv[1]);
    initLogFiles();
    execComponents(argv[1]);

    int centralFd, clientFd;
    centralFd = initServerSocket("central");
    printf("Start central ecu, waiting for connections...\n");

    for (int i = 0; i < 6; ++i)
    {
        components[i] = connectToComponent(centralFd);
        int flags = fcntl(components[i].fd, F_GETFL, 0);
        fcntl(components[i].fd, F_SETFL, flags | O_NONBLOCK);
    }

    centralECU(centralFd, components, argv[1]);
    close(centralFd);
    unlink("central");
    return 0;
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
    remove(CAMERAS_LOG);
}

void initLogFiles (void)
{
    removeLogFiles();
    printf("Init log files\n");
    int state = createLog(ECU_LOG) &&
                createLog(STEER_LOG) &&
                createLog(THROTTLE_LOG) &&
                createLog(BRAKE_LOG) &&
                createLog(CAMERA_LOG) &&
                createLog(RADAR_LOG) &&
                createLog(ASSIST_LOG) && 
                createLog(CAMERAS_LOG);
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

void execParkAssist(char *mode)
{
    if (fork() == 0)
    {
        printf("Start park assist\n");
        char *args[] = {PARK_ASSIST, mode, NULL};
        if (execvp(PARK_ASSIST, args) < 0)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
}

void throttleControlBrokeHandler(int sig)
{
    printf("Throttle control rotto!\n");
    speed = 0;
    for (int i = 0; i < 6; i++)
    {
        kill(components[i].pid, SIGTERM);
    }
    exit(EXIT_SUCCESS);
}

void centralECU (int centralFd, struct Component *components, char *mode)
{
    int hmiInput, steerByWire, brakeByWire, throttleControl, forwardFacingRadar, frontWindshieldCamera, parkAssist = 6; // Index

    for (int i = 0; i < 6; ++i)
    {
        if (strcmp(components[i].name, "hmiInput") == 0)
        {
            hmiInput = i;
            sendOk(components[hmiInput].fd); // Start listen user input
            while (1)
            {
                memset(components[hmiInput].buffer, 0, sizeof components[hmiInput].buffer);
                if (readLine(components[hmiInput].fd, components[hmiInput].buffer) > 0)
                {
                    sendOk(components[hmiInput].fd);
                    if (strcmp(components[hmiInput].buffer, "INIZIO") == 0)
                    {
                        break;
                    }
                }
                sleep(1);
            }
        }
        if (strcmp(components[i].name, "steerByWire") == 0)
        {
            steerByWire = i;
        }
        if (strcmp(components[i].name, "brakeByWire") == 0)
        {
            brakeByWire = i;
        }
        if (strcmp(components[i].name, "forwardFacingRadar") == 0)
        {
            forwardFacingRadar = i;
        }
        if (strcmp(components[i].name, "frontWindshieldCamera") == 0)
        {
            frontWindshieldCamera = i;
        }
        if (strcmp(components[i].name, "throttleControl") == 0)
        {
            throttleControl = i;
        }
    }

    unsigned int readSpeed = 0, danger = 0, increment = 0, decrement = 0, suspend = 0, stop = 0, parking = 0;

    int logFd = open(ECU_LOG, O_WRONLY);
    if (logFd == -1)
    {
        perror("open throttle log");
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, throttleControlBrokeHandler); // should set speed to 0 and terminate todo

    while (1)
    {
        memset(components[hmiInput].buffer, 0, sizeof components[hmiInput].buffer);
        if (!parking && readLine(components[hmiInput].fd, components[hmiInput].buffer) > 0) // Ignore HMI info if parking is requested
        {
            sendOk(components[hmiInput].fd);
            if (strcmp(components[hmiInput].buffer, "PARCHEGGIO") == 0)
            {
                parking = 1;
            }

            if (strcmp(components[hmiInput].buffer, "ARRESTO") == 0)
            {
                writeln(logFd, "ARRESTO");
                kill(components[brakeByWire].pid, SIGUSR2); // Send signal to brake by wire
                speed = 0;
            }

            if (strcmp(components[hmiInput].buffer, "INIZIO") == 0)
            {
                suspend = 0; // Restart from status danger
            }
        }

        memset(components[forwardFacingRadar].buffer, 0, sizeof components[forwardFacingRadar].buffer); // Reset buffer
        if (!parking && readLine(components[forwardFacingRadar].fd, components[forwardFacingRadar].buffer) > 0) // Ignore forward facing radar info if parking is requested
        {
            sendOk(components[forwardFacingRadar].fd);
        }

        // Read speed to be achieved, turn left, turn right and danger from front windshield camera
        memset(components[frontWindshieldCamera].buffer, 0, sizeof components[frontWindshieldCamera].buffer);         // Reset buffer
        if (!parking && !suspend && readLine(components[frontWindshieldCamera].fd, components[frontWindshieldCamera].buffer) > 0) // Ignore front windshield camera if suspend or parking
        {
            sendOk(components[frontWindshieldCamera].fd); // Sync

            if (isNumber(components[frontWindshieldCamera].buffer)) // Read a number
            {
                readSpeed = toNumber(components[frontWindshieldCamera].buffer);
            }

            if (strcmp(components[frontWindshieldCamera].buffer, "DESTRA") == 0 || strcmp(components[frontWindshieldCamera].buffer, "SINISTRA") == 0)
            {
                writeln(logFd, components[frontWindshieldCamera].buffer);
                sendMsg(components[steerByWire].fd, components[frontWindshieldCamera].buffer); // Send comand to steer by wire
            }

            if (strcmp(components[frontWindshieldCamera].buffer, "PERICOLO") == 0)
            {
                writeln(logFd, "ARRESTO");
                kill(components[brakeByWire].pid, SIGUSR2); // Send signal to brake by wire
                suspend = 1;                                // Suspend until user input INIZIO
                speed = 0;
            }

            if (strcmp(components[frontWindshieldCamera].buffer, "PARCHEGGIO") == 0)
            {
                writeln(logFd, "PARCHEGGIO");
                parking = 1;
            }
        }

        if (!suspend && !parking) // Ignore actuators if suspend
        {
            // Increment speed
            memset(components[throttleControl].buffer, 0, sizeof components[throttleControl].buffer);          // Reset buffer
            if (increment || readLine(components[throttleControl].fd, components[throttleControl].buffer) > 0) // Throttle control ready to update
            {
                increment = 1;
                if (speed < readSpeed)
                {
                    speed += 5;
                    writeln(logFd, "INCREMENTO 5");
                    sendMsg(components[throttleControl].fd, "INCREMENTO 5");
                    increment = 0;
                }
            }

            // Decrement speed
            memset(components[brakeByWire].buffer, 0, sizeof components[brakeByWire].buffer);          // Reset buffer
            if (decrement || readLine(components[brakeByWire].fd, components[brakeByWire].buffer) > 0) // Brake by wire ready to update
            {
                decrement = 1;
                if (speed > readSpeed)
                {
                    speed -= 5;
                    writeln(logFd, "FRENO 5");
                    sendMsg(components[brakeByWire].fd, "FRENO 5");
                    decrement = 0;
                }
            }
        }

        if (parking)
        {
            // Decrement speed
            memset(components[brakeByWire].buffer, 0, sizeof components[brakeByWire].buffer);          // Reset buffer
            if (speed >= 5 && (decrement || readLine(components[brakeByWire].fd, components[brakeByWire].buffer) > 0)) // Brake by wire ready to update
            {
                decrement = 0;
                speed -= 5;
                writeln(logFd, "FRENO 5");
                sendMsg(components[brakeByWire].fd, "FRENO 5");
                if (speed == 0)
                {
                    execParkAssist(mode);
                    components[parkAssist] = connectToComponent(centralFd);
                    int flags = fcntl(components[parkAssist].fd, F_GETFL, 0);
                    fcntl(components[parkAssist].fd, F_SETFL, flags | O_NONBLOCK);
                    kill(components[frontWindshieldCamera].pid, SIGTERM);
                    kill(components[forwardFacingRadar].pid, SIGTERM);
                    kill(components[steerByWire].pid, SIGTERM);
                    kill(components[brakeByWire].pid, SIGTERM);
                    kill(components[throttleControl].pid, SIGTERM);
                }
            }

            if (speed == 0)
            {
                memset(components[parkAssist].buffer, 0, sizeof components[parkAssist].buffer);             // Reset buffer
                if (readLine(components[parkAssist].fd, components[parkAssist].buffer) > 0)   // Brake by wire ready to update
                {
                    if (checkHex(components[parkAssist].buffer))
                    {
                        printf("PARKING FAILED, RESTART PARKING\n");
                        kill(components[parkAssist].pid, SIGTERM); // Stop park assist and restart
                        execParkAssist(mode);
                        components[parkAssist] = connectToComponent(centralFd);
                        int flags = fcntl(components[parkAssist].fd, F_GETFL, 0);
                        fcntl(components[parkAssist].fd, F_SETFL, flags | O_NONBLOCK);
                    }
                    else if (strcmp(components[parkAssist].buffer, "END") == 0)
                    {
                        kill(components[hmiInput].pid, SIGTERM); // Park assist and surround view cameras should quit by their self
                        break;
                    }
                }
            }
        }

        usleep(100000); // 0.1s to use less cpu
    }
}

int checkHex(char *buffer)
{
    return strstr(buffer, "172A") != NULL
        || strstr(buffer, "D693") != NULL
        || strstr(buffer, "0000") != NULL
        || strstr(buffer, "BDD8") != NULL
        || strstr(buffer, "FAEE") != NULL 
        || strstr(buffer, "4300") != NULL;
}