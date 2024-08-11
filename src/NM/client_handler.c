#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

#include "client_handler.h"
#include "../Common/requests.h"
#include "../Common/responses.h"
#include "../Common/loggers.h"
#include "../Common/network_config.h"
#include "request_handlers/create_request_handler.h"
#include "request_handlers/delete_request_handler.h"
#include "request_handlers/read_request_handler.h"
#include "request_handlers/file_info_handler.c"
#include "request_handlers/copy_request_handler.h"
#include "request_handlers/list_request_handler.c"
typedef struct ClientHandlerArguments
{
    int socket;
    struct sockaddr_in client_address;
    socklen_t client_address_size;
} ClientHandlerArguments;

void *client_handler(void *client_handler_arguments_raw)
{
    ClientHandlerArguments *client_handler_arguments = (ClientHandlerArguments *)client_handler_arguments_raw;
    Request request_buffer;
    if (receive_request(client_handler_arguments->socket, &request_buffer) == -1)
    {
        log_errno_error("Error while receiving request: %s\n");
        return NULL;
    }
    char response;
    switch (request_buffer.request_type)
    {
    case CREATE_REQUEST:
        log_info("CREATE_REQUEST", &client_handler_arguments->client_address);
        response = create_request_handler(
            request_buffer.request_content.create_request_data.path,
            request_buffer.request_content.create_request_data.is_folder);
        log_response(response, &client_handler_arguments->client_address);
        if (send_response(client_handler_arguments->socket, response) == -1)
        {
            log_errno_error("Couldn't send response: %s\n");
        }
        break;

    case DELETE_REQUEST:
        log_info("DELETE_REQUEST", &client_handler_arguments->client_address);
        response = delete_request_handler(
            request_buffer.request_content.delete_request_data.path);
        log_response(response, &client_handler_arguments->client_address);
        if (send_response(client_handler_arguments->socket, response) == -1)
        {
            log_errno_error("Couldn't send response: %s\n");
        }
        break;

    case READ_REQUEST:
        log_info("READ_REQUEST", &client_handler_arguments->client_address);
        read_request_handler(client_handler_arguments->socket, &client_handler_arguments->client_address, request_buffer.request_content.read_request_data.path);
        break;
    case WRITE_REQUEST:
        log_info("WRITE_REQUEST", &client_handler_arguments->client_address);
        // same as read, redirect
        read_request_handler(client_handler_arguments->socket, &client_handler_arguments->client_address, request_buffer.request_content.write_request_data.path);
        break;
    case FILE_INFO:
        log_info("FILE_INFO", &client_handler_arguments->client_address);
        // same as read, redirect
        file_info_request_handler(client_handler_arguments->socket, &client_handler_arguments->client_address, request_buffer.request_content.file_info_request_data.path);
        break;
    case COPY_REQUEST:
        log_info("COPY_REQUEST", &client_handler_arguments->client_address);
        char *source_path = request_buffer.request_content.copy_request_data.source_path;
        char *destination_path = request_buffer.request_content.copy_request_data.destination_path;
        response = copy_request_handler(source_path, destination_path);
        log_response(response, &client_handler_arguments->client_address);
        if (send_response(client_handler_arguments->socket, response) == -1)
            log_errno_error("Couldn't send response: %s\n");
        break;
    case GET_LIST:
        log_info("GET_LIST", &client_handler_arguments->client_address);
        // same as read, redirect
        list_request_handler(client_handler_arguments->socket, &client_handler_arguments->client_address, request_buffer.request_content.get_list_request_data.path);
        break;
    default:
        log_info("INVALID_REQUEST_RESPONSE", &client_handler_arguments->client_address);
        send_response(client_handler_arguments->socket, INVALID_REQUEST_RESPONSE);
        break;
    }

    free(client_handler_arguments);
    return NULL;
}

void *client_connection_acceptor_thread()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        log_errno_error("Error while creating Client Handler Socket: %s\n");
        return NULL;
    }
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(NM_CLIENT_HANDLER_SERVER_PORT),
        .sin_addr = {
            .s_addr = inet_addr(NM_CLIENT_HANDLER_SERVER_IP),
        }};
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        log_errno_error("Error while binding Client Handler Socket: %s\n");
        return NULL;
    }

    if (listen(server_socket, NM_CLIENT_HANDLER_TCP_WAIT_QUEUE_LENGTH) == -1)
    {
        log_errno_error("Error while listening using Client Handler Socket: %s\n");
        return NULL;
    }

    log_info("Listening for Client Connections", &server_address);

    while (1)
    {
        ClientHandlerArguments *client_handler_arguments = malloc(sizeof(ClientHandlerArguments));
        if (client_handler_arguments == NULL)
        {
            log_errno_error("Couldn't malloc: %s\n");
            return NULL;
        }
        client_handler_arguments->socket = accept(server_socket,
                                                  (struct sockaddr *)&client_handler_arguments->client_address,
                                                  &client_handler_arguments->client_address_size);
        if (client_handler_arguments->socket == -1)
        {
            log_errno_error("Error while accepting connection: %s\n");
            continue;
        }
        pthread_t client_handler_thread_id;
        pthread_create(&client_handler_thread_id, NULL, client_handler, client_handler_arguments);
    }
}
