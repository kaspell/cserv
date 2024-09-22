#include "server.h"


#define PORT 5000
#define QUEUE_LENGTH 16
#define THROTTLE_LAG 1

const socklen_t SOCKADDR_SZ = sizeof(struct sockaddr_in);
int prev_id = -1;


int
main(int argc, char *argv[])
{
        int clifd = 0, servfd = 0, clicnt = 0;
        struct sockaddr_in cliaddr, servaddr;
        pthread_t tid;

        servfd = socket(AF_INET, SOCK_STREAM, 0);
        if (servfd < 0) {
                perror("socket");
                return EXIT_FAILURE; /* Socket creation failed */
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(PORT);

        if (bind(servfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
                perror("bind");
                return EXIT_FAILURE;
        }

        if (listen(servfd, QUEUE_LENGTH) < 0) {
                perror("listen");
                return EXIT_FAILURE;
        }

        while (1) {
                if (clicnt == MAX_CLIENTS)
                        continue;
                socklen_t clisz = sizeof(cliaddr);
                clifd = accept(servfd, (struct sockaddr*)&cliaddr, (socklen_t*)&clisz);
                if (clifd < 0) {
                        perror("accept");
                        return EXIT_FAILURE;
                }
                Client *client = create_client(cliaddr, clifd, ++prev_id);
                clicnt++;
                register_client(client);
                pthread_create(&tid, NULL, &client_interface, (void*)client);

                sleep(THROTTLE_LAG);
        }
        return EXIT_SUCCESS;
}