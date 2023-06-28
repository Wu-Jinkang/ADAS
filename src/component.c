#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

void centerStopHandler(int sig)
{
    printf("%d Attempted inappropriate read from control terminal\n", sig);
    exit(1);
}
void brakeDangerHandler(int sig);
int readLine(int fd, char *str);
int read8(int fd, char *str);
int throttle_breaks();
int readLineFromIndex(int fd, char *str, int *index);
int writeln(int fd, char *str);
int readPidFile(char *path);
void initParkAssist();

void initCentralECU()
{
    // CAR_SPEED = 0;
    char str[100];
    int fd_log, fd_central, start;
    struct sockaddr_un name;
    start = 0;

    fd_log = open(ECU_LOG, O_WRONLY | O_APPEND);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }

    int fd_steer = open(STEER_BY_WIRE, O_WRONLY);
    if (fd_steer == -1)
    {
        perror("open() error");
        exit(-1);
    }
    // fd_central = open(CENTRAL_ECU, O_RDONLY);
    // if (fd_central == -1)
    // {
    //     perror("open() error");
    //     exit(-1);
    // }

    fd_central = open(CENTRAL_ECU, O_RDONLY);
    if (fd_central == -1)
    {
        perror("open() error");
        exit(-1);
    }

    int fd_speed = open(STEER_BY_WIRE, O_WRONLY);
    if (fd_speed == -1)
    {
        perror("open() error");
        exit(-1);
    }
    write(fd_speed, "0", 1);
    while (1)
    {
        memset(str, 0, sizeof(str));
        readLine(fd_central, str);

        if (start || strcmp(str, "INIZIO") == 0)
        {
            start = 1;
            // pid_central = readPidFile("run/central.pid");
            // pid_steer = readPidFile("run/steer.pid");
            // pid_throttle = readPidFile("run/throttle.pid");
            // pid_brake = readPidFile("run/brake.pid");
            // pid_front_camera = readPidFile("run/front_camera.pid");
            // pid_radar = readPidFile("run/radar.pid");
            // pid_park = readPidFile("run/park.pid");
            if (strcmp(str, "SINISTRA") == 0 || strcmp(str, "DESTRA") == 0)
            {
                if (writeln(fd_steer, str) == -1)
                {
                    perror("write() error");
                    exit(-1);
                }
                if (writeln(fd_log, str) == -1)
                {
                    perror("write() error");
                    exit(-1);
                }
            }
            else if (strcmp(str, "PERICOLO") == 0)
            {
                kill(readPidFile("run/steer.pid"), SIGUSR1);
                if (writeln(fd_log, str) == -1)
                {
                    perror("write() error");
                    exit(-1);
                }
            }
            else if (strcmp(str, "PARCHEGGIO") == 0)
            {
                // if (fork() == 0)
                // {
                //     createPidFile("run/park.pid");
                //     initParkAssist();
                // }
            }
            // printf("%d,%d,%d,%d,%d,%d,%d\n", pid_central, pid_steer, pid_throttle, pid_brake, pid_front_camera, pid_radar, pid_park);
        }

        // if (strcmp(str, "ARRESTO") == 0)
        // {
        //     break;
        // }
    }

    sleep(10);
    close(fd_steer);
    close(fd_log);
    close(fd_central);
}

void initSteerByWire()
{
    char str[100], print_str[100];
    int fd_log, fd_steer, count, repeat;
    fd_log = open(STEER_LOG, O_WRONLY);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }

    fd_steer = open(STEER_BY_WIRE, O_RDONLY | O_NONBLOCK);
    while (1)
    {
        memset(str, 0, sizeof(str));
        readLine(fd_steer, str);

        repeat = 1;

        if (strcmp(str, "SINISTRA") == 0)
        {
            repeat = 4;
            strcpy(print_str, "STO GIRANDO A SINISTRA");
        }
        else if (strcmp(str, "DESTRA") == 0)
        {
            repeat = 4;
            strcpy(print_str, "STO GIRANDO A DESTRA");
        }
        else
        {
            strcpy(print_str, "NO ACTION");
        }

        for (count = 0; count < repeat; ++count)
        {
            if (writeln(fd_log, print_str) == -1)
            {
                perror("write() error");
                exit(-1);
            }
            sleep(1);
        }
    }
    close(fd_steer);
}

void initThrottleControl()
{
    char str[100], print_str[100];
    int fd_log, fd_throttle;

    fd_log = open(THROTTLE_LOG, O_WRONLY);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }
    fd_throttle = open(THROTTLE_CONTROL, O_RDONLY);
    if (fd_throttle == -1)
    {
        perror("open() error");
        exit(-1);
    }

    while (1)
    {
        memset(str, 0, sizeof(str));
        readLine(fd_throttle, str);

        if (throttle_breaks())
        {
            // send signal to central ecu
            printf("OPS! ACCELERATORE ROTTO!");
            break;
        }
        if (strcmp(str, "INCREMENTO 5") == 0)
        {
            // CAR_SPEED += 5;
            sprintf(print_str, "%d:INCREMENTO 5\n", (int)time(NULL));
            if (writeln(fd_log, print_str) == -1)
            {
                perror("write() error");
                exit(-1);
            }
        }
    }
    close(fd_throttle);
    close(fd_log);
}

void initBrakeByWire()
{
    char str[100], print_str[100];
    int fd_log, fd_brake;

    fd_log = open(BRAKE_LOG, O_WRONLY);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }
    signal(SIGUSR1, brakeDangerHandler);

    while (1)
    {
        memset(str, 0, sizeof(str));
        
        fd_brake = open(BRAKE_BY_WIRE, O_RDONLY);

        if (fd_brake == -1)
        {
            perror("open() error");
            exit(-1);
        }
        readLine(fd_brake, str);
        close(fd_brake);

        if (strcmp(str, "FRENO 5") == 0) // && CAR_SPEED >= 5
        {
            // CAR_SPEED -= 5;
            sprintf(print_str, "%d:INCREMENTO 5\n", (int)time(NULL));
        }

        if (writeln(fd_log, print_str) == -1)
        {
            perror("write() error");
            exit(-1);
        }

        sleep(1);
    }
    close(fd_log);
}

