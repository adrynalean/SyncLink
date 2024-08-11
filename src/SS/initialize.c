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

#include "initialize.h"
#include "../Common/network_config.h"
#include "../Common/requests.h"
#include "../Common/responses.h"
#include "../Common/loggers.h"

int ss_folder_create(const char *path)
{
    return mkdir(path, 0777);
}

int if_folder_exists(char *ss_id)
{
    // check if folder named ss_id exists in the current directory
    int x = opendir(ss_id) == NULL ? 0 : 1;
    // printf("x: %d\n", x);
    return x;
}
void recursive_path_finder(char *ss_id, char *prefix, char list_of_paths[][MAX_PATH_LENGTH], int *paths_count)
{
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(ss_id);
    if (dir == NULL)
    {
        log_errno_error("Unable to open directory: %s\n");
        // perror("Unable to open directory");
        return;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL)
    {
        // Ignore "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Construct the full path
        char *path = list_of_paths[(*paths_count)++];
        char follow_path[MAX_PATH_LENGTH];
        snprintf(follow_path, MAX_PATH_LENGTH, "%s/%s", ss_id, entry->d_name);
        if (*prefix != '\0')
            snprintf(path, MAX_PATH_LENGTH, "%s/%s", prefix, entry->d_name);
        else
            snprintf(path, MAX_PATH_LENGTH, "%s", entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            // Recursively call the function for subdirectories
            recursive_path_finder(follow_path, path, list_of_paths, paths_count);
        }
    }

    // Close the directory
    closedir(dir);
}

int initialize(int argc, char *argv[])
{
    char list_of_paths[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH];
    char *ss_id = argv[1];

    int flag = if_folder_exists(ss_id);
    int paths_count = 0;
    if (flag == 0)
    {
        ss_folder_create(ss_id);
    }
    else
    {
        recursive_path_finder(ss_id, "", list_of_paths, &paths_count);
    }
    if (chdir(ss_id) == -1)
    {
        log_errno_error("Couldn't change directory: %s\n");
        return -1;
    }

    int nm_init_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (nm_init_socket == -1)
    {
        log_errno_error("Error while creating NM Init Socket: %s\n");
        // printf("Error while creating NM Init Socket:\n");
        return 0;
    }

    struct sockaddr_in nm_init_server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(NM_SS_HANDLER_SERVER_PORT),
        .sin_addr = {
            .s_addr = inet_addr(NM_SS_HANDLER_SERVER_IP),
        }};

    int k = connect(nm_init_socket, (struct sockaddr *)&nm_init_server_address, sizeof(nm_init_server_address));
    if (k == -1)
    {
        log_errno_error("Error while connecting NM Init Socket: %s\n");
    }
    // connecting NM
    int server_port = SS_NM_HANDLER_BASE_PORT + atoi(ss_id);

    struct sockaddr_in ss_nm_server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(server_port),
        .sin_addr = {
            .s_addr = inet_addr(SS_NM_HANDLER_IP),
        }};

    // connecting SS to Client

    // int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    int client_port = SS_CLIENT_HANDLER_BASE_PORT + atoi(ss_id);
    struct sockaddr_in SS_client_address = {
        .sin_family = AF_INET,
        .sin_port = htons(client_port),
        .sin_addr = {
            .s_addr = inet_addr(SS_CLIENT_HANDLER_IP),
        }};

    if (send_register_ss_request(nm_init_socket, atoi(ss_id), &ss_nm_server_address, &SS_client_address, paths_count, list_of_paths) == 0)
    {
        log_info("Initialising SS connection with NM\n", &nm_init_server_address);
    }
    else
    {
        log_errno_error("Error while sending register request to NM %s\n");
    }

    char response;
    if (receive_response(nm_init_socket, &response) == -1)
    {
        log_errno_error("Couldn't receive response: %s\n");
        exit(-1);
    }
    log_response(response, &nm_init_server_address);
    if (response != OK_RESPONSE)
    {
        exit(-1);
    }
    return 0;
}