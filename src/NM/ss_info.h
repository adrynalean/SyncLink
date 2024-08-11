#ifndef SS_INFO_H
#define SS_INFO_H

#include <arpa/inet.h>
#include "../Common/requests.h"

void register_ss(int ss_id, struct sockaddr_in nm_connection_address, struct sockaddr_in client_connection_address,
                 uint64_t accessible_paths_count, char **accessible_paths);

void unregister_ss(int ss_id);

void add_path(int ss_id, char *path);

void remove_path(char *path);

int get_ss_id_of_path(char *path);

int get_random_registered_ss_id();

struct sockaddr_in get_nm_connection_address(int ss_id);

struct sockaddr_in get_client_connection_address(int ss_id);

#endif // SS_INFO_H
