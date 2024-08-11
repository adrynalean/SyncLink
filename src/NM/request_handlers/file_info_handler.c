#include "read_request_handler.h"
#include "../../Common/responses.h"
#include "../../Common/loggers.h"
#include "../ss_info.h"


void file_info_request_handler(int socket, struct sockaddr_in *address, char *path)
{
    int ss_id = get_ss_id_of_path(path);
    if (ss_id == -1)
    {
        send_response(socket, NOT_FOUND_RESPONSE);
        log_response(NOT_FOUND_RESPONSE, address);
    }

    struct sockaddr_in redirect_address = get_client_connection_address(ss_id);
    if (send_response(socket, REDIRECT_RESPONSE) == -1)
    {
        log_errno_error("Couldn't send response: %s\n");
        return;
    }

    log_response(REDIRECT_RESPONSE, address);

    if (send_redirect_response_payload(socket, &redirect_address) == -1)
    {
        log_errno_error("Couldn't send redirect response: %s\n");
    }
}