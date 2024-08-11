#ifndef HASH_H
#define HASH_H

#include <time.h>
#include "../Common/requests.h"

#define HASHTABLE_SIZE 2
#define HASH_SIZE 1000

typedef struct
{
    int hash;
    char path[MAX_PATH_LENGTH];
    time_t entry_time;
    int ssid;
} CacheEntry;

typedef struct
{
    CacheEntry entries[HASHTABLE_SIZE];
} Hashtable;

unsigned int hash(char *path);
Hashtable *initializeHashtable(Hashtable *ht);
void addPath(Hashtable *ht, char *path, int ssid);
void deletePath(Hashtable *ht, char *path);
int getSSID(Hashtable *ht, char *path);
void printHashtable(Hashtable *ht);

#endif // HASH_H
