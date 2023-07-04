#ifndef UTIL_H
#define UTIL_H

int readLineFromIndex(int fd, char *str, int *index);
int writeln(int fd, char *str);
int readLine(int fd, char *str);
char* getDataSrc(char *mode);
int read8(int fd, char *str);

#endif