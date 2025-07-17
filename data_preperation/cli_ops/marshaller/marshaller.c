// marshaller.c
#include "marshaller.h"
#include "../../arithmetic_lib/fat_data/fat_data.h"
#include "../martix_lib.h"
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

    // Optionally pass shared result struct pointer
} thread_args_t;

void *thread_operations(void *args) {
    thread_args_t *targs = (thread_args_t *)args;
    char **subregion = targs->subregion;
    int start_idx = targs->start_idx;
    int end_idx = targs->end_idx;
    int operations = targs->operations;
    int chunk_size = targs->chunk_size;

    // Initialize the local thread chunk
    targs->local_values = malloc(sizeof(char *) * chunk_size);
    for (int i = start_idx, j = 0; i < end_idx; i++, j++) {
        if (!subregion[i]) {
            fprintf(stderr, "WARNING: subregion[%d] is NULL\n", i);
        }

        // Set shallow references
        targs->local_values[j] = subregion[i];
    }

    //     // Bitwise checks for each operation
    // if (operations & OP_MAX) {
    //     printf("[Thread] Performing MAX operation...\n");
    //     // TODO: compute local max
    // }

    // if (operations & OP_MIN) {
    //     printf("[Thread] Performing MIN operation...\n");
    //     // TODO: compute local min
    // }

    // if (operations & OP_MEAN) {
    //     printf("[Thread] Performing MEAN operation...\n");
    //     // TODO: compute local mean
    // }

    // if (operations & OP_MEDIAN) {
    //     printf("[Thread] Performing MEDIAN operation...\n");
    //     // TODO: compute local median
    // }

    // if (operations & OP_MODE) {
    //     printf("[Thread] Performing MODE operation...\n");
    //     // TODO: compute local mode
    // }

    // if (operations & OP_STDDEV) {
    //     printf("[Thread] Performing STDDEV operation...\n");
    //     // TODO: compute local standard deviation
    // }

    return NULL;
}

void thread_cleanup(pthread_t *threads, int thread_count) {
    for (int i = 0; i < thread_count; i++) {
        // Join threads and free the threads array allocated on heap
        pthread_join(threads[i], NULL);
       
    }
    free(threads);

   return;
}

void print_thread_structs(thread_args_t *thread_args, int num_threads) {
        // Now print all thread results
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
    }
}

void thread_struct_cleanup(thread_args_t *thread_args, int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        // Free the array of structures allocated on heap, freeing the local_values on heap in each struct
        if (thread_args[i].local_values) free(thread_args[i].local_values);
        thread_args[i].local_values = NULL;
    }

    return;
}

int marshall_operations(char **subregion, int sub_height, int sub_width, int subregion_size, int operations, int thread_count) {
    // If we pass invalid data, exit
    if (!subregion || sub_height <= 0 || sub_width <= 0) {
        fprintf(stderr, "Invalid subregion dimensions.\n");
        return 1;
    }
    // Prevent overlapping chunk allocation to threads
    else if (thread_count > subregion_size) thread_count = subregion_size;

    // Pretty print the subregion
    pretty_print_values(subregion, subregion_size, sub_width);

    printf("From marshaller\n");
    printf("Operations: %d\n", operations);
    printf("Thread count: %d\n\n",  thread_count);

    // Allocate an array of threads and thread structs per thread
    pthread_t *threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
    thread_args_t *thread_args = (thread_args_t *)malloc(thread_count * sizeof(thread_args_t));

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

        // Send threads to build their chunk and compute vals from them
        pthread_create(&threads[i], NULL, thread_operations, &thread_args[i]);

        current_index = end_idx;  // Move to the next chunk
    }

    thread_cleanup(threads, thread_count);

    print_thread_structs(thread_args, thread_count);

    thread_struct_cleanup(thread_args, thread_count);

    return 0;
}