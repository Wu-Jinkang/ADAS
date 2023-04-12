#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void function1() {
    printf("Executing function 1\n");
}

void function2() {
    printf("Executing function 2\n");
}

int main(int argc, char *argv[])
{
    int processes = 2;
    int i;
    for (i = 1; i <= processes; i++) {
        if (fork() == 0) {
            if (i == 1) {
                function1();
            } else if (i == 2) {
                function2();
            }
            exit(0);
        }
    }
    int status;
    for (i = 1; i <= processes; i++)
        wait(&status);

    return 0;
}