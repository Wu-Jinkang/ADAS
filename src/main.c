#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
// #include "./hmi.c"
#define CENTRAL_ECU "central_ecu"
#define ECU_LOG "./log/ECU.log"
#define NORMAL "NORMALE"
#define ARTIFICIAL "ARTIFICIALE"

#include "./central_ecu.c"


int main(int argc, char *argv[])
{
    int mode;
    // Check input
    if (argv[1] == NULL) 
    {
        printf("Doesn't exist an argument after Main.");
        exit(2);
    }
    if (strcmp(NORMAL, argv[1]) == 0) 
    {
        mode = 0;
    }
    else if (strcmp(ARTIFICIAL, argv[1]) == 0) 
    {
        mode = 1;
    }
    else 
    {
        printf("Modality not valid.");
        exit(2);
    }

    int processes = 2;
    int i;
    for (i = 1; i <= processes; i++) {
        if (fork() == 0) {
            if (i == 1) {
                initCentralECU();
            } else if (i == 2) {
            }
            exit(0);
        }
    }
    int status;
    for (i = 1; i <= processes; i++)
        wait(&status);

    return 0;
}