#include <stdio.h>
#include <stdlib.h>

# include "input.h"

input_params params;

int main(int argc, char *argv[]) {
    if (get_input_params(argc, argv, &params) != 0) {
        return 1;
    }

    printf("Buffer size: %zu\n", params.buffer_size);
    printf("Number of producers: %zu\n", params.num_producers);
    printf("Number of consumers: %zu\n", params.num_consumers);
    printf("Upper limit: %d\n", params.upper_limit);

    return 0; 
}
