#ifndef server_h
#define server_h

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>


#define BUFFER_OUT_SZ 1024
#define BUFFER_IN_SZ 1024
#define ID_SZ 16
#define MAX_CLIENTS 128

typedef struct {
        int fd;
        int id;
        struct sockaddr_in addr;
} Client;

static Client *clients[MAX_CLIENTS];


void *client_interface(void *);
Client *create_client(struct sockaddr_in cliaddr, int clifd, int id);
int deregister_client(int id);
int register_client(Client *);
void sendall(char *s);

#endif