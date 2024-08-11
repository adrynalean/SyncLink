#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "copy_request_handler.h"
#include "../ss_info.h"
#include "../../Common/requests.h"
#include "../../Common/loggers.h"
#include "../../Common/path_utils.h"

char copy_request_handler(char *source_path, char *destination_path)
{
    int source_ss_id = get_ss_id_of_path(source_path);
    if (source_ss_id == -1)
    {
        return NOT_FOUND_RESPONSE;
    }

    int destination_ss_id = get_ss_id_of_path(destination_path);
    if (destination_ss_id != -1)
    {
        return ALREADY_EXISTS_RESPONSE;
    }

    char *parent = malloc(strlen(destination_path) + 1);
    get_parent(destination_path, parent);
    if (*parent != '\0')
    {
        destination_ss_id = get_ss_id_of_path(parent);
        if (destination_ss_id == -1)
        {
            return NOT_FOUND_RESPONSE;
        }
    }
    else
    {
        destination_ss_id = get_random_registered_ss_id();
        if (destination_ss_id == -1)
        {
            return INTERNAL_ERROR_RESPONSE;
        }
    }
    free(parent);
    // connect to client handler as a client as client handler of ss has copy handler
    struct sockaddr_in connection_address = get_client_connection_address(source_ss_id);
    int connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connection_socket == -1)
    {
        log_errno_error("Couldn't create socket: %s\n");
        return INTERNAL_ERROR_RESPONSE;
    }

    if (connect(connection_socket, (struct sockaddr *)&connection_address, sizeof(connection_address)) == -1)
    {
        log_errno_error("Couldn't connect to ss: %s\n");
        return INTERNAL_ERROR_RESPONSE;
    }

    if (send_copy_request(connection_socket, source_path, destination_path) == -1)
    {
        log_errno_error("Couldn't send copy request: %s\n");
        return INTERNAL_ERROR_RESPONSE;
    }

    struct sockaddr_in destination_address = get_client_connection_address(destination_ss_id);
    if (send_redirect_response_payload(connection_socket, &destination_address) == -1)
    {
        log_errno_error("Couldn't send destination address: %s\n");
        return INTERNAL_ERROR_RESPONSE;
    }

    char response;
    if (receive_response(connection_socket, &response) == -1)
    {
        log_errno_error("Couldn't receive response: %s\n");
        return INTERNAL_ERROR_RESPONSE;
    }

    if (response == OK_RESPONSE)
    {
        char copied_paths[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH];
        uint64_t copied_paths_count;
        if (receive_copied_paths(connection_socket, &copied_paths_count, copied_paths) == -1)
            return INTERNAL_ERROR_RESPONSE;

        // TODO maybe dont release lock after each add
        for (uint64_t i = 0; i < copied_paths_count; i++)
            add_path(destination_ss_id, copied_paths[i]);
    }

    return response;
}