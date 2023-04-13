#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

int readLine(int fd, char *str);

void initCentralECU() {
    int car_speed = 0;
    char str[100];
    int fd_log, fd_central, start;

    /* Remove named pipe if it already exists */
    unlink(CENTRAL_ECU);

    /* Create named pipe */
    mknod(CENTRAL_ECU, S_IFIFO, 0);

    /* Change its permissions */
    chmod(CENTRAL_ECU, 0660);

    fd_central = open(CENTRAL_ECU, O_RDONLY | O_NONBLOCK);
    if (fd_central == -1) {
        perror("open() error");
        exit(-1);
    }

    fd_log = open(ECU_LOG, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_log == -1) {
        perror("open() error");
        exit(-1);
    }

    while (1) {
        readLine(fd_central, str);
        if (strcmp(str, "ARRESTO") == 0) {
            break;
        }

        if (start || strcmp(str, "INIZIO") == 0) {
            start = 1;
            size_t len = strlen(str);
            if (write(fd_log, str, len) == -1) {
                perror("write() error");
                close(fd_log);
                close(fd_central);
                exit(-1);
            }
        }
    }

    unlink(CENTRAL_ECU); /* Remove used pipe */
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