#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

typedef struct ClientConnectionAcceptorArguments
{
    int ssid;
} ClientConnectionAcceptorArguments;
void *client_connection_acceptor(void *);

#endif // CLIENT_HANDLER_H