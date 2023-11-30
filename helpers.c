#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "helpers.h"

/* Get inputs from user and check for errors */
int get_input_params(int argc, char *argv[], input_params *params) {
    if (argc < 5) {
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
    /* Make sure params are greater than 0 */
    if (params->buffer_size == 0) {
        printf("Error: buffer_size must be greater than 0\n");
        return 1;
    }
    params->num_producers = strtoul(argv[2], NULL, 10);
    if (params->num_producers == 0) {
        printf("Error: num_producers must be greater than 0\n");
        return 1;
    }
    params->num_consumers = strtoul(argv[3], NULL, 10);
    if (params->num_consumers == 0) {
        printf("Error: num_consumers must be greater than 0\n");
        return 1;
    }
    params->upper_limit = (int)strtoul(argv[4], NULL, 10);
    /* Optional argument only for experimental runs */
    #ifdef EXPERIMENTAL
    if (argc == 6) {
        params->cs_lenth = strtoul(argv[5], NULL, 10);
    }
    #endif

    return 0;
}

/* Initialize variables shared between all threads */
int initialize_shared_variables(shared_variables *shared, input_params *params) {
    shared->buffer = malloc(params->buffer_size * sizeof(int));
    if (shared->buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for the buffer\n");
        return 1;
    }
    shared->buffer_size = params->buffer_size;
    shared->upper_limit = params->upper_limit;
    #ifdef EXPERIMENTAL
    shared->cs_lenth = params->cs_lenth;
    #endif
    if (sem_init(&shared->can_produce, 0, params->buffer_size) != 0) {
        fprintf(stderr, "Failed to initialize condition variable\n");
        return 1;
    }
    if (sem_init(&shared->can_consume, 0, 0) != 0) {
        fprintf(stderr, "Failed to initialize condition variable\n");
        return 1;
    }
    return 0;
}

/* Initialize shared parameters for producer threads */
int initialize_producer_params(producer_shared_params *params, shared_variables *shared) {
    params->shared = shared;
    params->next_produced = 1;
    params->in = 0;
    #ifdef MUTEX
    if (pthread_mutex_init(&params->prod_mutex, NULL) != 0) {
        fprintf(stderr, "Failed to initialize mutex\n");
        return 1;
    }
    #endif
    #ifdef SPINLOCK
    if (pthread_spin_init(&params->prod_spinlock, 0) != 0) {
        fprintf(stderr, "Failed to initialize spinlock\n");
        return 1;
    }
    #endif
    return 0;
}

/* Initialize shared parameters for consumer threads */
int initialize_consumer_params(consumer_shared_params *params, shared_variables *shared) {
    params->shared = shared;
    params->out = 0;
    #ifdef MUTEX
    if (pthread_mutex_init(&params->con_mutex, NULL) != 0) {
        fprintf(stderr, "Failed to initialize mutex\n");
        return 1;
    }
    #endif
    #ifdef SPINLOCK
    if (pthread_spin_init(&params->con_spinlock, 0) != 0) {
        fprintf(stderr, "Failed to initialize spinlock\n");
        return 1;
    }
    #endif
    return 0;
}

/* For calculating the time difference between two timespec structs
 * In MS */
long get_time_diff(struct timespec *start, struct timespec *end) {
    time_t seconds = end->tv_sec - start->tv_sec;
    long nanoseconds = end->tv_nsec - start->tv_nsec;
    /* Convert to milliseconds */
    return seconds * 1000 + nanoseconds / 1000000;
}

/* Used in larger script to run experiments */
void print_results(struct timespec *start, struct timespec *end, input_params *params) {
    #ifndef EXPERIMENTAL
    return;
    #endif
    #ifdef EXPERIMENTAL
    
    #ifdef MUTEX
    char *filename = "results_mutex.csv";
    #endif
    #ifdef SPINLOCK
    char *filename = "results_spinlock.csv";
    #endif
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        /* Need to create the file */
        fp = fopen(filename, "a");
        if (fp == NULL) {
            fprintf(stderr, "Failed to open file %s\n", filename);
            return;
        }
        fprintf(fp, "buffer_size,num_producers,num_consumers,cs_length,upper_limit,time_ms\n");
    } else {
        fclose(fp);
        fp = fopen(filename, "a");
        if (fp == NULL) {
            fprintf(stderr, "Failed to open file %s\n", filename);
            return;
        }
    }
    fprintf(fp, "%ld,%ld,%ld,%ld,%d,%ld\n", params->buffer_size,
            params->num_producers, params->num_consumers,
            params->cs_lenth,
            params->upper_limit, get_time_diff(start, end));

    fclose(fp);
    #endif
}
