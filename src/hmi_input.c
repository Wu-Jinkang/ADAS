#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define CENTRAL_ECU "central_ecu"

int main (void) 
{
    char input[1024];
    int fd = open(CENTRAL_ECU, O_WRONLY);
    if (fd == -1) {
        perror("open() error");
        return -1;
    }

    while (1) {
        printf("Enter a command: ");
        
        if (scanf("%s", input) == EOF) {
            break;
        }

        if (strcmp(input, "INIZIO") == 0 || strcmp(input, "PARCHEGGIO") == 0 || strcmp(input, "ARRESTO") == 0) {
            size_t len = strlen(input);
            if (write(fd, input, len) == -1) {
                perror("write() error");
                close(fd);
                return -1;
            }
        }
    }

    close(fd);
    return 0; 
}