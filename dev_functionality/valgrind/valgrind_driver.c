// asan/asan_driver.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Your shared library's entry point
extern int load_data(const char *, const char *, const char *, const char *, const char *, int, int);

int main(int argc, char *argv[]) {
    // Probably redundant
    if (argc != 8) {
        fprintf(stderr, "Usage: %s <file> <y0> <y1> <x0> <x1> <operations> <thread_count>\n", argv[0]);
        return 1;
    }

    const char *file = argv[1];
    const char *y0 = argv[2];
    const char *y1 = argv[3];
    const char *x0 = argv[4];
    const char *x1 = argv[5];
    int operations = atoi(argv[6]);
    int thread_count = atoi(argv[7]);

    printf("--------------------\n");
    printf("[RUNNING] %s %s %s %s %s\n", file, y0, y1, x0, x1);

    int result = load_data(file, y0, y1, x0, x1, operations, thread_count);

    if (result != 0) {
        fprintf(stderr, "[ERROR] Command failed with error code: %d\n", result);
    }

    return result;
}


