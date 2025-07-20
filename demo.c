#include <stdlib.h>
#include <stdio.h>
#include "./data_preperation/arithmetic_lib/fat_data/fat_data.h"
#include "./data_preperation/arithmetic_lib/statistical_ops/statistical_ops.h"
#include "./data_preperation/cli_ops/marshaller/marshaller.h"
// #include "./data_preperation/cli_ops/matrix_lib.h"

int main() {
    printf("Comparing these numbers %d\n", compare_big_numbers("23452", "85678"));

    return 0;
}