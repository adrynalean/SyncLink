#include <arpa/inet.h>

#include "network_utils.h"

int send_message(int socket, Message message)
{
    if (send(socket, &message.type, sizeof(message.type), 0) == -1)
        return -1;

    if (send(socket, &message.payload_size, sizeof(message.payload_size), 0) == -1)
        return -1;

    if (message.payload_size != 0)
    {
        uint64_t total_bytes_sent = 0;
        uint64_t bytes_sent;
        while (1)
        {
            if ((bytes_sent = send(socket, message.payload + total_bytes_sent, MIN(message.payload_size - total_bytes_sent, MAX_CHUNK_SIZE), 0)) == -1)
                return -1;

            total_bytes_sent += bytes_sent;
            if (total_bytes_sent >= message.payload_size)
                break;
        }
    }

    return 0;
}

int receive_message(int socket, Message *message_buffer)
{
    if (recv(socket, &(message_buffer->type), sizeof(message_buffer->type), 0) == -1)
        return -1;

    if (recv(socket, &(message_buffer->payload_size), sizeof(message_buffer->payload_size), 0) == -1)
        return -1;

    uint64_t total_bytes_recieved = 0;
    uint64_t bytes_recieved;
    while (message_buffer->payload_size > 0)
    {
        if ((bytes_recieved = recv(socket, message_buffer->payload + total_bytes_recieved, MIN(message_buffer->payload_size - total_bytes_recieved, MAX_CHUNK_SIZE), 0)) == -1)
            return -1;

        total_bytes_recieved += bytes_recieved;
        if (total_bytes_recieved >= message_buffer->payload_size)
            break;
    }

    return 0;
}