#include <stdbool.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "../ss_info.h"
#include "../../Common/responses.h"
#include "../../Common/loggers.h"
#include "../../Common/path_utils.h"

char create_request_handler(char *path, bool is_folder)
{
    if (*path == '\0' || *path == '.' || *path == '/' || *get_basename(path) == '\0')
    {
        return INVALID_REQUEST_CONTENT_RESPONSE;
    }

    if (get_ss_id_of_path(path) != -1)
    {
        return ALREADY_EXISTS_RESPONSE;
    }

    char *parent = malloc(strlen(path) + 1);
    get_parent(path, parent);
    int ss_id;
    if (*parent != '\0')
    {
        ss_id = get_ss_id_of_path(parent);
        if (ss_id == -1)
        {
            return NOT_FOUND_RESPONSE;
        }
    }
    else
    {
        ss_id = get_random_registered_ss_id();
        if (ss_id == -1)
        {
            return INTERNAL_ERROR_RESPONSE;
        }
    }
    struct sockaddr_in connection_address = get_nm_connection_address(ss_id);
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

    if (send_create_request(connection_socket, path, is_folder) == -1)
    {
        log_errno_error("Couldn't send create request: %s\n");
        return INTERNAL_ERROR_RESPONSE;
    }

    char response;
    if (receive_response(connection_socket, &response) == -1)
    {
        log_errno_error("Couldn't receive response: %s\n");
        return INTERNAL_ERROR_RESPONSE;
    }

    if (response == OK_RESPONSE){
        add_path(ss_id, path);
    }

    return response;
}