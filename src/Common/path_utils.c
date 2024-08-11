#include <string.h>

#include "path_utils.h"


const char *get_basename(const char *path)
{
    const char *lastItem = strrchr(path, '/');

    if (lastItem != NULL) {
        return lastItem + 1;
    } else {
        // No '/' found, so the whole path is the last item
        return path;
    }
}


void get_parent(const char *path, char *parent)
{
    // Initialize the parent path as an empty string
    parent[0] = '\0';

    // Find the last occurrence of '/' in the path
    const char *last_separator = strrchr(path, '/');

    if (last_separator != NULL) {
        // Calculate the length of the parent path
        size_t parent_length = last_separator - path;

        // Copy the parent path to the 'parent' variable
        strncpy(parent, path, parent_length);
        parent[parent_length] = '\0'; // Null-terminate the parent path
    }
}