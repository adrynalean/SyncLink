#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "../Common/network_config.h"
#include "ss_info.h"
#include "trie.h"
#include "hash.h"

pthread_rwlock_t is_registered_lock = PTHREAD_RWLOCK_INITIALIZER;
bool is_registered[MAX_SS_COUNT] = {false};

pthread_rwlock_t nm_connection_addresses_lock = PTHREAD_RWLOCK_INITIALIZER;
struct sockaddr_in nm_connection_addresses[MAX_SS_COUNT];

pthread_rwlock_t client_connection_addresses_lock = PTHREAD_RWLOCK_INITIALIZER;
struct sockaddr_in client_connection_addresses[MAX_SS_COUNT];

pthread_rwlock_t path_ss_info_lock = PTHREAD_RWLOCK_INITIALIZER;
TrieNode path_ss_info = {.data = TRIE_SENTINEL_DATA, .character = '\0', .children = {NULL}};

pthread_rwlock_t ht_lock = PTHREAD_RWLOCK_INITIALIZER;
Hashtable *ht = NULL;

void register_ss(int ss_id, struct sockaddr_in nm_connection_address, struct sockaddr_in client_connection_address,
                 uint64_t accessible_paths_count, char **accessible_paths)
{
    pthread_rwlock_wrlock(&nm_connection_addresses_lock);
    nm_connection_addresses[ss_id] = nm_connection_address;
    pthread_rwlock_unlock(&nm_connection_addresses_lock);

    pthread_rwlock_wrlock(&client_connection_addresses_lock);
    client_connection_addresses[ss_id] = client_connection_address;
    pthread_rwlock_unlock(&client_connection_addresses_lock);

    pthread_rwlock_wrlock(&path_ss_info_lock);
    for (uint64_t i = 0; i < accessible_paths_count; i++)
        insert_trie(&path_ss_info, accessible_paths[i], ss_id);
    pthread_rwlock_unlock(&path_ss_info_lock);

    pthread_rwlock_wrlock(&ht_lock);
    ht = initializeHashtable(ht);
    pthread_rwlock_unlock(&ht_lock);

    pthread_rwlock_wrlock(&is_registered_lock);
    is_registered[ss_id] = true;
    pthread_rwlock_unlock(&is_registered_lock);
}

void unregister_ss(int ss_id)
{
    pthread_rwlock_wrlock(&is_registered_lock);
    is_registered[ss_id] = false;
    pthread_rwlock_unlock(&is_registered_lock);
}

void add_path(int ss_id, char *path)
{
    pthread_rwlock_wrlock(&path_ss_info_lock);
    insert_trie(&path_ss_info, path, ss_id);
    pthread_rwlock_unlock(&path_ss_info_lock);

    pthread_rwlock_wrlock(&ht_lock);
    addPath(ht, path, ss_id);
    pthread_rwlock_unlock(&ht_lock);
}

void remove_path(char *path)
{
    pthread_rwlock_wrlock(&ht_lock);
    deletePath(ht, path);
    pthread_rwlock_unlock(&ht_lock);

    pthread_rwlock_wrlock(&path_ss_info_lock);
    delete_trie(&path_ss_info, path);
    pthread_rwlock_unlock(&path_ss_info_lock);
}

int get_ss_id_of_path(char *path)
{
    int ss_id = -1;
    pthread_rwlock_rdlock(&ht_lock);
    ss_id = getSSID(ht, path);
    pthread_rwlock_unlock(&ht_lock);

    if (ss_id == -1)
    {
        pthread_rwlock_rdlock(&path_ss_info_lock);
        ss_id = search_trie(&path_ss_info, path);
        pthread_rwlock_unlock(&path_ss_info_lock);
    }

    if (ss_id != -1)
    {

        pthread_rwlock_wrlock(&ht_lock);
        addPath(ht, path, ss_id);
        pthread_rwlock_unlock(&ht_lock);
    }

    return ss_id;
}

int get_random_registered_ss_id()
{

    int registered_ssids[MAX_SS_COUNT];
    int registered_count = 0;
    pthread_rwlock_rdlock(&is_registered_lock);
    for (int i = 0; i < MAX_SS_COUNT; i++)
        if (is_registered[i])
            registered_ssids[registered_count++] = i;
    pthread_rwlock_unlock(&is_registered_lock);

    if (registered_count == 0)
        return -1;

    srand(time(NULL));
    int random_index = rand() % registered_count;
    return registered_ssids[random_index];
}

struct sockaddr_in get_nm_connection_address(int ss_id)
{
    pthread_rwlock_rdlock(&nm_connection_addresses_lock);
    struct sockaddr_in address = nm_connection_addresses[ss_id];
    pthread_rwlock_unlock(&nm_connection_addresses_lock);
    return address;
}

struct sockaddr_in get_client_connection_address(int ss_id)
{
    pthread_rwlock_rdlock(&client_connection_addresses_lock);
    struct sockaddr_in address = client_connection_addresses[ss_id];
    pthread_rwlock_unlock(&client_connection_addresses_lock);
    return address;
}
