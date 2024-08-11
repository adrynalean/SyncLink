#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

unsigned int hash(char *path)
{
    unsigned int hashval = 0;
    for (; *path != '\0'; path++)
    {
        hashval = *path + (hashval << 5) - hashval;
    }
    return hashval % HASH_SIZE;
}

Hashtable *initializeHashtable(Hashtable *ht)
{
    if (ht == NULL)
    {
        ht = malloc(sizeof(Hashtable));

        for (int i = 0; i < HASHTABLE_SIZE; i++)
        {
            ht->entries[i].entry_time = 0;
            ht->entries[i].hash = -1;
        }
    }
    return ht;
}

void addPath(Hashtable *ht, char *path, int ssid)
{
    if (ht == NULL)
        return;
    unsigned int path_hash = hash(path);

    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (ht->entries[i].hash == path_hash && strcmp(ht->entries[i].path, path) == 0)
        {
            ht->entries[i].entry_time = time(NULL);
            return;
        }
    }

    int insert_at;
    time_t oldest_time = time(NULL);
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (ht->entries[i].hash == -1)
        {
            insert_at = i;
            break;
        }

        if (ht->entries[i].entry_time < oldest_time)
            insert_at = i;
    }

    CacheEntry newEntry;
    strcpy(newEntry.path, path);
    newEntry.ssid = ssid;
    newEntry.entry_time = time(NULL);
    newEntry.hash = path_hash;

    ht->entries[insert_at] = newEntry;
}

void deletePath(Hashtable *ht, char *path)
{

    if (ht == NULL)
        return;

    unsigned int path_hash = hash(path);
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (ht->entries[i].hash == path_hash && strcmp(ht->entries[i].path, path) == 0)
        {
            ht->entries[i].entry_time = 0;
            ht->entries[i].hash = -1;
        }
    }

    size_t path_len = strlen(path);
    char compare_path[path_len + 1 + 1];
    strcpy(compare_path, path);
    compare_path[path_len] = '/';
    compare_path[path_len + 1] = '\0';

    // delete child paths
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (strncmp(ht->entries[i].path, compare_path, path_len + 1) == 0)
        {
            ht->entries[i].entry_time = 0;
            ht->entries[i].hash = -1;
        }
    }
}

int getSSID(Hashtable *ht, char *path)
{
    if (ht == NULL)
        return -1;
    unsigned int path_hash = hash(path);

    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (ht->entries[i].hash == path_hash && strcmp(ht->entries[i].path, path) == 0)
        {
            return ht->entries[i].ssid;
        }
    }

    return -1;
}

void printHashtable(Hashtable *ht)
{
    printf("Hashtable contents:\n");
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        if (ht->entries[i].entry_time != 0)
        {
            printf("Index %d: Path: %s, SSID: %d, Entry Time: %ld\n", i, ht->entries[i].path, ht->entries[i].ssid, ht->entries[i].entry_time);
        }
    }
    printf("\n");
}

// int main()
// {
//     // Create a hashtable
//     Hashtable ht;
//     // Initialize the hashtable
//     initializeHashtable(&ht);
//     // Example usage
//     addPath(&ht, "/example/path1", 123);

//     // sleep(2);
//     addPath(&ht, "/example/path2", 456);
//     // addPath(&ht, "/example/path1", 127);
//     // addPath(&ht, "/example/path2", 457);
//     // addPath(&ht, "/example/path1", 173);
//     // addPath(&ht, "/example/path2", 476);
//     // addPath(&ht, "/example/path1", 723);
//     // addPath(&ht, "/example/path2", 756);
//     // addPath(&ht, "/example/path1", 223);
//     // addPath(&ht, "/example/path2", 466);
//     // addPath(&ht, "/example/path1", 103);
//     // addPath(&ht, "/example/path2", 406);
//     printHashtable(&ht);
//     // printf("SSID for /example/path1: %d\n", getSSID(&ht, "/example/path1"));
//     // printf("SSID for /example/path2: %d\n", getSSID(&ht, "/example/path2"));

//     // Delete path "/example/path1"
//     // deletePath(&ht, "/example/path1");

//     // Try to get the SSID for the deleted path
//     // printf("SSID for /example/path1 after deletion: %d\n", getSSID(&ht, "/example/path1"));

//     return 0;
// }
