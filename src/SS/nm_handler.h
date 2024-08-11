#ifndef NM_HANDLER_H
#define NM_HANDLER_H

typedef struct NMConnectionAcceptorArguments
{
    int ssid;
} NMConnectionAcceptorArguments;
void *nm_connection_acceptor(void *);

#endif