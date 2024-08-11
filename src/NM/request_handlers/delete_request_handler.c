#include "delete_request_handler.h"
#include "../ss_info.h"
#include "../../Common/responses.h"
#include "../../Common/loggers.h"
#include "../../Common/requests.h"

char delete_request_handler(char *path)
{
    int ss_id = get_ss_id_of_path(path);
    if (ss_id == -1)
    {
        return NOT_FOUND_RESPONSE;
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

    if (send_delete_request(connection_socket, path) == -1)
    {
        log_errno_error("Couldn't send delete request: %s\n");
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
        remove_path(path);
    }

    return response;
}