#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "k_way.h"
#include "../../fat_data/fat_data.h"  // For compare_big_numbers

typedef struct {
    char *value;
    int chunk_index;
    int element_index;
} HeapNode;

void swap(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_down(HeapNode heap[], int size, int i) {
    int smallest = i;
    int left = 2*i + 1;
    int right = 2*i + 2;

    if (left < size && compare_big_numbers(heap[left].value, heap[smallest].value) < 0)
        smallest = left;
    if (right < size && compare_big_numbers(heap[right].value, heap[smallest].value) < 0)
        smallest = right;
    if (smallest != i) {
        swap(&heap[i], &heap[smallest]);
        heapify_down(heap, size, smallest);
    }
}

void heapify_up(HeapNode heap[], int i) {
    if (i == 0) return;
    int parent = (i - 1) / 2;
    if (compare_big_numbers(heap[i].value, heap[parent].value) < 0) {
        swap(&heap[i], &heap[parent]);
        heapify_up(heap, parent);
    }
}

char **k_way_merge(char ***chunks, int *chunk_sizes, int num_threads, int subregion_length) {
    // Final result array
    char **result = malloc(sizeof(char*) * subregion_length);
    if (!result) {
        perror("Failed to allocate result array");
        exit(EXIT_FAILURE);
    }

    // Heap of current elements (one per chunk)
    HeapNode *heap = malloc(sizeof(HeapNode) * num_threads);
    int heap_size = 0;

    // Track progress through each chunk
    int *positions = calloc(num_threads, sizeof(int));
    if (!positions) {
        perror("Failed to allocate positions array");
        free(result);
        exit(EXIT_FAILURE);
    }

    // Initialize heap with first element of each chunk
    for (int i = 0; i < num_threads; i++) {
        if (chunk_sizes[i] > 0) {
            heap[heap_size++] = (HeapNode){chunks[i][0], i, 0};
            heapify_up(heap, heap_size - 1);
        }
    }

    // Merge loop
    int result_index = 0;
    while (heap_size > 0) {
        // Get smallest element
        HeapNode min = heap[0];
        result[result_index++] = min.value;

        // Advance in that chunk
        int ci = min.chunk_index;
        int ei = ++positions[ci];

        if (ei < chunk_sizes[ci]) {
            heap[0] = (HeapNode){chunks[ci][ei], ci, ei};
        } else {
            heap[0] = heap[--heap_size];
        }

        heapify_down(heap, heap_size, 0);
    }

    // Cleanup
    free(heap);
    free(positions);

    return result;
}


