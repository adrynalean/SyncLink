#include "create.h"
#include "../Common/requests.h"
#include <sys/stat.h>

int create_folder(const char *path)
{
    return mkdir(path, 0777);
}
int create_file(const char *path)
{
    // printf("Creating file %s\n", path);
    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        return -1;
    }
    fclose(file);
    return 0;
}

int create_backup_file(const char *path)
{

    char k[MAX_PATH_LENGTH];
    strcpy(k, "../backup/");
    strcat(k, path);
    // printf("Creating backup file %s\n", k);
    FILE *file = fopen(k, "w");
    if (file == NULL)
    {
        return -1;
    }
    fclose(file);
    return 0;
}
int create_backup_folder(const char *path)
{

    char k[MAX_PATH_LENGTH];
    strcpy(k, "../backup/");
    strcat(k, path);
    // printf("Creating backup folder %s\n", k);
    return mkdir(k, 0777);
}