#ifndef HELPERS_H 
#define HELPERS_H

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/* For LSP and in case no define is 
 * specified at compile time */
#ifndef SPINLOCK
#ifndef MUTEX
#define MUTEX
#endif
#endif

/* Inputs from command line */
typedef struct {
    size_t buffer_size;
    size_t num_producers;
    size_t num_consumers;
    unsigned int upper_limit;
#ifdef EXPERIMENTAL
    size_t cs_lenth; // For experiment only
#endif
} input_params;

/* Variables shared between producers and consumers */
typedef struct {
    int *buffer;
    size_t buffer_size;
    unsigned int upper_limit;
    sem_t can_produce;
    sem_t can_consume;
#ifdef EXPERIMENTAL
    size_t cs_lenth;
#endif
} shared_variables;

/* Parameters shared between producers */
typedef struct {
    shared_variables *shared;
#ifdef MUTEX
    pthread_mutex_t prod_mutex;
#endif
#ifdef SPINLOCK
    pthread_spinlock_t prod_spinlock;
#endif
    int next_produced;
    int in;
} producer_shared_params;

/* Parameters shared between consumers */
typedef struct {
    shared_variables *shared;
#ifdef MUTEX
    pthread_mutex_t con_mutex;
#endif
#ifdef SPINLOCK
    pthread_spinlock_t con_spinlock;
#endif
    int out;
} consumer_shared_params;

/* Parameters specific to each consumer thread */
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

/* Functions for calculating performance and printing results */
long get_time_diff(struct timespec *start, struct timespec *end);
void print_results(struct timespec *start, struct timespec *end, input_params *params);

#endif
