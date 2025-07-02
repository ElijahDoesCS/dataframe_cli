// marshaller.c
#include "marshaller.h"
#include <stdio.h>

int marshall_operations(char **subregion, int sub_height, int sub_width, short operations) {
    // For now, just print what you received
    
    printf("Hello from marshaller\n");

    // printf("Marshaller called with sub_height=%d, sub_width=%d, operations=%d\n", sub_height, sub_width, operations);
    // for (int i = 0; i < sub_height * sub_width; i++) {
    //     printf("%s ", subregion[i]);
    //     if ((i + 1) % sub_width == 0) printf("\n");
    // }
    // Later: perform actual operations
    return 0;
}