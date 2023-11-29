#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>

typedef struct {
    size_t buffer_size;
    size_t num_producers;
    size_t num_consumers;
    unsigned int upper_limit;
} input_params;

int get_input_params(int argc, char *argv[], input_params *params);

#endif
