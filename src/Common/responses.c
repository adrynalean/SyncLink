#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

#include "responses.h"
#include "network_utils.h"
#include "requests.h"

int send_response(int socket, char response_type)
{
    return send(socket, &response_type, 1, 0);
}

int receive_response(int socket, char *response_buffer)
{
    return recv(socket, response_buffer, 1, 0);
}

int send_redirect_response_payload(int socket, struct sockaddr_in *address)
{
    return send(socket, address, sizeof(struct sockaddr_in), 0);
}
int receive_redirect_response_payload(int socket, struct sockaddr_in *address_buffer)
{
    return recv(socket, address_buffer, sizeof(struct sockaddr_in), 0);
}

int send_streaming_response_payload(int socket, char *data, uint64_t size)
{
    Message message = {.type = MESSAGE_TYPE_STREAM,
                       .payload_size = size,
                       .payload = data};
    return send_message(socket, message);
}

int end_streaming_response_payload(int socket)
{
    char dummy_payload = '\0';
    Message message = {.type = MESSAGE_TYPE_STREAM_END,
                       .payload_size = 1,
                       .payload = &dummy_payload};

    return send_message(socket, message);
}

int receive_streaming_response_payload(int socket, char *data_buffer)
{
    Message message = {.payload = data_buffer};
    if (receive_message(socket, &message))
    {
        return -1;
    }
    else if (message.type == MESSAGE_TYPE_STREAM)
    {
        return (int)message.payload_size;
    }
    else if (message.type == MESSAGE_TYPE_STREAM_END)
    {
        return 0;
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
}

int send_copied_paths(int socket, uint64_t copied_paths_count, char paths[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH])
{
    if (send(socket, &copied_paths_count, sizeof(copied_paths_count), 0) == -1)
        return -1;
    uint64_t total_bytes_sent = 0, bytes_sent;
    while (1)
    {
        if ((bytes_sent = send(socket, paths + total_bytes_sent, MIN(MAX_PATH_LENGTH * MAX_ACCESIBLE_PATHS - total_bytes_sent, MAX_CHUNK_SIZE), 0)) == -1)
            return -1;

        total_bytes_sent += bytes_sent;
        if (total_bytes_sent >= MAX_PATH_LENGTH * MAX_ACCESIBLE_PATHS)
            break;
    }

    return 0;
}

int receive_copied_paths(int socket, uint64_t *copied_paths_count_buffer, char path_buffer[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH])
{
    if (recv(socket, copied_paths_count_buffer, sizeof(uint64_t), 0) == -1)
        return -1;
    uint64_t total_bytes_received = 0, bytes_received;
    while (1)
    {
        if ((bytes_received = recv(socket, path_buffer + total_bytes_received, MIN(MAX_PATH_LENGTH * MAX_ACCESIBLE_PATHS - total_bytes_received, MAX_CHUNK_SIZE), 0)) == -1)
            return -1;
        total_bytes_received += bytes_received;
        if (total_bytes_received >= MAX_PATH_LENGTH * MAX_ACCESIBLE_PATHS)
            break;
    }
    return 0;
}
