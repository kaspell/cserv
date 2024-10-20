#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"


pthread_mutex_t cli_mutex = PTHREAD_MUTEX_INITIALIZER;


typedef enum {
        GET,
        INCREMENT,
        DECREMENT
} Mode;

int
manage_clcnt(Mode mode)
{
        static int clcnt = 0;
        switch (mode) {
                case GET:
                        return clcnt;
                case INCREMENT:
                        ++clcnt;
                        break;
                case DECREMENT:
                        --clcnt;
                        break;
        }
        return -1;
}

int
next_id_to_assign()
{
        static int topid = -1;
        return ++topid;
}

int
accept_client_connection(int *svsock, int *clsock, struct sockaddr_in *claddr, socklen_t *sockaddr_sz)
{
        if ((*clsock = accept(*svsock, (struct sockaddr*)claddr, (socklen_t*)sockaddr_sz)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
        } else if (manage_clcnt(GET) >= MAX_CLIENTS) {
                close(*clsock);
                *clsock = 0;
                return -1;
        }
        return 0;
}

Client *
add_client(struct sockaddr_in claddr, int clsock)
{
        Client *client = (Client *) malloc(sizeof(Client));
        client->addr = claddr;
        client->sock = clsock;
        client->id = next_id_to_assign();

        for (int i=0; i<MAX_CLIENTS; i++)
                if (!clients[i]) {
                        clients[i] = client;
                        pthread_mutex_lock(&cli_mutex);
                        manage_clcnt(INCREMENT);
                        pthread_mutex_unlock(&cli_mutex);
                        break;
                }
        return client;
}

void
remove_client(Client *client)
{
        close(client->sock);
        client->sock = -1;

        pthread_mutex_lock(&cli_mutex);
        for (int i=0; i<MAX_CLIENTS; i++)
                if (clients[i] && clients[i]->id == client->id) {
                        clients[i] = 0;
                        manage_clcnt(DECREMENT);
                        break;
                }
        pthread_mutex_unlock(&cli_mutex);

        free(client);
        client = NULL;
}


void
sendall(char *s)
{
        pthread_mutex_lock(&cli_mutex);
        for (int i=0; i<MAX_CLIENTS; i++)
                if (clients[i] && write(clients[i]->sock, s, strlen(s)) < 0) {
                        perror("write");
                        break;
                }
        pthread_mutex_unlock(&cli_mutex);
}

void *
serve_client(void *clptr)
{
        int rbytes = 0;
        char buffer_out[BUFFER_OUT_SZ];
        char buffer_in[BUFFER_IN_SZ];
        Client *client = (Client *) clptr;

        sprintf(buffer_out, "%d joined the channel\n", client->id);
        sendall(buffer_out);

        while ((rbytes = read(client->sock, buffer_in, sizeof(buffer_in)-1)) > 0) {
                buffer_in[rbytes] = buffer_out[0] = '\0';
                if (!rbytes)
                        continue;
                else if ((rbytes >= 4) && !strncmp(buffer_in, "exit", 4))
                        break;
                else {
                        snprintf(buffer_out, sizeof(buffer_out), "[%d] %d: %s", (int)time(NULL), client->id, buffer_in);
                        sendall(buffer_out);
                }
        }
        sprintf(buffer_out, "%d exited the channel\n", client->id);
        sendall(buffer_out);
        remove_client(client);
        pthread_detach(pthread_self());

        return NULL;
}

int
setup_server(struct sockaddr_in *svaddr, int *svsock)
{
        if ((*svsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                exit(EXIT_FAILURE);
        }

        svaddr->sin_family = AF_INET;
        svaddr->sin_addr.s_addr = htonl(INADDR_ANY);
        svaddr->sin_port = htons(PORT);

        if (bind(*svsock, (struct sockaddr*)svaddr, sizeof(*svaddr)) < 0) {
                perror("bind");
                exit(EXIT_FAILURE);
        }
        if (listen(*svsock, QUEUE_LENGTH) < 0) {
                perror("listen");
                exit(EXIT_FAILURE);
        }
        printf(" * Started server\n");
        printf(" * Running on port %d\n", PORT);
        fflush(stdout);
        return 1;
}