#ifndef STATISTICAL_OPS_H
#define STATISTICAL_OPS_H

#include <stddef.h> // for size_t

void compute_local_max(char **chunk, int chunk_size, char *result);
void compute_local_min(char **chunk, int chunk_size, char *result);
void compute_local_sum(char **chunk, int chunk_size, char *result);
void compute_local_counts(char **chunk, int chunk_size, char *result);
// void compute_median(char **subregion, int subregion_size, char *result);

#endif // STATISTICAL_OPS_H