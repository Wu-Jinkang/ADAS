#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

char *getCWD();
int readLine(int fd, char *str);

int main (void) 
{
    int fd;
    char str[100];
    char *cwd = getCWD();
    char *filename = "/hmi_output";
    size_t len = strlen(cwd) + strlen(filename) + 1;
    char *fullpath = malloc(len);
    if (fullpath == NULL) {
        free(cwd);
        perror("malloc() error");
        return -1;
    }
    strcpy(fullpath, cwd);
    free(cwd);
    strcat(fullpath, filename);
    printf("Full path: %s\n", fullpath);
    free(fullpath);

    /* Remove named pipe if it already exists */
    unlink(fullpath);
    /* Create named pipe */
    mknod("aPipe", S_IFIFO, 0);
    /* Change its permissions */
    chmod("aPipe", 0660);

    fd = open("aPipe", O_RDONLY); /* Open it for reading */ 
    printf("fd status %d\n", fd);

    /* Display received messages */
    while(readLine (fd, str)) 
    {
        printf ("%s\n", str);
    }

    close(fd); /* Close pipe */ 
    unlink("aPipe"); /* Remove used pipe */

    return 1;
}

char *getCWD() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        char *result = malloc(len + 1);
        if (result == NULL) {
            perror("malloc() error");
            exit(-1);
        }
        strcpy(result, cwd);
        return result;
    } else {
        perror("getcwd() error");
        exit(-1);
    }
}

int readLine(int fd, char *str) 
{
    /* Read a single ’\0’-terminated line into str from fd */
    /* Return 0 when the end-of-input is reached and 1 otherwise */
    int n;
    do 
    {   /* Read characters until ’\0’ or end-of-input */
        n = read(fd, str, 1); /* Read one character */ 
    } while (n > 0 && *str++ != '\0');

    return (n > 0); /* Return false if end-of-input */
}