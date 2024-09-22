#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>


const int MAX_CLIENTS = 128;
const int ID_SZ = 16;
const int THROTTLE_LAG = 1;
const socklen_t SOCKADDR_SZ = sizeof(struct sockaddr_in);

typedef struct {
        char id[ID_SZ]; /* ID displayed next to the messages */
        int fd;
        struct sockaddr_in addr;
} Client;

Client *clients[MAX_CLIENTS];


/* Add a client to the list of clients.
   First confirm that the client ID is not already taken. */
int
add(Client *client)
{
        int avail_idx = -1;
        for (int i=0; i<MAX_CLIENTS; i++) {
                if (!clients[i] && !strmp(clients[i]->id, client->id))
                        return 0; /* ID is already in use */
                if (avail_idx<0 && !clients[i])
                        avail_idx = i;
        }
        if (avail_idx > 0) {
                clients[avail_idx] = client;
                return 1;
        }
        return 0;
}

int
remove(char id[])
{
        for (int i=0; i<MAX_CLIENTS; i++)
                if (clients[i] && !strcmp(clients[i]->id, id)) {
                        clients[i] = 0;
                        return 1;
                }
        return 0;
}

/* Broadcast a message to every client */
void
sendall(char *s)
{
        int nbytes = 0;
        for (int i=0; i<MAX_CLIENTS; i++)
                if (clients[i]) {
                        nbytes = write(clients[i]->fd, s, strlen(s)+1);
                        if (nbytes < 0)
                                exit(EXIT_FAILURE);
                }
}

int
main(int argc, char *argv[])
{
        int clifd = 0, servfd = 0, clicnt = 0;
        struct sockaddr_in cliaddr, servaddr;

        servfd = socket(AF_INET, SOCK_STREAM, 0);
        if (servfd < 0)
                return EXIT_FAILURE; /* Socket creation failed */

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = 2048;

        while (1) {
                if (clicnt == MAX_CLIENTS)
                        continue;
                clifd = accept(servfd, (struct sockaddr*)&cliaddr, (socklen_t*)&SOCKADDR_SZ);
                sleep(THROTTLE_LAG);
        }
        return EXIT_SUCCESS;
}