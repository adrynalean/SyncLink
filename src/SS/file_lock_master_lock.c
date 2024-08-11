#include <pthread.h>

#include "file_lock_master_lock.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void acquire_file_master_lock()
{
    pthread_mutex_lock(&lock);
}

void release_file_master_lock()
{
    pthread_mutex_unlock(&lock);
}