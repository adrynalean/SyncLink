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
#include <arpa/inet.h>

#include "client_handler.h"
#include "../Common/loggers.h"
#include "../Common/network_config.h"
#include "../Common/requests.h"
#include "../Common/responses.h"
#include "../Common/network_utils.h"
#include "file_lock_master_lock.h"
#include "create.h"
#include "delete.h"

typedef struct ClientHandlerArguments
{
    int ssid;
    int socket;
    struct sockaddr_in client_address;
    socklen_t client_address_size;
} ClientHandlerArguments;
int try_lock_file(FILE *file, short type)
{
    int fd = fileno(file); // Get the file descriptor associated with the FILE pointer

    struct flock lock;
    lock.l_type = type; // lock for both read and write depending on the type
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;

    // Attempt to obtain the write lock without blocking
    int res = fcntl(fd, F_SETLKW, &lock);
    if (res == -1)
    {
        // Handle the case when the lock cannot be acquired immediately
        log_errno_error("Error obtaining write lock: %s\n");
        // perror("Error obtaining write lock");
        return -1;
    }

    // Lock obtained successfully
    return 0;
}

void unlock_file(FILE *file)
{
    int fd = fileno(file); // Get the file descriptor associated with the FILE pointer

    struct flock unlock;
    unlock.l_type = F_UNLCK; // Unlock
    unlock.l_start = 0;
    unlock.l_whence = SEEK_SET;
    unlock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &unlock) == -1)
    {
        log_errno_error("Error unlocking file: %s\n");
        // perror("Error unlocking file");
        exit(EXIT_FAILURE);
    }
}
int read_file_and_send_data(const char *path, int client_socket)
{
    // char path[MAX_PATH_LENGTH + sizeof(int) + 1];
    // snprintf(path, MAX_PATH_LENGTH + sizeof(int), "%d/%s", ssid, filepath);

    acquire_file_master_lock();
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        log_errno_error("Error while opening file: %s\n");
        return -1;
    }
    try_lock_file(file, F_RDLCK);
    release_file_master_lock();
    fseek(file, 0, SEEK_SET);

    char buffer[MAX_STREAMING_RESPONSE_PAYLOAD_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, MAX_STREAMING_RESPONSE_PAYLOAD_SIZE, file)) > 0)
    {
        if (send_streaming_response_payload(client_socket, buffer, bytes_read) == -1)
        {
            log_errno_error("Error while sending data to client: %s\n");
            return -1;
        }
    }
    end_streaming_response_payload(client_socket);
    unlock_file(file);
    fclose(file);
    return 0;
}
int recursive_path_finderr(char *path, char *prefix, char list_of_paths[][MAX_PATH_LENGTH], int *paths_count)
{
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(path);
    if (dir == NULL)
    {
        log_errno_error("Unable to open directory: %s\n");
        // perror("Unable to open directory");
        return -1;
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
        snprintf(follow_path, MAX_PATH_LENGTH, "%s/%s", path, entry->d_name);
        if (*prefix != '\0')
            snprintf(path, MAX_PATH_LENGTH, "%s/%s", prefix, entry->d_name);
        else
            snprintf(path, MAX_PATH_LENGTH, "%s", entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            // Recursively call the function for subdirectories
            recursive_path_finderr(follow_path, path, list_of_paths, paths_count);
        }
    }
    return 0;
    // Close the directory
    closedir(dir);
}
int is_directory(const char *path)
{
    struct stat file_stat;

    if (stat(path, &file_stat) == 0)
    {
        return S_ISDIR(file_stat.st_mode) ? 1 : 0;
    }
    else
    {
        log_errno_error("Couldn't get dir info: %s\n");
        return -1; // Return -1 on error
    }
}
int copy_(const char *path, const char *destination, struct sockaddr_in *destination_address, uint64_t *copied_paths_count, char copied_paths[][MAX_PATH_LENGTH], char *exclude_copy)
{
    // TODO remove copy inside self bugs
    if (strcmp(path, exclude_copy) == 0)
        return 0;
    int is_dir = is_directory(path);
    if (is_dir == -1)
        return -1;
    int destination_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (destination_socket == -1)
    {
        log_errno_error("Couldn't create socket: %s\n");
        return -1;
    }

    if (connect(destination_socket, (struct sockaddr *)destination_address, sizeof(*destination_address)) == -1)
    {
        log_errno_error("Couldn't connect to destination ss: %s\n");
        return -1;
    }

    if (send_create_request(destination_socket, destination, is_dir == 1) == -1)
    {
        log_errno_error("Couldn't send create request: %s\n");
        return -1;
    }

    char response;
    if (receive_response(destination_socket, &response) == -1)
    {
        log_errno_error("Couldn't receive response: %s\n");
        return -1;
    }
    log_response(response, destination_address);
    if (response != OK_RESPONSE)
    {
        return -1;
    }

    close(destination_socket);

    if (is_dir == 1)
    {
        DIR *dir;
        struct dirent *entry;
        // Open the directory
        dir = opendir(path);
        if (dir == NULL)
        {
            log_errno_error("Unable to open directory: %s\n");
            // perror("Unable to open directory");
            return -1;
        }

        // Read directory entries
        while ((entry = readdir(dir)) != NULL)
        {
            // Ignore "." and ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
            char follow_source_path[MAX_PATH_LENGTH];
            snprintf(follow_source_path, MAX_PATH_LENGTH, "%s/%s", path, entry->d_name);
            char follow_destination_path[MAX_PATH_LENGTH];
            snprintf(follow_destination_path, MAX_PATH_LENGTH, "%s/%s", destination, entry->d_name);

            if (copy_(follow_source_path, follow_destination_path, destination_address, copied_paths_count, copied_paths, exclude_copy) == -1)
                return -1;
        }
    }
    else
    {
        int destination_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (destination_socket == -1)
        {
            log_errno_error("Couldn't create socket: %s\n");
            return -1;
        }

        if (connect(destination_socket, (struct sockaddr *)destination_address, sizeof(*destination_address)) == -1)
        {
            log_errno_error("Couldn't connect to destination ss: %s\n");
            return -1;
        }

        if (send_write_request(destination_socket, destination) == -1)
        {
            log_errno_error("Couldn't send write request: %s\n");
            return -1;
        }

        char response;
        if (receive_response(destination_socket, &response) == -1)
        {
            log_errno_error("Couldn't receive response: %s\n");
            return -1;
        }
        log_response(response, destination_address);
        if (response != OK_START_STREAM_RESPONSE)
        {
            return -1;
        }

        if (read_file_and_send_data(path, destination_socket) == -1)
        {
            return -1;
        }

        close(destination_socket);
    }
    char *new_path = copied_paths[(*copied_paths_count)++];
    strncpy(new_path, destination, MAX_PATH_LENGTH);
    return 0;
}

