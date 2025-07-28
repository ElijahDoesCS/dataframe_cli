#include "statistical_ops.h"
#include "../fat_data/fat_data.h"
#include "../hashmap/hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>  // For DBL_MIN
#include <errno.h>

void compute_local_max(char **chunk, int chunk_size, char *result) {
    if (!chunk || chunk_size <= 0 || !result) {
        if (result) result[0] = '\0';
        fprintf(stderr, "Invalid call to compute local max function\n");
        return;
    }

    char *current_result = chunk[0];

    for (int i = 1; i < chunk_size; i++) {
        if (compare_big_numbers(chunk[i], current_result) == 1) {
            current_result = chunk[i];
        }
    }

    strncpy(result, current_result, MAX_NUMBER_LENGTH - 1);
    result[MAX_NUMBER_LENGTH - 1] = '\0';

    return;
}

void compute_local_min(char **chunk, int chunk_size, char *result) {
    if (!chunk || chunk_size <= 0 || !result) {
        if (result) result[0] = '\0';
        fprintf(stderr, "Invalid call to compute local min function\n");
        return;
    }

    char *current_result = chunk[0];

    for (int i = 1; i < chunk_size; i++) {
        if (compare_big_numbers(chunk[i], current_result) == -1) {
            current_result = chunk[i];
        }
    }

    strncpy(result, current_result, MAX_NUMBER_LENGTH - 1);
    result[MAX_NUMBER_LENGTH - 1] = '\0'; // Pretty sure strncpy delimits by default

    return;
}

void compute_local_sum(char **chunk, int chunk_size, char *result) {
    char temp_result[MAX_NUMBER_LENGTH] = "0";
    char temp_sum[MAX_NUMBER_LENGTH];

    for (int i = 0; i < chunk_size; i++) {
        add_big_integers(temp_result, chunk[i], temp_sum); // temp_sum = temp_result + chunk[i]
        strncpy(temp_result, temp_sum, MAX_NUMBER_LENGTH); // update temp_result
    }

    strncpy(result, temp_result, MAX_NUMBER_LENGTH);

    return;
}


void compute_local_counts(char **chunk, int chunk_size, hashmap_t *freq_map) {
    if (!chunk || chunk_size <= 0 || !freq_map) {
        fprintf(stderr, "Invalid call to compute local counts function\n");
        return;
    }

    for (int i = 0; i < chunk_size; i++) {
        if (chunk[i] && strlen(chunk[i]) > 0) {
            int count = hashmap_get(freq_map, chunk[i]);
            hashmap_put(freq_map, chunk[i], count + 1);
        }
    }
    
    return;
}