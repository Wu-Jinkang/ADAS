#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

int getCWD();

int main (void) 
{
    getCWD();
    int fd, messageLen, i, pid;
    char message [100];

    pid = getpid();

    sprintf(message,"Hello from PID %d", pid); /* Prepare message */

    messageLen = strlen(message) + 1;
    int c = 0;
    do 
    { /* Keep trying to open the file until successful */
        fd = open("aPipe", O_WRONLY); /* Open named pipe for writing */
        printf("Attempt to open aPipe %d. (%d)\n", fd, c++);
        if (fd == -1)
            sleep(1); /* Try again in 1 second */ 
    } while (fd == -1);

    for (i = 1; i <= 10; i++) 
    {   /* Send three messages */
        /* Write message down pipe */
        int res = write(fd, message, messageLen);
        printf("Attempt to write on aPipe %d. counter(%d) status(%d)\n", fd, i, res);
        sleep(3); /* Pause a while */
    }
    close(fd); /* Close pipe descriptor */
    return 0; 
}

int getCWD() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) 
    {
        printf("Current working dir: %s\n", cwd);
    } 
    else 
    {
        perror("getcwd() error");
        return 1;
    }
    return 0;
}