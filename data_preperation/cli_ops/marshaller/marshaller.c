// marshaller.c
#include "marshaller.h"
#include "../../arithmetic_lib/fat_data/fat_data.h"
#include "../martix_lib.h"
#include "../../arithmetic_lib/statistical_ops/statistical_ops.h"
#include "../../arithmetic_lib/sorting/merge/merge.h"
#include "../../arithmetic_lib/sorting/k_way/k_way.h"
#include "../../arithmetic_lib/hashmap/hashmap.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  

#define OP_MAX      1
#define OP_MIN      2
#define OP_MEAN     4
#define OP_MEDIAN   8
#define OP_MODE     16
#define OP_STDDEV   32

typedef struct {
    char **subregion;
    int start_idx;       // inclusive
    int end_idx;         // exclusive
    int chunk_size;
    int operations;

    // Result caching
    char local_sum[MAX_NUMBER_LENGTH];   // for mean
    char local_min[MAX_NUMBER_LENGTH];
    char local_max[MAX_NUMBER_LENGTH];
    char **local_values;                 // for median/mode
    int local_count;

    hashmap_t *local_freq_map; // Store the counts for each value for mode

    // Optionally pass shared result struct pointer
} thread_args_t;

typedef struct {
    char max_result[MAX_NUMBER_LENGTH];
    char min_result[MAX_NUMBER_LENGTH];
    char mean_result[MAX_NUMBER_LENGTH];
    char median_result[MAX_NUMBER_LENGTH];
    char mode_result[MAX_NUMBER_LENGTH];
    char stddev_result[MAX_NUMBER_LENGTH];
} final_args_t;

void print_thread_structs(thread_args_t *thread_args, int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        printf("Thread [%d]\n", i);
        printf("Start index: %d\n", thread_args[i].start_idx);
        printf("End index:   %d\n", thread_args[i].end_idx);
        printf("Chunk size:   %d\n", thread_args[i].chunk_size);
        printf("Operations:  %d\n", thread_args[i].operations);
        printf("\n");

        printf("  Local values:\n");
        for (int j = 0; j < thread_args[i].chunk_size; j++) {
            if (thread_args[i].local_values && thread_args[i].local_values[j]) {
                printf("    [%d] %s\n", j, thread_args[i].local_values[j]);
            } else {
                printf("    [%d] (NULL)\n", j);
            }
        }
        printf("\n");  

        if (thread_args[i].operations & OP_MAX) printf("Local max: %s\n", thread_args[i].local_max);
        if (thread_args[i].operations & OP_MIN) printf("Local min: %s\n", thread_args[i].local_min);
        if (thread_args[i].operations & OP_MEAN) printf("Local mean (sum): %s\n", thread_args[i].local_sum);
        // printf("\n");
    }
}

void print_final_results(final_args_t *final_results, int operations) {
        printf("\nAggregate results\n");
        if (operations & OP_MAX) {
            printf("    Max: %s\n", final_results->max_result);
        }
        if (operations & OP_MIN) {
            printf("    Min: %s\n", final_results->min_result);
        }
        if (operations & OP_MEAN) {
            printf("   Mean: %s\n", final_results->mean_result);
        }
        if (operations & OP_MEDIAN) {
            printf(" Median: %s\n", final_results->median_result);
        }
        if (operations & OP_MODE) {
            printf("   Mode: %s\n", final_results->mode_result);
        }
        if (operations & OP_STDDEV) {
            printf(" Stddev: %s\n", final_results->stddev_result);
        } 
}

