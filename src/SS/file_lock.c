#include <pthread.h>
#include <string.h>
#include "file_lock.h"
#include "../Common/requests.h"

pthread_mutex_t master_lock = PTHREAD_MUTEX_INITIALIZER;
char lock_file_list[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH] = {0};
pthread_rwlock_t locks[MAX_ACCESIBLE_PATHS] = {PTHREAD_RWLOCK_INITIALIZER};

int find_path_index(char *path)
{
    int i;
    for (i = 0; i < MAX_ACCESIBLE_PATHS; ++i)
    {
        if (strcmp(lock_file_list[i], path) == 0)
        {
            return i;
        }
    }
    return -1; // Path not found
}

int read_lock_file(char *path)
{
    int index = find_path_index(path);

    if (index == -1)
    {

        pthread_mutex_lock(&master_lock);
        // Path not found, create a new entry
        for (index = 0; index < MAX_ACCESIBLE_PATHS; ++index)
        {
            if (lock_file_list[index][0] == '\0')
            {
                strcpy(lock_file_list[index], path);
                break;
            }
        }

        pthread_mutex_unlock(&master_lock);
    }

    pthread_rwlock_rdlock(&locks[index]);

    return index;
}

int write_lock_file(char *path)
{

    pthread_mutex_lock(&master_lock);
    int index = find_path_index(path);

    if (index == -1)
    {
        // Path not found, create a new entry
        for (index = 0; index < MAX_ACCESIBLE_PATHS; ++index)
        {
            if (lock_file_list[index][0] == '\0')
            {
                strcpy(lock_file_list[index], path);
                break;
            }
        }
    }

    pthread_rwlock_wrlock(&locks[index]);

    pthread_mutex_unlock(&master_lock);
    return index;
}

void unlock_file(int lock_id)
{

    if (lock_id < 0 || lock_id >= MAX_ACCESIBLE_PATHS)
    {
        // Invalid lock_id
        return;
    }

    pthread_mutex_lock(&master_lock);
    // Try acquiring a write lock to check if no one else is holding
    if (pthread_rwlock_trywrlock(&locks[lock_id]) == 0)
    {
        // No one else is holding the lock, unlock and remove the entry
        pthread_rwlock_unlock(&locks[lock_id]);

        // No other locks found for this path, remove the entry
        lock_file_list[lock_id][0] = '\0';
    }
    else
    {
        // Someone else is holding the lock, just release it
        pthread_rwlock_unlock(&locks[lock_id]);
    }

    pthread_mutex_unlock(&master_lock);
}