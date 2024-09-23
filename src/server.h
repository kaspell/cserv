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
#define PORT 5000
#define QUEUE_LENGTH 16

typedef struct {
        int sock;
        int id;
        struct sockaddr_in addr;
} Client;

static Client *clients[MAX_CLIENTS];
static int last_id_in_use = -1;


Client *create_client(struct sockaddr_in claddr, int clsock, int id);
int deregister_client(int id);
int register_client(Client *);
void sendall(char *s);
void *serve_client(void *);
int setup_server(struct sockaddr_in *, int *);

#endif