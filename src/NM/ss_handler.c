#include <arpa/inet.h>

#include "ss_handler.h"
#include "ss_info.h"
#include "../Common/requests.h"
#include "../Common/responses.h"
#include "../Common/loggers.h"
#include "../Common/network_config.h"

#include <stdio.h>

void *ss_handler_thread()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        log_errno_error("Error while creating Client Handler Socket: %s\n");
        return NULL;
    }
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(NM_SS_HANDLER_SERVER_PORT),
        .sin_addr = {
            .s_addr = inet_addr(NM_SS_HANDLER_SERVER_IP),
        }};
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        log_errno_error("Error while binding SS Handler Socket: %s\n");
        return NULL;
    }

    if (listen(server_socket, NM_SS_HANDLER_TCP_WAIT_QUEUE_LENGTH) == -1)
    {
        log_errno_error("Error while listening using SS Handler Socket: %s\n");
        return NULL;
    }

    log_info("Listening for SS Connections", &server_address);

    while (1)
    {
        struct sockaddr_in ss_address;
        socklen_t ss_address_size;
        int client_socket = accept(server_socket, (struct sockaddr *)&ss_address, &ss_address_size);
        if (client_socket == -1)
        {
            log_errno_error("Error while accepting connection: %s\n");
            continue;
        }
        Request request_buffer;
        if (receive_request(client_socket, &request_buffer) == -1)
        {
            log_errno_error("Error while receiving request: %s\n");
            continue;
        }
        switch (request_buffer.request_type)
        {
        case SS_REGISTER_REQUEST:

            log_info("SS_REGISTER_REQUEST", &ss_address);
            register_ss(
                request_buffer.request_content.ss_register_data.ss_id,
                request_buffer.request_content.ss_register_data.nm_connection_address,
                request_buffer.request_content.ss_register_data.client_connection_address,
                request_buffer.request_content.ss_register_data.accessible_paths_count,
                request_buffer.request_content.ss_register_data.accessible_paths);
            log_info("Registered SS", &ss_address);
            log_info("OK_RESPONSE", &ss_address);
            send_response(client_socket, OK_RESPONSE);
            break;
        default:
            log_info("INVALID_REQUEST_RESPONSE", &ss_address);
            send_response(client_socket, INVALID_REQUEST_RESPONSE);
            break;
        }
    }
}