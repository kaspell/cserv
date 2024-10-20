#include "server.h"


#define THROTTLE_LAG 1

socklen_t SOCKADDR_SZ = sizeof(struct sockaddr_in);


int
main(int argc, char *argv[])
{
        int clsock = 0, svsock = 0;
        struct sockaddr_in claddr, svaddr;
        pthread_t tid;

        setup_server(&svaddr, &svsock);

        while (1) {
                sleep(THROTTLE_LAG);
                if (accept_client_connection(&svsock, &clsock, &claddr, &SOCKADDR_SZ) < 0)
                        continue;
                Client *client = add_client(claddr, clsock);
                pthread_create(&tid, NULL, &serve_client, (void*)client);
        }
        return EXIT_SUCCESS;
}