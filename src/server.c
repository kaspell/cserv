#include "server.h"


extern int clicnt;
pthread_mutex_t cli_mutex = PTHREAD_MUTEX_INITIALIZER;


Client *
create_client(struct sockaddr_in cliaddr, int clifd, int id)
{
        Client *client = (Client *) malloc(sizeof(Client));
        client->addr = cliaddr;
        client->fd = clifd;
        client->id = id;
        register_client(client);
        return client;
}

void
remove_client(Client *client)
{
        close(client->fd);
        client->fd = -1;
        deregister_client(client->id);
        free(client);
        client = NULL;
        pthread_mutex_lock(&cli_mutex);
        clicnt--;
        pthread_mutex_unlock(&cli_mutex);
}

/* Add a client to the array of clients */
int
register_client(Client *client)
{
        pthread_mutex_lock(&cli_mutex);
        int added = 0;
        for (int i=0; i<MAX_CLIENTS; i++)
                if (!clients[i]) {
                        clients[i] = client;
                        added = 1;
                        break;
                }
        pthread_mutex_unlock(&cli_mutex);
        return added;
}

/* Remove a client from the array of clients */
int
deregister_client(int id)
{
        pthread_mutex_lock(&cli_mutex);
        int removed = 0;
        for (int i=0; i<MAX_CLIENTS; i++)
                if (clients[i] && clients[i]->id == id) {
                        clients[i] = 0;
                        removed = 1;
                        break;
                }
        pthread_mutex_unlock(&cli_mutex);
        return removed;
}

/* Broadcast a message to every client */
void broadcast(char *s)
{

}

void
sendall(char *s)
{
        pthread_mutex_lock(&cli_mutex);
        int nbytes = 0;
        for (int i=0; i<MAX_CLIENTS; i++)
                if (clients[i] && write(clients[i]->fd, s, strlen(s)) < 0) {
                        perror("write");
                        break;
                }
        pthread_mutex_unlock(&cli_mutex);
}

void *
serve_client(void *cliptr)
{
        int rbytes = 0;
        char buffer_out[BUFFER_OUT_SZ];
        char buffer_in[BUFFER_IN_SZ];
        Client *client = (Client *) cliptr;

        sprintf(buffer_out, "%d joined the channel\n", client->id);
        sendall(buffer_out);

        while ((rbytes = read(client->fd, buffer_in, sizeof(buffer_in)-1)) > 0) {
                buffer_in[rbytes] = '\0';
                buffer_out[0] = '\0';

                if (!rbytes)
                        continue;
                else if ((rbytes >= 4) && buffer_in[0] == 'e' && buffer_in[1] == 'x' && buffer_in[2] == 'i' && buffer_in[3] == 't')
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
setup_server(struct sockaddr_in *servaddr, int *svsock)
{
        *svsock = socket(AF_INET, SOCK_STREAM, 0);
        if (*svsock < 0) {
                perror("socket");
                exit(EXIT_FAILURE); /* Socket creation failed */
        }

        servaddr->sin_family = AF_INET;
        servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr->sin_port = htons(PORT);

        if (bind(*svsock, (struct sockaddr*)servaddr, sizeof(*servaddr)) < 0) {
                perror("bind");
                exit(EXIT_FAILURE);
        }

        if (listen(*svsock, QUEUE_LENGTH) < 0) {
                perror("listen");
                exit(EXIT_FAILURE);
        }
        return 1;
}