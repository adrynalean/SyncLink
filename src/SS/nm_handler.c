#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

#include "../Common/network_config.h"
#include "../Common/loggers.h"
#include "nm_handler.h"
#include "../Common/requests.h"
#include "../Common/responses.h"
#include "create.h"
#include "delete.h"

typedef struct NMHandlerArguments
{
    int socket;
    struct sockaddr_in nm_address;
    socklen_t nm_address_size;
    int ssid;
} NMHandlerArguments;

void *nm_handler(void *arguments)
{
    NMHandlerArguments *nm_arguments = (NMHandlerArguments *)arguments;
    Request request_buffer;
    if (receive_request(nm_arguments->socket, &request_buffer) == -1)
    {
        log_errno_error("Error while receiving request: %s\n");
        return NULL;
    }
    char response;
    switch (request_buffer.request_type)
    {
    case CREATE_REQUEST:
        log_info("CREATE_REQUEST", &nm_arguments->nm_address);
        if (request_buffer.request_content.create_request_data.is_folder)
        {
            if (create_folder(request_buffer.request_content.create_request_data.path) == -1||create_backup_folder(request_buffer.request_content.create_request_data.path) == -1)
            {
                response = INTERNAL_ERROR_RESPONSE;
            }
            else
            {
                response = OK_RESPONSE;
            }
        }
        else
        {
            if (create_file(request_buffer.request_content.create_request_data.path) == -1 || create_backup_file(request_buffer.request_content.create_request_data.path) == -1)
            {
                response = INTERNAL_ERROR_RESPONSE;
            }
            else
            {
                response = OK_RESPONSE;
            }
        }
        break;
    case DELETE_REQUEST:
        log_info("DELETE_REQUEST", &nm_arguments->nm_address);
        if (delete_file_or_folder(request_buffer.request_content.delete_request_data.path) == -1 ||delete_file_or_folder_backup(request_buffer.request_content.delete_request_data.path) == -1)
        {
            log_errno_error("Couldn't delete: %s\n");
            response = INTERNAL_ERROR_RESPONSE;
        }
        else
        {
            response = OK_RESPONSE;
        }
        break;
    default:
        response = INVALID_REQUEST_RESPONSE;
        break;
    }

    log_response(response, &nm_arguments->nm_address);
    send_response(nm_arguments->socket, response);
    free(nm_arguments);
    return NULL;
}

void *nm_connection_acceptor(void *arguments)
{
    int ssid = ((NMConnectionAcceptorArguments *)arguments)->ssid;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        log_errno_error("Error while creating NM Handler Socket: %s\n");
        return NULL;
    }
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(SS_NM_HANDLER_BASE_PORT + ssid),
        .sin_addr = {
            .s_addr = inet_addr(SS_NM_HANDLER_IP),
        }};
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        log_errno_error("Error while binding NM Handler Socket: %s\n");
        return NULL;
    }

    if (listen(server_socket, SS_NM_HANDLER_TCP_WAIT_QUEUE) == -1)
    {
        log_errno_error("Error while listening using NM Handler Socket: %s\n");
        return NULL;
    }

    log_info("Listening for NM Connections", &server_address);

    while (1)
    {
        NMHandlerArguments *nm_handler_arguments = malloc(sizeof(NMHandlerArguments));

        if (nm_handler_arguments == NULL)
        {
            log_errno_error("Couldn't malloc: %s\n");
            return NULL;
        }
        nm_handler_arguments->ssid = ssid;
        nm_handler_arguments->socket = accept(server_socket,
                                              (struct sockaddr *)&nm_handler_arguments->nm_address,
                                              &nm_handler_arguments->nm_address_size);
        if (nm_handler_arguments->socket == -1)
        {
            log_errno_error("Error while accepting connection: %s\n");
            continue;
        }
        pthread_t client_handler_thread_id;
        pthread_create(&client_handler_thread_id, NULL, nm_handler, nm_handler_arguments);
    }
}