int write_file(FILE *file, char *data_buffer)
{
    // char path[MAX_PATH_LENGTH + 1];
    // snprintf(path, MAX_PATH_LENGTH, "%d/%s", ssid, filepath);

    // write the data buffer to the file by concatinating it on the end of the file
    // Append the data buffer to the end of the file
    if (fprintf(file, "%s", data_buffer) < 0)
    {
        log_errno_error("Error while writing to file: %s\n");
        fclose(file); // Close the file before returning in case of an error
        return -1;
    }
    return 0;
}
int check_file_exists(char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return 0;
    }
    fclose(file);
    return 1;
}
int get_info_send_info(const char *path, int client_socket)
{
    struct stat file_stat;
    if (stat(path, &file_stat) == -1)
    {
        log_errno_error("Error while getting file info: %s\n");
        return -1;
    }
    if (send(client_socket, &file_stat, sizeof(file_stat), 0) == -1)
    {
        log_errno_error("Error while sending file info: %s\n");
        return -1;
    }
    // printf
    // printf("File Size: \t\t%ld bytes\n", file_stat.st_size);

    return 0;
}

void *client_handler(void *arguments)
{
    ClientHandlerArguments *client_ss_handler_arguments = (ClientHandlerArguments *)arguments;
    Request request_buffer;
    if (receive_request(client_ss_handler_arguments->socket, &request_buffer) == -1)
    {
        log_errno_error("Error while receiving request from client: %s\n");
        return NULL;
    }
    char response;
    switch (request_buffer.request_type)
    {
    case CREATE_REQUEST:
        log_info("CREATE_REQUEST", &client_ss_handler_arguments->client_address);
        if (request_buffer.request_content.create_request_data.is_folder)
        {
            if (create_folder(request_buffer.request_content.create_request_data.path) == -1)
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
            if (create_file(request_buffer.request_content.create_request_data.path) == -1)
            {
                response = INTERNAL_ERROR_RESPONSE;
            }
            else
            {
                response = OK_RESPONSE;
            }
        }
        log_response(response, &client_ss_handler_arguments->client_address);
        if (send_response(client_ss_handler_arguments->socket, response) == -1)
        {
            log_errno_error("Couldn't send response: %s\n");
        }
        break;
    case DELETE_REQUEST:
        log_info("DELETE_REQUEST", &client_ss_handler_arguments->client_address);
        if (delete_file_or_folder(request_buffer.request_content.delete_request_data.path) == -1)
        {
            log_errno_error("Couldn't delete: %s\n");
            response = INTERNAL_ERROR_RESPONSE;
        }
        else
        {
            response = OK_RESPONSE;
        }
        log_response(response, &client_ss_handler_arguments->client_address);
        if (send_response(client_ss_handler_arguments->socket, response) == -1)
        {
            log_errno_error("Couldn't send response: %s\n");
        }
        break;
    case READ_REQUEST:
        log_info("READ_REQUEST", &client_ss_handler_arguments->client_address);
        if (is_directory(request_buffer.request_content.read_request_data.path) == -1)
        {
            response = NOT_FOUND_RESPONSE;
            send_response(client_ss_handler_arguments->socket, response);
            log_response(response, &client_ss_handler_arguments->client_address);

            break;
        }

        send_response(client_ss_handler_arguments->socket, OK_START_STREAM_RESPONSE);
        log_response(OK_START_STREAM_RESPONSE, &client_ss_handler_arguments->client_address);
        if (read_file_and_send_data(request_buffer.request_content.read_request_data.path, client_ss_handler_arguments->socket) == -1)
        {
            response = INTERNAL_ERROR_RESPONSE;
            send_response(client_ss_handler_arguments->socket, response);
            log_response(response, &client_ss_handler_arguments->client_address);
        }

        break;

    case WRITE_REQUEST:

        // TODO handle file locking
        // TODO handle folder path given for read and write
        log_info("WRITE_REQUEST", &client_ss_handler_arguments->client_address);
        if (check_file_exists(request_buffer.request_content.write_request_data.path) == -1)
        {
            response = NOT_FOUND_RESPONSE;
        }
        else
        {
            response = OK_START_STREAM_RESPONSE;
        }

        acquire_file_master_lock();
        FILE *file = fopen(request_buffer.request_content.write_request_data.path, "w");
        if (file == NULL)
        {
            log_errno_error("Error while opening file: %s\n");
            return NULL;
        }
        try_lock_file(file, F_WRLCK);
        release_file_master_lock();
        send_response(client_ss_handler_arguments->socket, response);
        log_response(response, &client_ss_handler_arguments->client_address);
        while (1)

        {
            char buffer[MAX_STREAMING_RESPONSE_PAYLOAD_SIZE + 1] = {0};
            int k = receive_streaming_response_payload(client_ss_handler_arguments->socket, buffer);
            if (k == -1)
            {
                log_errno_error("Error while receiving data from client: %s\n");
                response = INTERNAL_ERROR_RESPONSE;
                break;
            }
            else if (k == 0)
            {
                break;
            }
            else
            {
                if (write_file(file, buffer) == -1)
                {
                    response = INTERNAL_ERROR_RESPONSE;
                    break;
                }
                else
                {
                    response = OK_RESPONSE;
                }
            }
        }
        unlock_file(file);
        fclose(file);
        send_response(client_ss_handler_arguments->socket, response);
        break;

    case COPY_REQUEST:
        log_info("COPY_REQUEST", &client_ss_handler_arguments->client_address);
        struct sockaddr_in destination_address;
        if (receive_redirect_response_payload(client_ss_handler_arguments->socket, &destination_address) == -1)
        {
            log_errno_error("Couldn't receive destination address: %s\n");
            response = INTERNAL_ERROR_RESPONSE;
        }
        else if (check_file_exists(request_buffer.request_content.copy_request_data.source_path) == 0 && is_directory(request_buffer.request_content.copy_request_data.source_path) != 1)
        {
            response = NOT_FOUND_RESPONSE;
        }
        else
        {

            char copied_paths[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH];
            uint64_t copied_paths_count = 0;
            if (copy_(request_buffer.request_content.copy_request_data.source_path, request_buffer.request_content.copy_request_data.destination_path, &destination_address, &copied_paths_count, copied_paths, request_buffer.request_content.copy_request_data.destination_path) == -1)
            {
                response = INTERNAL_ERROR_RESPONSE;
            }
            else
            {
                response = OK_RESPONSE;
                if (send_response(client_ss_handler_arguments->socket, response) == -1)
                {
                    log_errno_error("Couldn't send response: %s\n");
                    break;
                }

                if (send_copied_paths(client_ss_handler_arguments->socket, copied_paths_count, copied_paths) == -1)
                {
                    log_errno_error("Couldn't send copied paths: %s\n");
                }
                break;
            }
        }

        log_response(response, &client_ss_handler_arguments->client_address);
        if (send_response(client_ss_handler_arguments->socket, response) == -1)
        {
            log_errno_error("Couldn't send response: %s\n");
        }
        break;
    case FILE_INFO:
        log_info("FILE_INFO", &client_ss_handler_arguments->client_address);
        if (get_info_send_info(request_buffer.request_content.file_info_request_data.path, client_ss_handler_arguments->socket) == -1)
        {
            response = INTERNAL_ERROR_RESPONSE;
        }
        else
        {
            response = OK_RESPONSE;
        }
        send_response(client_ss_handler_arguments->socket, response);

        break;
    case GET_LIST:
        log_info("GET_LIST", &client_ss_handler_arguments->client_address);
        char list_of_paths[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH];
        int paths_count = 0;
        if (recursive_path_finderr(request_buffer.request_content.get_list_request_data.path, "", list_of_paths, &paths_count) == -1)
        {
            response = INTERNAL_ERROR_RESPONSE;
        }
        else
        {
            response = OK_RESPONSE;
        }
        // send the data to the client list of paths , paths_count
        send_response(client_ss_handler_arguments->socket, response);
        if (send(client_ss_handler_arguments->socket, list_of_paths, sizeof(list_of_paths), 0) == -1)
        {
            log_errno_error("Error while sending paths: %s\n");
            return NULL;
        }
        break;

    default:
        log_info("Invalid Request Type", &client_ss_handler_arguments->client_address);
        response = INVALID_REQUEST_RESPONSE;
        send_response(client_ss_handler_arguments->socket, response);
        break;
    }
    free(client_ss_handler_arguments);
    return NULL;
}

void *client_connection_acceptor(void *arguments)
{
    int ssid = ((ClientConnectionAcceptorArguments *)arguments)->ssid;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        log_errno_error("Error while creating client handler socket: %s\n");
        return NULL;
    }

    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(SS_CLIENT_HANDLER_BASE_PORT + ssid),
        .sin_addr = {
            .s_addr = inet_addr(SS_CLIENT_HANDLER_IP)}};

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        log_errno_error("Error while binding Client Handler Socket: %s\n");
        return NULL;
    }

    if (listen(server_socket, SS_CLIENT_HANDLER_TCP_WAIT_QUEUE) == -1)
    {
        log_errno_error("Error while listening using Client Handler Socket: %s\n");
        return NULL;
    }

    log_info("Listening for Client Connections", &server_address);
    while (1)
    {
        ClientHandlerArguments *client_handler_arguments = malloc(sizeof(ClientHandlerArguments));
        client_handler_arguments->ssid = ssid;
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