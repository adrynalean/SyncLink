#ifndef FILE_LOCK_H
#define FILE_LOCK_H

int read_lock_file(char *path);
int write_lock_file(char *path);
void unlock_file(int lock_id);

#endif