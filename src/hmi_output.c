#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#define ECU_LOG "./log/ECU.log"

int readLineFromIndex(int fd, char *str, int *index);

int main(void)
{
    int fd, len, n;
    size_t str_len = 20;
    char *str = (char *)calloc(str_len, sizeof(char));

    len = 0;

    do
    {
        memset(str, 0, str_len);
        fd = open(ECU_LOG, O_RDONLY);
        if (fd == -1)
        {
            perror("open() error");
            return -1;
        }

        // Get file string current length
        off_t currentPos = lseek(fd, (size_t)0, SEEK_END);

        n = readLineFromIndex(fd, str, &len);
        close(fd);
        if (currentPos > len)
        {
            len = currentPos;
            printf("%s", str);
        }
    } while (strcmp(str, "ARRESTO\n") != 0);

    free(str);
    return 0;
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