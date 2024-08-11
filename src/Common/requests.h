#ifndef REQUESTS_H
#define REQUESTS_H

#define MAX_PATH_LENGTH 4096UL
#define MAX_ACCESIBLE_PATHS 400

#define CREATE_REQUEST '0'
#define CREATE_BACKUP_REQUEST '1'
#define SS_REGISTER_REQUEST '2'
#define DELETE_REQUEST '3'
#define READ_REQUEST '4'
#define WRITE_REQUEST '5'
#define COPY_REQUEST '6'
#define FILE_INFO '7'
#define GET_LIST '8'

#include <stdbool.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "../Common/responses.h"

extern char writing_file[MAX_PATH_LENGTH][MAX_ACCESIBLE_PATHS];
extern int writing_file_count;

typedef struct CreateRequestData
{
    bool is_folder;
    char path[MAX_PATH_LENGTH];
} CreateRequestData;

typedef struct CreateBackupRequestData
{
    bool is_folder;
    char path[MAX_PATH_LENGTH];
} CreateBackupRequestData;

typedef struct DeleteRequestData
{
    char path[MAX_PATH_LENGTH];
} DeleteRequestData;

typedef struct ReadRequestData
{
    char path[MAX_PATH_LENGTH];
} ReadRequestData;

typedef struct SSRegisterData
{
    int ss_id;
    struct sockaddr_in nm_connection_address;
    struct sockaddr_in client_connection_address;
    uint64_t accessible_paths_count;
    char **accessible_paths;
} SSRegisterData;

typedef struct CopyRequestData
{
    char source_path[MAX_PATH_LENGTH];
    char destination_path[MAX_PATH_LENGTH];
} CopyRequestData;
typedef struct FileInfoRequestData
{
    char path[MAX_PATH_LENGTH];
} FileInfoRequestData;
typedef struct WriteRequestData
{
    char path[MAX_PATH_LENGTH];
} WriteRequestData;

typedef struct GetListRequestData
{
    char path[MAX_PATH_LENGTH];
} GetListRequestData;
union RequestContent
{
    CreateRequestData create_request_data;
    CreateBackupRequestData create_backup_request_data;
    SSRegisterData ss_register_data;
    DeleteRequestData delete_request_data;
    ReadRequestData read_request_data;
    WriteRequestData write_request_data;
    CopyRequestData copy_request_data;
    FileInfoRequestData file_info_request_data;
    GetListRequestData get_list_request_data;
};

typedef struct Request
{
    char request_type;
    union RequestContent request_content;
} Request;

int send_create_request(int socket, const char *file_path, bool is_folder);

int send_create_backup_request(int socket, const char *file_path, bool is_folder);

int send_register_ss_request(int socket, int ss_id, struct sockaddr_in *nm_connection_address,
                             struct sockaddr_in *client_connection_address,
                             uint64_t accessible_paths_count,
                             char accessible_paths[accessible_paths_count][MAX_PATH_LENGTH]);

int send_delete_request(int socket, const char *path);

int send_read_request(int socket, const char *path);

int send_write_request(int socket, const char *path);

int send_copy_request(int socket, const char *source_path, const char *destination_path);

int receive_request(int socket, Request *request_buffer);

int send_get_request(int socket, const char *path);

int send_list_request(int socket, const char *path);

#endif // REQUESTS_H