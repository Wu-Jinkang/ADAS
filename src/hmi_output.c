#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#define ECU_LOG "/log/ECU.log"

int readLine(int fd, char *str);

int main (void) 
{
    int fd;
    char str[100];

    fd = open(ECU_LOG, O_RDONLY | O_NONBLOCK); /* Open it for reading */ 

    /* Display received messages */
    do 
    {
        readLine(fd, str);
        printf("%s\n", str);
    } while (strcmp(str, "FINE") != 0);

    close(fd); /* Close pipe */ 

    return 0;
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