#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
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

char* getDataSrcUrandom(char *mode)
{
    return strcmp(mode, "ARTIFICIALE") == 0 ? URANDOM_ARTIFICIAL : URANDOM;
}

int read8(int fd, char *str)
{
    unsigned int buffer[8];
    ssize_t n = 0;
    n = read(fd, buffer, 8);
    if (n < 0)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    sprintf(str, "%04X", *buffer);

    return n;
}

int isNumber(char *str)
{
    for (int i = 0, len = strlen(str); i < len; i++)
    {
        if (!isdigit(str[i]))
        {
            return 0;
        }
    }

    return 1;
}

int toNumber(char *str)
{
    int num = 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        num = num * 10 + (str[i] - 48);
    }

    return num;
}