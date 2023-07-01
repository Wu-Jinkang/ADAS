#ifndef CONN_H
#define CONN_H

int initServerSocket(void);
int connectToServer(void);
int readLine(int fd, char *str);

#endif
