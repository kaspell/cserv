#ifndef server_h
#define server_h

#include <netinet/in.h>


typedef struct {
        int sock;
        int id;
        struct sockaddr_in addr;
} Client;


int accept_client_connection(int *svsock, int *clsock, struct sockaddr_in *claddr, socklen_t *sockaddr_sz);
Client *add_client(struct sockaddr_in claddr, int clsock);
void *serve_client(void *);
int setup_server(struct sockaddr_in *, int *);

#endif