void *thread_operations(void *args) {
    thread_args_t *targs = (thread_args_t *)args;
    char **subregion = targs->subregion;
    int start_idx = targs->start_idx;
    int end_idx = targs->end_idx;
    int operations = targs->operations;
    int chunk_size = targs->chunk_size;

    // Initialize the local thread chunk
    targs->local_values = malloc(sizeof(char *) * chunk_size);
    if (!targs->local_values) {
        fprintf(stderr, "Malloc failed in thread\n");
        pthread_exit((void *)1);  // 1 = failure
    }

    for (int i = start_idx, j = 0; i < end_idx; i++, j++) {
        if (!subregion[i]) {
            fprintf(stderr, "WARNING: subregion[%d] is NULL\n", i);
            pthread_exit((void *)1);
        }

        // Set shallow references
        targs->local_values[j] = subregion[i];
    }

    // Bitwise checks for each operation, compute val on chunk and store in thread structure
    if ((operations & OP_MAX) && !(operations & OP_MEDIAN)) {
        // printf("We are computing the max\n");
        compute_local_max(targs->local_values, targs->chunk_size, targs->local_max); 
    }
    else if (operations & OP_MIN) {
        // Set null by default
        char *cpy = "\0";
        strncpy(targs->local_min, cpy, MAX_NUMBER_LENGTH);
    }

    if ((operations & OP_MIN) && !(operations & OP_MEDIAN)) {
        // printf("We computing the min\n");
        compute_local_min(targs->local_values, targs->chunk_size, targs->local_min);
    }
    else if (operations & OP_MIN) {
        // Set null by default
        char *cpy = "\0";
        strncpy(targs->local_min, cpy, MAX_NUMBER_LENGTH);
    }

    if (operations & OP_MEAN) {
        compute_local_sum(targs->local_values, targs->chunk_size, targs->local_sum);
    }

    if (operations & OP_MEDIAN) {
        merge_sort(targs->local_values, targs->chunk_size); // Sort chunk
    }

    if (operations & OP_MODE) { 
        printf("Computing local counts for mode\n");
        targs->local_freq_map = hashmap_create();
        if (!targs->local_freq_map) {
            fprintf(stderr, "Error creating hashmap for local frequency map\n");
            pthread_exit((void *)1);  // 1 = failure
        }

        // Compute local counts
        compute_local_counts(targs->local_values, targs->chunk_size, targs->local_freq_map);
        // hashmap_print(targs->local_freq_map);
    }

    if (operations & OP_STDDEV) {        
    }     

    return NULL;
}

void thread_cleanup(pthread_t *threads, int thread_count) {
    void *retval;
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], retval);

        if ((long)retval != 0) {
            fprintf(stderr, "Thread %d exited with failure code %ld\n", i, (long)retval);

            // Handle status code...
        }
    }
    free(threads);

   return;
}

void thread_structs_cleanup(thread_args_t *thread_args, final_args_t *final_args,
                            int num_threads, int operations, int subregion_size, int sub_width,
                            char **subregion) {
    char max_result[MAX_NUMBER_LENGTH];
    char min_result[MAX_NUMBER_LENGTH];
    char mean_result[MAX_NUMBER_LENGTH];    
    char median_result[MAX_NUMBER_LENGTH];    
    char *mode_result = "hi\0";
    char stddev_result[MAX_NUMBER_LENGTH];

    char subregion_len[MAX_NUMBER_LENGTH];
    snprintf(subregion_len, MAX_NUMBER_LENGTH, "%d", subregion_size);
    subregion_len[MAX_NUMBER_LENGTH - 1] = '\0';

    // mother fuck
    char **merged_array = NULL;
    if (operations & OP_MEDIAN) {
        char ***k_way = malloc(sizeof(char **) * num_threads);
        int *chunk_sizes = malloc(sizeof(int) * num_threads);
        for (int i = 0; i < num_threads; i++) {
            k_way[i] = thread_args[i].local_values;
            chunk_sizes[i] = thread_args[i].chunk_size;
        }

        merged_array = k_way_merge(k_way, chunk_sizes, num_threads, subregion_size);

        pretty_print_values(merged_array, subregion_size, sub_width);

        // Just take the damned indeces
        if (operations & OP_MAX) {
            strncpy(max_result, merged_array[subregion_size - 1], MAX_NUMBER_LENGTH);
        }
        if (operations & OP_MIN) {
            strncpy(min_result, merged_array[0], MAX_NUMBER_LENGTH);
        }

        // Obtain the actual median from the sorted array
        if (subregion_size % 2 == 1) {
            strncpy(median_result, merged_array[subregion_size / 2], MAX_NUMBER_LENGTH);
        }
        else {
                char temp_sum[MAX_NUMBER_LENGTH];
                char *divide_by_2 = "2\0";
                add_big_integers(merged_array[(subregion_size / 2) - 1],
                    merged_array[subregion_size / 2],
                    temp_sum);
                divide_big_decimals(temp_sum, divide_by_2, DEFAULT_PRECISION, median_result);
        }
    }

    // Initialize the final hashmap for mode globally
    hashmap_t *final_map = NULL;
    if (operations & OP_MODE) {
        final_map = hashmap_create();
        if (!final_map) {
            fprintf(stderr, "Error creating final hashmap for mode\n");
            return;
        }
    } 
    if (!final_map) {
        fprintf(stderr, "Final hashmap for mode is NULL\n");
        return;
    }
  

    for (int i = 0; i < num_threads; i++) {

        // Bitwise checks for each operation
        if ((operations & OP_MAX) && !(operations & OP_MEDIAN)) {
            // printf("we are doing this\n");
            // Take the max of each chunk's max
            if (i == 0) {
                // Initialize max_result with the first thread's result
                strncpy(max_result, thread_args[i].local_max, MAX_NUMBER_LENGTH - 1);
            } else {
                if (compare_big_numbers(thread_args[i].local_max, max_result) == 1) {
                    strncpy(max_result, thread_args[i].local_max, MAX_NUMBER_LENGTH - 1);
                }
            }
        }

        if ((operations & OP_MIN) && !(operations & OP_MEDIAN)) {
            // printf("we are taking the global min here\n");
            // Take the min of each local min
            if (i == 0) {
                // Initialize min_result with the first thread's result
                strncpy(min_result, thread_args[i].local_min, MAX_NUMBER_LENGTH - 1);
            } else {
                if (compare_big_numbers(thread_args[i].local_min, min_result) == -1) {
                    strncpy(min_result, thread_args[i].local_min, MAX_NUMBER_LENGTH - 1);
                }
            }
        }

        if (operations & OP_MEAN) {
            if (i == 0) {
                strncpy(mean_result, thread_args[i].local_sum, MAX_NUMBER_LENGTH - 1);
                mean_result[MAX_NUMBER_LENGTH - 1] = '\0'; // Ensure null-termination
            } else {
                char temp_result[MAX_NUMBER_LENGTH];
                add_big_integers(mean_result, thread_args[i].local_sum, temp_result);
                strncpy(mean_result, temp_result, MAX_NUMBER_LENGTH - 1);
                mean_result[MAX_NUMBER_LENGTH - 1] = '\0';
            }
        }

        if (operations & OP_MODE) {   
            // Merge the current hashmap into the final one
            hashmap_merge(final_map, thread_args[i].local_freq_map);
        }

        if (operations & OP_STDDEV) {

        }  

        // Free the array of structures allocated on heap, freeing the local_values on heap in each struct
        if (thread_args[i].local_values) free(thread_args[i].local_values);
        thread_args[i].local_values = NULL;

        // Cleanup the local frequency map
        if (thread_args[i].local_freq_map) {
            hashmap_destroy(thread_args[i].local_freq_map);
            thread_args[i].local_freq_map = NULL;
        }
    }

    mode_result = get_mode_key(final_map);

    strncpy(final_args->max_result, max_result, MAX_NUMBER_LENGTH - 1);
    strncpy(final_args->min_result, min_result, MAX_NUMBER_LENGTH - 1);
    divide_big_decimals(mean_result, subregion_len, DEFAULT_PRECISION, final_args->mean_result);
    strncpy(final_args->median_result, median_result, MAX_NUMBER_LENGTH - 1);
    strncpy(final_args->mode_result, mode_result, MAX_NUMBER_LENGTH - 1);
    strncpy(final_args->stddev_result, stddev_result, MAX_NUMBER_LENGTH - 1);

    free(thread_args);
    if (operations & OP_MODE) hashmap_destroy(final_map);

    return;
}


