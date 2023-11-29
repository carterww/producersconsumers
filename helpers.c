#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "helpers.h"

int get_input_params(int argc, char *argv[], input_params *params) {
    if (argc != 5) {
        printf("Usage: %s <buffer_size> <num_producers> <num_consumers> <upper_limit>\n", argv[0]);
        return 1;
    }
    for (int i = 1; i < 5; i++) {
        unsigned long len = strlen(argv[i]);
        if (len != 0 && argv[i][0] == '-') {
            printf("Error: argument \"%s\" must be positive\n", argv[i]);
            return 1;
        }
    }

    params->buffer_size = strtoul(argv[1], NULL, 10);
    params->num_producers = strtoul(argv[2], NULL, 10);
    params->num_consumers = strtoul(argv[3], NULL, 10);
    params->upper_limit = (int)strtoul(argv[4], NULL, 10);

    return 0;
}


int initialize_shared_variables(shared_variables *shared, input_params *params) {
    shared->buffer = malloc(params->buffer_size * sizeof(int));
    if (shared->buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for the buffer\n");
        return 1;
    }
    shared->items_in_buffer = 0;
    shared->buffer_size = params->buffer_size;
    shared->upper_limit = params->upper_limit;
    /* Initialize pthread stuff */
    if (pthread_mutex_init(&shared->mutex, NULL) != 0) {
        fprintf(stderr, "Failed to initialize mutex\n");
        return 1;
    }
    if (pthread_cond_init(&shared->can_produce, NULL) != 0) {
        fprintf(stderr, "Failed to initialize condition variable\n");
        return 1;
    }
    if (pthread_cond_init(&shared->can_consume, NULL) != 0) {
        fprintf(stderr, "Failed to initialize condition variable\n");
        return 1;
    }
    return 0;
}
int initialize_producer_params(producer_shared_params *params, shared_variables *shared) {
    params->shared = shared;
    params->next_produced = 1;
    params->in = 0;
    return 0;
}

int initialize_consumer_params(consumer_shared_params *params, shared_variables *shared) {
    params->shared = shared;
    params->out = 0;
    return 0;
}
