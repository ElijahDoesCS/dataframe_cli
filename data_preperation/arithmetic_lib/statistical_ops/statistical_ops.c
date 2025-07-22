#include "statistical_ops.h"
#include "../fat_data/fat_data.h"
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

    // printf("current_result max: %s\n", current_result);

    strncpy(result, current_result, MAX_NUMBER_LENGTH - 1);
    result[MAX_NUMBER_LENGTH - 1] = '\0';

    return;
}

void compute_local_min(char **chunk, int chunk_size, char *result) {
    // printf("hi from compute local min\n");
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

    // printf("current_result min: %s\n", current_result);

    strncpy(result, current_result, MAX_NUMBER_LENGTH - 1);
    result[MAX_NUMBER_LENGTH - 1] = '\0'; // Pretty sure strncpy already delimits

    return;
}

void compute_local_sum(char **chunk, int chunk_size, char *result) {
    // printf("hi from local sum\n");
    char temp_result[MAX_NUMBER_LENGTH] = "0";
    char temp_sum[MAX_NUMBER_LENGTH];

    for (int i = 0; i < chunk_size; i++) {
        add_big_integers(temp_result, chunk[i], temp_sum); // temp_sum = temp_result + chunk[i]
        strncpy(temp_result, temp_sum, MAX_NUMBER_LENGTH); // update temp_result
    }

    strncpy(result, temp_result, MAX_NUMBER_LENGTH);
}


void compute_local_counts(char **chunk, int chunk_size, char *result) {
    
    
    return;
}