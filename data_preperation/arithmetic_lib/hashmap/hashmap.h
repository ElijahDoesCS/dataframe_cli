#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct hashmap hashmap_t;
hashmap_t* hashmap_create();
void hashmap_put(hashmap_t* map, const char* key, int value);
int hashmap_get(hashmap_t* map, const char* key);
unsigned long hash_function(const char *str);
void hashmap_destroy(hashmap_t* map);
void hashmap_print(hashmap_t* map);
void hashmap_merge(hashmap_t *dest, hashmap_t *src);
char *get_mode_key(hashmap_t *map);

#endif
