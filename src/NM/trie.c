#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "trie.h"

TrieNode *init_trie_node(char character)
{
    TrieNode *node = malloc(sizeof(TrieNode));
    for (int i = 0; i < UCHAR_MAX; i++)
        node->children[i] = NULL;

    node->data = TRIE_SENTINEL_DATA;
    node->character = character;
    return node;
}

void free_trienode(TrieNode *node)
{
    for (int i = 0; i < UCHAR_MAX; i++)
        if (node->children[i] != NULL)
            free_trienode(node->children[i]);

    free(node);
}

void insert_trie(TrieNode *root, char *word, int data)
{
    for (int i = 0; word[i] != '\0'; i++)
    {
        int index = (unsigned char)word[i];
        if (root->children[index] == NULL)
            root->children[index] = init_trie_node(word[i]);

        root = root->children[index];
    }
    root->data = data;
}

int search_trie(TrieNode *root, const char *word)
{
    for (int i = 0; word[i] != '\0'; i++)
    {
        int index = (unsigned char)word[i];
        if (root->children[index] == NULL)
            return TRIE_SENTINEL_DATA;
        root = root->children[index];
    }
    return root->data;
}

bool is_leaf_node(TrieNode *root, const char *word)
{
    for (int i = 0; word[i] != '\0'; i++)
    {
        int index = (unsigned char)word[i];
        if (root->children[index] != NULL)
            root = root->children[index];
    }
    return root->data != TRIE_SENTINEL_DATA;
}

int check_divergence(TrieNode *root, char *word)
{
    size_t length = strlen(word);
    if (length == 0)
        return 0;

    int last_index = 0;
    for (int i = 0; i < length; i++)
    {
        int index = (unsigned char)word[i];
        if (root->children[index])
        {
            for (int j = 0; j < UCHAR_MAX; j++)
                if (j != index && root->children[j])
                {
                    last_index = i + 1;
                    break;
                }

            root = root->children[index];
        }
    }
    return last_index;
}

char *find_longest_prefix(TrieNode *root, char *word)
{
    if (word == NULL || *word == '\0')
        return NULL;

    size_t length = strlen(word);

    char *longest_prefix = malloc(length + 1);
    strcpy(longest_prefix, word);

    int branch_index = check_divergence(root, longest_prefix) - 1;
    if (branch_index >= 0)
    {
        longest_prefix[branch_index] = '\0';
        longest_prefix = realloc(longest_prefix, (branch_index + 1));
    }

    return longest_prefix;
}

void delete_trie(TrieNode *root, char *word)
{
    if (root == NULL)
        return;

    if (word == NULL || *word == '\0')
        return;

    if (!is_leaf_node(root, word))
        return;

    char *longest_prefix = find_longest_prefix(root, word);
    //    if (longest_prefix[0] == '\0') {
    //        free(longest_prefix);
    //        return;
    //    }
    int i = 0;
    for (; longest_prefix[i] != '\0'; i++)
    {
        int index = (unsigned char)longest_prefix[i];
        if (root->children[index] != NULL)
            root = root->children[index];
        else
        {
            free(longest_prefix);
            return;
        }
    }

    // Do we need the for loop? just call free trie node on the next char
    int index = (unsigned char)word[i];
    if (root->children[index] != NULL)
    {
        TrieNode *remove_node = root->children[index];
        root->children[index] = NULL;
        free_trienode(remove_node);
    }

    free(longest_prefix);
}
