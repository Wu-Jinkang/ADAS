#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "util.h"
#include "def.h"

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

char* getDataSrc(char *mode)
{
    return strcmp(mode, "ARTIFICIALE") == 0 ? URANDOM_ARTIFICIAL : URANDOM;
}

int read8(int fd, char *str)
{
    ssize_t n = 0;
    n = read(fd, str, 8);
    if (n < 0)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    return n;
}