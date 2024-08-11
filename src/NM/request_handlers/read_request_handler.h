#ifndef READ_REQUEST_HANDLER_H
#include <arpa/inet.h>

void read_request_handler(int socket, struct sockaddr_in *address, char *path);

#define READ_REQUEST_HANDLER_H
#endif