int marshall_operations(char **subregion, int sub_height, int sub_width, int subregion_size, int operations, int thread_count) {
    if (!subregion || sub_height <= 0 || sub_width <= 0) {
        fprintf(stderr, "Invalid subregion dimensions.\n");
        return 1;
    }
    // Prevent overlapping chunk allocation to threads
    else if (thread_count > subregion_size) thread_count = subregion_size;

    // // Pretty print the subregion
    pretty_print_values(subregion, subregion_size, sub_width);

    // Allocate an array of threads and thread structs per thread
    pthread_t *threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
    thread_args_t *thread_args = (thread_args_t *)malloc(thread_count * sizeof(thread_args_t));

    final_args_t final_answers;

    // Divvy up the subregion array row wise by threads
    int chunk_size = subregion_size / thread_count;
    int remainder = subregion_size % thread_count;

    int current_index = 0;
    for (int i = 0; i < thread_count; i++) {
        // Distribute the array into chunks across threads
        int this_chunk_size = chunk_size + (i < remainder ? 1 : 0);  // distribute extra elements to first threads
        int start_idx = current_index;
        int end_idx = current_index + this_chunk_size;

        thread_args[i].subregion = subregion;
        thread_args[i].start_idx = start_idx;
        thread_args[i].end_idx = end_idx;
        thread_args[i].chunk_size = this_chunk_size;
        thread_args[i].operations = operations;
        // printf("operations %d: %d", i, operations);
        

        // Send threads to build their chunk and compute vals from them
        int thread_creation = pthread_create(&threads[i], NULL, thread_operations, &thread_args[i]);
        if (thread_creation != 0) break;

        current_index = end_idx;  // Move to the next chunk
    }

    thread_cleanup(threads, thread_count);
    
    print_thread_structs(thread_args, thread_count);
    thread_structs_cleanup(thread_args, &final_answers, thread_count, operations, subregion_size, sub_width, subregion);
        
    print_final_results(&final_answers, operations);

    return 0;
}