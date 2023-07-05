#ifndef CONN_H
#define CONN_H

int initServerSocket(void);
int connectToServer(void);
int readLine(int fd, char *str);
void sendComponentName(int clientFd, char *name);
struct Component connectToComponent(int centralFd);
void sendOk(int clientFd);
void waitOk(int clientFd);

#endif
