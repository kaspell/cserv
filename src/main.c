#include "server.h"


#define THROTTLE_LAG 1

int clcnt = 0;
socklen_t SOCKADDR_SZ = sizeof(struct sockaddr_in);


int
main(int argc, char *argv[])
{
        int clsock = 0, svsock = 0;
        struct sockaddr_in claddr, svaddr;
        pthread_t tid;

        setup_server(&svaddr, &svsock);

        printf(" * Started server\n");
        printf(" * Running on port %d\n", PORT);
        fflush(stdout);

        while (1) {
                sleep(THROTTLE_LAG);
                if ((clsock = accept(svsock, (struct sockaddr*)&claddr, (socklen_t*)&SOCKADDR_SZ)) < 0) {
                        perror("accept");
                        return EXIT_FAILURE;
                } else if (clcnt >= MAX_CLIENTS) {
                        close(clsock);
                        clsock = 0;
                        continue;
                }
                Client *client = add_client(claddr, clsock);
                pthread_create(&tid, NULL, &serve_client, (void*)client);
        }
        return EXIT_SUCCESS;
}