void initFrontWindshieldCamera()
{
    char str[100];
    int fd_log, fd_camera, fd_central;
    fd_log = open(CAMERA_LOG, O_WRONLY);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }

    fd_camera = open(FRONT_CAMERA_DATA, O_RDONLY);

    if (fd_camera == -1)
    {
        perror("open() error");
        close(fd_camera);
        exit(-1);
    }
    while (1)
    {
        memset(str, 0, sizeof(str));

        readLine(fd_camera, str);

        if (writeln(fd_log, str) == -1)
        {
            perror("write() error");
            close(fd_log);
            close(fd_camera);
            close(fd_central);
            exit(-1);
        }
        fd_central = open(CENTRAL_ECU, O_WRONLY);
        if (fd_central == -1)
        {
            perror("open() error");
            exit(-1);
        }
        if (writeln(fd_central, str) == -1)
        {
            perror("write() error");
            close(fd_central);
            close(fd_log);
            close(fd_camera);
            exit(-1);
        }
        close(fd_central);

        // if (strcmp(str, "PARCHEGGIO") == 0 || strcmp(str, "ARRESTO") == 0)
        // {
        //     break;
        // }

        sleep(1);
    }

    close(fd_camera);
    close(fd_log);
}

void initForwardFacingRadar()
{
    char str[17];
    int fd_log, fd_central, fd_urandom;
    fd_log = open(RADAR_LOG, O_WRONLY);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }

    while (1)
    {
        memset(str, 0, sizeof(str));
        fd_urandom = open(URANDOM, O_RDONLY);
        if (fd_urandom == -1)
        {
            perror("open() error");
            close(fd_urandom);
            exit(-1);
        }
        int byte_read = read8(fd_urandom, str);
        close(fd_urandom);
        if (byte_read == 8)
        {
            if (writeln(fd_log, str) == -1)
            {
                perror("write() error");
                close(fd_log);
                exit(-1);
            }
            fd_central = open(CENTRAL_ECU, O_WRONLY);
            if (fd_central == -1)
            {
                perror("open() error");
                close(fd_central);
                close(fd_log);
                exit(-1);
            }
            if (writeln(fd_central, str) == -1)
            {
                perror("write() error");
                close(fd_central);
                close(fd_log);
                exit(-1);
            }
            close(fd_central);
        }
        sleep(1);
    }

    close(fd_log);
}

void initParkAssist()
{
    char str[8];
    int fd_log, fd_central, fd_urandom, i = 0, byte_read = 0;
    fd_log = open(ASSIST_LOG, O_WRONLY);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }

    for (; i < 30; i++)
    {
        memset(str, 0, sizeof(str));
        fd_urandom = open(URANDOM, O_RDONLY);
        if (fd_urandom == -1)
        {
            perror("open() error");
            close(fd_urandom);
            exit(-1);
        }
        byte_read = read8(fd_urandom, str);
        printf("%d, %d\n", byte_read, i);
        close(fd_urandom);
        if (byte_read == 8)
        {
            if (writeln(fd_log, str) == -1)
            {
                perror("write() error");
                close(fd_log);
                exit(-1);
            }
            fd_central = open(CENTRAL_ECU, O_WRONLY);
            if (fd_central == -1)
            {
                perror("open() error");
                close(fd_central);
                close(fd_log);
                exit(-1);
            }
            if (writeln(fd_central, str) == -1)
            {
                perror("write() error");
                close(fd_central);
                close(fd_log);
                exit(-1);
            }
            close(fd_central);
        }
        sleep(1);
    }

    close(fd_log);
}

int readLine(int fd, char *str)
{
    int n;
    while (1)
    {
        n = read(fd, str, 1);
        if (n <= 0 || *str == 10 || *str == '\n' || *str == '\0')
        {
            *str = '\0';
            break;
        }
        str++;
    }

    return n > 0;
}

int read8(int fd, char *str)
{
    ssize_t n = 0;
    n = read(fd, str, 8);

    return n;
}

int throttle_breaks()
{
    int probability = 100000;
    int random_number = rand() % probability;
    return random_number == 0;
}

int readLineFromIndex(int fd, char *str, int *index)
{
    int n;
    lseek(fd, *index, SEEK_SET);
    do
    {
        n = read(fd, str, 1);
    } while (n > 0 && *str++ != '\0');

    return (n > 0);
}

int writeln(int fd, char *str)
{
    size_t len = strlen(str);
    char *newstr = malloc(len + 2);
    strcpy(newstr, str);
    strcat(newstr, "\n");
    
    return write(fd, newstr, len + 1);
}

void brakeDangerHandler(int sig)
{
    int fd_log;
    fd_log = open(BRAKE_LOG, O_WRONLY);
    if (fd_log == -1)
    {
        perror("open() error");
    }
    // CAR_SPEED = 0;
    if (writeln(fd_log, "ARRESTO AUTO") == -1)
    {
        perror("write() error");
        close(fd_log);
    }
}

int readPidFile(char *path)
{
    char pid[8];
    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("open() error");
        exit(-1);
    }

    // read pid
    readLine(fd, pid);

    // converting string to number
    int num = 0;
    for (int i = 0; pid[i] != '\0'; i++)
    {
        num = num * 10 + (pid[i] - 48);
    }

    return num;
}