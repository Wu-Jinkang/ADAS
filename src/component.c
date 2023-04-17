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
int throttle_breaks();
int readLineFromIndex(int fd, char *str, int *index);
int readLine2(int fd, char *str);

void initCentralECU()
{
    CAR_SPEED = 0;
    char str[100];
    int fd_log, fd_central, start;

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
            size_t len = strlen(str);
            char *newstr = malloc(len + 2);
            strcpy(newstr, str);
            strcat(newstr, "\n");
            if (write(fd_log, newstr, len + 2) == -1)
            {
                perror("write() error");
                free(newstr);
                unlink(CENTRAL_ECU);
                close(fd_log);
                close(fd_central);
                exit(-1);
            }
            free(newstr);
            close(fd_log);
        }

        if (strcmp(str, "ARRESTO") == 0)
        {
            break;
        }
    }

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

        size_t len = strlen(print_str);
        if (write(fd_log, print_str, len) == -1)
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
            size_t len = strlen(print_str);
            if (write(fd_log, print_str, len) == -1)
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

        size_t len = strlen(print_str);
        if (write(fd_log, print_str, len) == -1)
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
    printf("It works\n");
    while (1)
    {
        memset(str, 0, sizeof(str));

        readLine2(fd_camera, str);
        str[sizeof(str) - 1] = '\0';

        size_t len = strlen(str);
        if (write(fd_log, str, len) == -1)
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
        if (write(fd_central, str, len) == -1)
        {
            perror("write() error");
            close(fd_central);
            close(fd_log);
            close(fd_camera);
            exit(-1);
        }
        close(fd_central);

        if (strcmp(str, "ARRESTO") == 0)
        {
            break;
        }

        sleep(1);
    }

    close(fd_camera);
    close(fd_log);
}

int readLine(int fd, char *str)
{
    int n;
    do
    {
        n = read(fd, str, 1);
    } while (n > 0 && *str++ != '\0');

    return (n > 0);
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

int readLine2(int fd, char *str)
{
    int n;
    do
    {
        n = read(fd, str, 1);
        if (n <= 0 || *str == 10 || *str == EOF)
        {
            break;
        }
        str += 1;
    } while (1);

    return (n > 0);
}