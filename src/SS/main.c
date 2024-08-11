#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "initialize.h"
#include "nm_handler.h"
#include "client_handler.h"

int main(int argc, char *argv[argc])
{
    if (argc < 2)
    {
        printf("ssid argument required\n");
        return -1;
    }
    initialize(argc, argv);
    ClientConnectionAcceptorArguments client_arguments = {.ssid = atoi(argv[1])};
    NMConnectionAcceptorArguments nm_arguments = {.ssid = atoi(argv[1])};
    pthread_t nm_acceptor_thread_id, client_acceptor_thread_id;
    pthread_create(&nm_acceptor_thread_id, NULL, nm_connection_acceptor, &client_arguments);     // TODO pass ssid
    pthread_create(&client_acceptor_thread_id, NULL, client_connection_acceptor, &nm_arguments); // TODO pass ssid

    pthread_join(nm_acceptor_thread_id, NULL);
    pthread_join(client_acceptor_thread_id, NULL);
}