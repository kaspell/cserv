#include "server.h"

#define THROTTLE_LAG 1

int clicnt = 0;
socklen_t SOCKADDR_SZ = sizeof(struct sockaddr_in);


int
main(int argc, char *argv[])
{
        int clisock = 0, svsock = 0;
        struct sockaddr_in cliaddr, servaddr;
        pthread_t tid;

        setup_server(&servaddr, &svsock);

        while (1) {
                sleep(THROTTLE_LAG);
                clisock = accept(svsock, (struct sockaddr*)&cliaddr, (socklen_t*)&SOCKADDR_SZ);
                if (clicnt >= MAX_CLIENTS) {
                        close(clisock);
                        continue;
                }
                if (clisock < 0) {
                        perror("accept");
                        return EXIT_FAILURE;
                }

                Client *client = create_client(cliaddr, clisock, ++last_id_in_use);
                ++clicnt;
                pthread_create(&tid, NULL, &serve_client, (void*)client);
        }
        return EXIT_SUCCESS;
}