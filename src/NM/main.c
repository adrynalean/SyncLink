#include <stdio.h>
#include <pthread.h>

#include "client_handler.h"
#include "ss_handler.h"

int main()
{

    pthread_t client_connection_acceptor_thread_id;
    pthread_create(&client_connection_acceptor_thread_id, NULL, client_connection_acceptor_thread, NULL);

    pthread_t ss_handler_thread_id;
    pthread_create(&ss_handler_thread_id, NULL, ss_handler_thread, NULL);

    pthread_join(client_connection_acceptor_thread_id, NULL);
    pthread_join(ss_handler_thread_id, NULL);
    
}