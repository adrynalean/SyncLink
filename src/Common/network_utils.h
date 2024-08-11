#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <stdint.h>

#define MESSAGE_TYPE_REQUEST '0'
#define MESSAGE_TYPE_SS_REGISTER_PAYLOAD 'S'
#define MESSAGE_TYPE_STREAM '1'
#define MESSAGE_TYPE_STREAM_END '2'

#define MAX_CHUNK_SIZE 1024
#define MIN(x, y) ((x) < (y) ? (x) : (y))

typedef struct Message
{
    char type;
    uint64_t payload_size;
    void *payload;
} Message;

int send_message(int socket, Message);
int receive_message(int socket, Message *message_buffer);

#endif // NETWORK_UTILS_H