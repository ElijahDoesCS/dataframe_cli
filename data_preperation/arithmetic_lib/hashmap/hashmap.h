#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct HashNode {
    char *key;
    int count;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode **buckets;
    int size;
} HashMap;

unsigned long hash_function(const char *str);
HashMap *create_hashmap(int size);
void hashmap_insert(HashMap *map, const char *key);
void print_hashmap(HashMap *map);
void free_hashmap(HashMap *map);

#endif
