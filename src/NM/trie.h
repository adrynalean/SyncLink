#ifndef TRIE_H
#define TRIE_H

#include <limits.h>

#define TRIE_SENTINEL_DATA (-1)
typedef struct TrieNode
{
    char character;
    struct TrieNode *children[UCHAR_MAX + 1];
    int data;
} TrieNode;

TrieNode *init_trie_node(char character);

void insert_trie(TrieNode *root, char *word, int data);

int search_trie(TrieNode *root, const char *word);

void delete_trie(TrieNode *root, char *word);

#endif // TRIE_H
