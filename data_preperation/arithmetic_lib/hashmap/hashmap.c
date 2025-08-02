#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "./hashmap.h"
#include <stdlib.h>

// Implement hash function with static bucket size but seperate chaining

#define NUM_BUCKETS 2048 

typedef struct entry {
    char *key;
    int value; // Count of key occurrences in this bucket
    struct entry *next; // Pointer to next bucket in case of collision
} entry_t;

typedef struct hashmap {
    int num_buckets; // Number of buckets already stored 
    int mode; // Mode of the hashmap
    char *mode_key;
    entry_t *buckets[NUM_BUCKETS]; // Array of bucket linked lists
} hashmap_t;

// djb2 hash function
unsigned long hash_function(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    return hash;
}

hashmap_t* hashmap_create() {
    hashmap_t *map = calloc(1, sizeof(hashmap_t));
    if (!map) {
        fprintf(stderr, "Failed to allocate memory for hashmap\n");
        return NULL;
    }
    for (int i = 0; i < NUM_BUCKETS; i++) {
        map->buckets[i] = NULL; // Initialize all buckets to NULL
    }

    map->mode = 0; // Default mode
    map->num_buckets = 0;

    return map;
}

void hashmap_put(hashmap_t* map, const char* key, int value) {
    // Check for valid map and key
    if (!map || !key) {
        fprintf(stderr, "Invalid hashmap or key\n");
        return;
    }

    if (value > INT_MAX || value < 0) {
        fprintf(stderr, "Invalid value for hashmap: %d\n", value);
        return;
    }

    unsigned long hash = hash_function(key);
    int index = hash % NUM_BUCKETS;

    entry_t *current = map->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value; // Update count of occurrences if key already exists
            if (value > map->mode) {
                map->mode = value; // Update mode if new value is greater
                map->mode_key = strdup(key); // Update mode key
            }

            return;
        }
        current = current->next;
    }

    // Create a new bucket for the new key
    entry_t *new_bucket = malloc(sizeof(entry_t));
    if (!new_bucket) {
        fprintf(stderr, "Failed to allocate memory for new bucket\n");
        return;
    }
    new_bucket->key = strdup(key);
    new_bucket->value = value;
    new_bucket->next = map->buckets[index];
    
    map->buckets[index] = new_bucket;
    map->num_buckets++;
    if (value > map->mode) { 
        map->mode = value;
        map->mode_key = strdup(key); // Allocate memory for mode key
    }
}

int hashmap_get(hashmap_t* map, const char* key) {
    // Check for valid map and key
    if (!map || !key) {
        fprintf(stderr, "Invalid hashmap or key\n");
        return -1; // Indicate error
    }

    unsigned long hash = hash_function(key);
    int index = hash % NUM_BUCKETS;

    entry_t *current = map->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current->value; // Return the count of occurrences
        }
        current = current->next;
    }

    return 0; // Key not found, return 0
}

void hashmap_print(hashmap_t* map) {
    if (!map) {
        fprintf(stderr, "Invalid hashmap\n");
        return;
    }

    for (int i = 0; i < NUM_BUCKETS; i++) {
        entry_t *current = map->buckets[i];
        if (current) {
            printf("Bucket %d:\n", i);
            while (current) {
                printf("  Key: %s, Value: %d\n", current->key, current->value);
                current = current->next;
            }
        }
    }

    printf("Hashmap mode: %d\n", map->mode);
    printf("Hashmap mode key: %s\n", map->mode_key);
}

void hashmap_destroy(hashmap_t* map) {
    if (!map) return;

    for (int i = 0; i < NUM_BUCKETS; i++) {
        entry_t *current = map->buckets[i];
        while (current) {
            entry_t *temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
    }

    free(map->mode_key); // Free mode key if allocated
    free(map);
}

void hashmap_merge(hashmap_t *dest, hashmap_t *src) {
    if (!dest || !src) {
        fprintf(stderr, "Invalid hashmap for merge\n");
        return;
    }

    for (int i = 0; i < NUM_BUCKETS; i++) {
        entry_t *current = src->buckets[i];
        while (current) {
            int count = hashmap_get(dest, current->key);
            hashmap_put(dest, current->key, count + current->value);
            current = current->next;
        }
    }
}

char *get_mode_key(hashmap_t *map) {
    if (!map) {
        fprintf(stderr, "Invalid hashmap\n");
        return NULL;
    }
    if (map->mode == 1 || map->mode == 0) {
        return "N/A"; // No mode or only one occurrence
    }

    return map->mode_key; // Return the mode key
}