#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <fcntl.h>
#include "get.h"
#include "../Common/network_config.h"
#include "../Common/requests.h"
#include "../Common/responses.h"
#include "../Common/loggers.h"
// #include "network_utils.h"
#include "../Common/network_utils.h"
#include "../Common/requests.h"

void list()
{
    char path[MAX_PATH_LENGTH + 1];
    printf("Enter Path (to get file info) :");
    if (fgets(path, sizeof(path), stdin) == NULL)
    {
        return;
    }
    size_t path_size = strlen(path);
    path[path_size - 1] = '\0'; // set newline to null

    struct sockaddr_in nm_address = {
        .sin_family = AF_INET,
        .sin_port = htons(NM_CLIENT_HANDLER_SERVER_PORT),
        .sin_addr = {
            .s_addr = inet_addr(NM_CLIENT_HANDLER_SERVER_IP),
        }};
    int connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connection_socket == -1)
    {
        log_errno_error("Couldn't create socket: %s\n");
        return;
    }

    if (connect(connection_socket, (struct sockaddr *)&nm_address, sizeof(nm_address)) == -1)
    {
        log_errno_error("Couldn't connect to ss: %s\n");
        return;
    }
    // SENDING GET REQUEST WITH THE PATH
    if (send_get_request(connection_socket, path) == -1)
    {
        log_errno_error("Couldn't send get request: %s\n");
        return;
    }
    // RECEIVING RESPONSE WITH HAS AN ADDRESS
    char response;
    // char address[100];
    if (receive_response(connection_socket, &response) == -1)
    {
        log_errno_error("Couldn't receive response: %s\n");
        return;
    }
    // log_response(response, &nm_address);
    // close(connection_socket);

    if (response == REDIRECT_RESPONSE)
    {

        struct sockaddr_in address_ss;
        if (receive_redirect_response_payload(connection_socket, &address_ss) == -1)
        {
            log_errno_error("Couldn't receive address: %s\n");
            return;
        }

        log_response(response, &nm_address);
        close(connection_socket);
        // MAKE A NEW CONNECTION TO SS
        int ss_connection = socket(AF_INET, SOCK_STREAM, 0);
        if (ss_connection == -1)
        {
            log_errno_error("Couldn't create socket: %s\n");
            return;
        }
        if (connect(ss_connection, (struct sockaddr *)&address_ss, sizeof(address_ss)) == -1)
        {
            log_errno_error("Couldn't connect to ss: %s\n");
            return;
        }
        // SENDING GET REQUEST WITH THE PATH
        if (send_list_request(connection_socket, path) == -1)
        {
            log_errno_error("Couldn't send get request to ss: %s\n");
            return;
        }

        // RECEIVING RESPONSE WITH HAS AN ADDRESS
        // Message fileinfo;
        char files[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH + 1];
        receive_response(connection_socket, &response);
        log_response(response, &nm_address);
        if (response == OK_RESPONSE)
        {
            if (recv(ss_connection, &files, sizeof(files), 0) == -1)
            {
                log_errno_error("Couldn't receive file info: %s\n");
                return;
            }
            printf("Files in the path are:\n");
            for (int i = 0; i < MAX_ACCESIBLE_PATHS; i++)
            {
                printf("%s\n", files[i]);
                if (files[i][0] == '\0')
                    break;
            }
        }
        close(ss_connection);
    }
}