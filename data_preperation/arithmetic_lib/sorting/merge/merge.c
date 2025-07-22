#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "merge.h"
#include "../../fat_data/fat_data.h"

void merge(char **chunk, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    char **L = malloc(n1 * sizeof(char *));
    char **R = malloc(n2 * sizeof(char *));
    if (!L || !R) {
        fprintf(stderr, "malloc failed in merge\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n1; i++) L[i] = chunk[left + i];
    for (int j = 0; j < n2; j++) R[j] = chunk[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (compare_big_numbers(L[i], R[j]) <= 0) {
            chunk[k++] = L[i++];
        } else {
            chunk[k++] = R[j++];
        }
    }

    while (i < n1) chunk[k++] = L[i++];
    while (j < n2) chunk[k++] = R[j++];

    free(L);
    free(R);
}

void merge_sort_interface(char **chunk, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort_interface(chunk, left, mid);
        merge_sort_interface(chunk, mid + 1, right);
        merge(chunk, left, mid, right);
    }
}

void merge_sort(char **chunk, int chunk_size) {
    int left = 0;
    int right =  chunk_size - 1;
    merge_sort_interface(chunk, left, right);
} 