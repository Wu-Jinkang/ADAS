#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define CENTRAL_ECU "./log/central_ecu"

int main(void)
{
    char input[1024];
    do
    {
        printf("Enter a command: ");

        if (scanf("%s", input) == EOF)
        {
            break;
        }

        if (strcmp(input, "INIZIO") == 0 || strcmp(input, "PARCHEGGIO") == 0 || strcmp(input, "ARRESTO") == 0)
        {
            int fd = open(CENTRAL_ECU, O_WRONLY);
            if (fd == -1)
            {
                perror("open() error");
                return -1;
            }
            size_t len = strlen(input);
            if (write(fd, input, len) == -1)
            {
                perror("write() error");
                close(fd);
                return -1;
            }
            close(fd);
            if (strcmp(input, "ARRESTO") == 0)
            {
                break;
            }
            memset(input, 0, sizeof(input));
        }
    } while (1);

    return 0;
}