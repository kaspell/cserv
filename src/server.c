#include "server.h"


extern int clcnt;
int top_id = -1;
pthread_mutex_t cli_mutex = PTHREAD_MUTEX_INITIALIZER;


Client *
add_client(struct sockaddr_in claddr, int clsock)
{
        Client *client = (Client *) malloc(sizeof(Client));
        client->addr = claddr;
        client->sock = clsock;
        client->id = ++top_id;

        pthread_mutex_lock(&cli_mutex);
        for (int i=0; i<MAX_CLIENTS; i++)
                if (!clients[i]) {
                        clients[i] = client;
                        ++clcnt;
                        break;
                }
        pthread_mutex_unlock(&cli_mutex);

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
                        --clcnt;
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
        return 1;
}