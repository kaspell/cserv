#ifndef server_h
#define server_h

#include <netinet/in.h>
#include <pthread.h>


#define BUFFER_OUT_SZ 1024
#define BUFFER_IN_SZ 1024
#define ID_SZ 16
#define MAX_CLIENTS 128

typedef struct {
        int fd;
        int id;
        struct sockaddr_in addr;
} Client;

Client *clients[MAX_CLIENTS];

pthread_mutex_t cli_mutex = PTHREAD_MUTEX_INITIALIZER;


int add_client(Client *);
void *client_interface(void *);
int remove_client(int id);
void sendall(char *s);

#endif