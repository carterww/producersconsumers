#ifndef HELPERS_H 
#define HELPERS_H

#include <stdlib.h>

/* Inputs from command line */
typedef struct {
    size_t buffer_size;
    size_t num_producers;
    size_t num_consumers;
    unsigned int upper_limit;
} input_params;

/* Variables shared between producers and consumers */
typedef struct {
    int *buffer;
    size_t buffer_size;
    size_t items_in_buffer;
    unsigned int upper_limit;
    pthread_mutex_t mutex;
    pthread_cond_t can_produce;
    pthread_cond_t can_consume;
} shared_variables;

/* Parameters for producers and consumers */
typedef struct {
    shared_variables *shared;
    int next_produced;
    int in;
} producer_shared_params;

typedef struct {
    shared_variables *shared;
    int out;
} consumer_shared_params;

typedef struct {
    consumer_shared_params *params;
    long id;
} consumer_thread_params;


/* Functions to get input parameters */
int get_input_params(int argc, char *argv[], input_params *params);

/* Functions to initialize structs */
int initialize_shared_variables(shared_variables *shared, input_params *params);
int initialize_producer_params(producer_shared_params *params, shared_variables *shared);
int initialize_consumer_params(consumer_shared_params *params, shared_variables *shared);

#endif
