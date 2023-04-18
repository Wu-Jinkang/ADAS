#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int readLine(int fd, char *str);
int read8(int fd, unsigned char *str);
int throttle_breaks();
int readLineFromIndex(int fd, char *str, int *index);
int writeln(int fd, char *str);

void initCentralECU()
{
    CAR_SPEED = 0;
    char str[100];
    int fd_log, fd_central, start;
    start = 0;

    unlink(CENTRAL_ECU);
    mknod(CENTRAL_ECU, S_IFIFO, 0);
    chmod(CENTRAL_ECU, 0660);
    fd_log = open(ECU_LOG, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }
    close(fd_log);

    while (1)
    {
        memset(str, 0, sizeof(str));
        fd_central = open(CENTRAL_ECU, O_RDONLY);

        if (fd_central == -1)
        {
            perror("open() error");
            close(fd_central);
            exit(-1);
        }
        readLine(fd_central, str);

        close(fd_central);

        if (start || strcmp(str, "INIZIO") == 0)
        {
            start = 1;
            fd_log = open(ECU_LOG, O_WRONLY | O_APPEND);
            if (fd_log == -1)
            {
                perror("open() error");
                exit(-1);
            }
            if (writeln(fd_log, str) == -1)
            {
                perror("write() error");
                unlink(CENTRAL_ECU);
                close(fd_log);
                close(fd_central);
                exit(-1);
            }
            close(fd_log);
        }

        if (strcmp(str, "ARRESTO") == 0)
        {
            break;
        }
    }

    sleep(10);
    unlink(CENTRAL_ECU);
}

void initSteerByWire()
{
    char str[100], print_str[100];
    int fd_log, fd_steer, count;

    count = 0;
    unlink(STEER_BY_WIRE);
    mknod(STEER_BY_WIRE, S_IFIFO, 0);
    chmod(STEER_BY_WIRE, 0660);
    fd_log = open(STEER_LOG, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }

    while (1)
    {
        memset(str, 0, sizeof(str));

        fd_steer = open(STEER_BY_WIRE, O_RDONLY);

        if (fd_steer == -1)
        {
            perror("open() error");
            close(fd_steer);
            exit(-1);
        }
        readLine(fd_steer, str);
        close(fd_steer);

        if (strcmp(str, "PARCHEGGIO") == 0 || count == 3)
        {
            count = 0;
        }

        if (count == 0)
        {
            if (strcmp(str, "SINISTRA") == 0)
            {
                strcpy(print_str, "STO GIRANDO A SINISTRA");
            }
            else if (strcmp(str, "DESTRA") == 0)
            {
                strcpy(print_str, "STO GIRANDO A DESTRA");
            }
            else
            {
                strcpy(print_str, "NO ACTION");
            }
        }

        if (writeln(fd_log, print_str) == -1)
        {
            perror("write() error");
            unlink(STEER_BY_WIRE);
            close(fd_log);
            close(fd_steer);
            exit(-1);
        }
        count++;

        if (strcmp(str, "ARRESTO") == 0)
        {
            break;
        }
        sleep(1);
    }

    unlink(STEER_BY_WIRE);
    close(fd_log);
}

void initThrottleControl()
{
    char str[100], print_str[100];
    int fd_log, fd_throttle;

    unlink(THROTTLE_CONTROL);
    mknod(THROTTLE_CONTROL, S_IFIFO, 0);
    chmod(THROTTLE_CONTROL, 0660);
    fd_log = open(THROTTLE_LOG, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }

    while (1)
    {
        memset(str, 0, sizeof(str));

        fd_throttle = open(THROTTLE_CONTROL, O_RDONLY);

        if (fd_throttle == -1)
        {
            perror("open() error");
            close(fd_throttle);
            exit(-1);
        }
        readLine(fd_throttle, str);
        close(fd_throttle);

        if (throttle_breaks())
        {
            // send signal to central ecu
            printf("OPS! ACCELERATORE ROTTO!");
            break;
        }

        if (strcmp(str, "INCREMENTO 5") == 0)
        {
            CAR_SPEED += 5;
            sprintf(print_str, "%d:INCREMENTO 5\n", (int)time(NULL));
            if (writeln(fd_log, print_str) == -1)
            {
                perror("write() error");
                unlink(THROTTLE_CONTROL);
                close(fd_log);
                close(fd_throttle);
                exit(-1);
            }
        }

        if (strcmp(str, "ARRESTO") == 0)
        {
            break;
        }
    }

    unlink(THROTTLE_CONTROL);
    close(fd_log);
}

void initBrakeByWire()
{
    char str[100], print_str[100];
    int fd_log, fd_brake;

    unlink(BRAKE_BY_WIRE);
    mknod(BRAKE_BY_WIRE, S_IFIFO, 0);
    chmod(BRAKE_BY_WIRE, 0660);
    fd_log = open(BRAKE_LOG, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_log == -1)
    {
        perror("open() error");
        exit(-1);
    }

    while (1)
    {
        memset(str, 0, sizeof(str));
        
        fd_brake = open(BRAKE_BY_WIRE, O_RDONLY);

        if (fd_brake == -1)
        {
            perror("open() error");
            close(fd_brake);
            exit(-1);
        }
        readLine(fd_brake, str);
        close(fd_brake);

        if (strcmp(str, "FRENO 5") == 0 && CAR_SPEED >= 5)
        {
            CAR_SPEED -= 5;
            sprintf(print_str, "%d:INCREMENTO 5\n", (int)time(NULL));
        }
        else if (strcmp(str, "ARRESTO") == 0)
        {
            CAR_SPEED = 0;
            strcpy(print_str, "ARRESTO AUTO");
        }

        if (writeln(fd_log, print_str) == -1)
        {
            perror("write() error");
            unlink(BRAKE_BY_WIRE);
            close(fd_log);
            close(fd_brake);
            exit(-1);
        }

        if (strcmp(str, "ARRESTO") == 0)
        {
            break;
        }

        printf("BRAKE\n");
        sleep(1);
    }

    unlink(BRAKE_BY_WIRE);
    close(fd_log);
}

void initFrontWindshieldCamera()
{
    char str[100];
    int fd_log, fd_camera, fd_central;
    fd_log = open(CAMERA_LOG, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
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

        if (strcmp(str, "PARCHEGGIO") == 0 || strcmp(str, "ARRESTO") == 0)
        {
            break;
        }

        sleep(1);
    }

    close(fd_camera);
    close(fd_log);
}

void initForwardFacingRadar()
{
    unsigned char str[17];
    int fd_log, fd_central, fd_urandom;
    fd_log = open(RADAR_LOG, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
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
    unsigned char str[8];
    int fd_log, fd_central, fd_urandom, i = 0, byte_read = 0;
    fd_log = open(ASSIST_LOG, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
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

    return (n > 0);
}

int read8(int fd, unsigned char *str)
{
    ssize_t n = 0;
    unsigned short value1, value2, value3, value4;
    n = read(fd, str, sizeof(str));

    printf("%x", str[0]);
    printf("-%x", str[1]);
    printf("-%x", str[2]);
    printf("-%x", str[3]);
    printf("-%x", str[4]);
    printf("-%x", str[5]);
    printf("-%x", str[6]);
    printf("-%x\n", str[7]);

    value1 = ((str[0] << 8) | str[1]);
    value2 = ((str[2] << 8) | str[3]);
    value3 = ((str[4] << 8) | str[5]);
    value4 = ((str[6] << 8) | str[7]);

    printf("%d", value1);
    printf("-%d", value2);
    printf("-%d", value3);
    printf("-%d\n", value4);

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