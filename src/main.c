#include "server.h"


#define PORT 5000
#define QUEUE_LENGTH 16
#define THROTTLE_LAG 1

socklen_t SOCKADDR_SZ = sizeof(struct sockaddr_in);
int last_id_in_use = -1;


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

int
main(int argc, char *argv[])
{
        int clisock = 0, svsock = 0, clicnt = 0;
        struct sockaddr_in cliaddr, servaddr;
        pthread_t tid;

        setup_server(&servaddr, &svsock);

        while (1) {
                if (clicnt == MAX_CLIENTS)
                        continue;
                clisock = accept(svsock, (struct sockaddr*)&cliaddr, (socklen_t*)&SOCKADDR_SZ);
                if (clisock < 0) {
                        perror("accept");
                        return EXIT_FAILURE;
                }
                Client *client = create_client(cliaddr, clisock, ++last_id_in_use);
                clicnt++;
                pthread_create(&tid, NULL, &client_interface, (void*)client);

                sleep(THROTTLE_LAG);
        }
        return EXIT_SUCCESS;
}