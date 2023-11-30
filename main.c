#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include "helpers.h"

/* Define the lock functions depending on the type of lock */
#ifdef MUTEX
#define lock_producer(params) pthread_mutex_lock(&params->prod_mutex)
#define unlock_producer(params) pthread_mutex_unlock(&params->prod_mutex)
#define lock_consumer(params) pthread_mutex_lock(&params->con_mutex)
#define unlock_consumer(params) pthread_mutex_unlock(&params->con_mutex)
#endif
#ifdef SPINLOCK
#define lock_producer(params) pthread_spin_lock(&params->prod_spinlock)
#define unlock_producer(params) pthread_spin_unlock(&params->prod_spinlock)
#define lock_consumer(params) pthread_spin_lock(&params->con_spinlock)
#define unlock_consumer(params) pthread_spin_unlock(&params->con_spinlock)
#endif

void *producer(void *param) {
    /* Get all the variables from the passed in struct */
    producer_shared_params *params = (producer_shared_params *)param;
    shared_variables *shared = params->shared;

    while (1) {
        /* Entry section for producer */
        lock_producer(params);

        /* Make CS longer */
        #ifdef EXPERIMENTAL
        for (size_t i = 0; i < shared->cs_lenth; i++);
        #endif

        /* Wait until we can produce. If buffer is full, then
         * we will wait until a consumer consumes an item. Don't release
         * lock because not other producer can produce either.
         */
        sem_wait(&shared->can_produce);
        
        /* +2 here because we place upper_limit + 1 to let
         * the consumers know that we are done producing
         */
        if (params->next_produced == shared->upper_limit + 2) {
            unlock_producer(params);
            /* Let all threads go so they will terminate */
            sem_post(&shared->can_produce);
            sem_post(&shared->can_consume);
            break;
        }
        /* Place the next_produced item into the buffer */
        shared->buffer[params->in] = params->next_produced++;
        params->in = (params->in + 1) % shared->buffer_size;

        /* Let the consumers know that they can consume */
        sem_post(&shared->can_consume);

        /* Exit section */
        unlock_producer(params);

    }
	return NULL;
}

void *consumer(void *param) {
    /* Get all the variables from the passed in struct */
    consumer_thread_params *thread_params = (consumer_thread_params *)param;
    consumer_shared_params *params = thread_params->params;
    shared_variables *shared = params->shared;

    /* Used for printing */
    #ifndef EXPERIMENTAL
    long consumer_id = thread_params->id;
    #endif

    while (1) {
        /* Consumer entry section */
        lock_consumer(params);

        /* Make CS longer */
        #ifdef EXPERIMENTAL
        for (size_t i = 0; i < params->shared->cs_lenth; i++);
        #endif

        /* Same as producer, just wait until we can consume */
        sem_wait(&shared->can_consume);

        /* Get the item from the buffer */
        int item = shared->buffer[params->out];
        /* If the item is upper_limit + 1, then we know that
         * the producer is done producing and we can exit
         */
        if (item == shared->upper_limit + 1) {
            unlock_consumer(params);
            /* Let all threads go so they will terminate */
            sem_post(&shared->can_consume);
            sem_post(&shared->can_produce);
            break;
        }

        /* Increment the out index */
        params->out = (params->out + 1) % shared->buffer_size;

        /* Let the producer know that it can produce */
        sem_post(&shared->can_produce);

        #ifndef EXPERIMENTAL
        fprintf(stdout, "%d, %ld\n", item, consumer_id);
        #endif


        /* Exit section */
        unlock_consumer(params);

    }
	return NULL;
}

int main(int argc, char *argv[]) {
    #ifdef EXPERIMENTAL
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    #endif
    input_params params;
    shared_variables shared;
    producer_shared_params producer_params;
    consumer_shared_params consumer_params;

    /* Initialize all the variables and parameters
     * needed for the producer and consumer threads
     */
    if (get_input_params(argc, argv, &params) != 0)
        return 1;
    if (initialize_shared_variables(&shared, &params) != 0)
        return 1;
    if (initialize_producer_params(&producer_params, &shared) != 0)
        return 1;
    if (initialize_consumer_params(&consumer_params, &shared) != 0)
        return 1;

    // Create producer and consumer threads
    pthread_t producers[params.num_producers];
    pthread_t consumers[params.num_consumers];

    // Consumer specific variables
    consumer_thread_params consumer_thread_params[params.num_consumers];

    for (long i = 0; i < params.num_producers; i++)
        pthread_create(&producers[i], NULL, producer, (void *)&producer_params);

    for (long i = 0; i < params.num_consumers; i++) {
        consumer_thread_params[i].params = &consumer_params;
        consumer_thread_params[i].id = i;
        pthread_create(&consumers[i], NULL, consumer, (void *)&consumer_thread_params[i]);
    }

    // Cleanup
    for (int i = 0; i < params.num_producers; i++) 
        pthread_join(producers[i], NULL);
    
    for (int i = 0; i < params.num_consumers; i++) 
        pthread_join(consumers[i], NULL);

    /* Destroy all the locks and semaphores */
    #ifdef MUTEX
    pthread_mutex_destroy(&producer_params.prod_mutex);
    pthread_mutex_destroy(&consumer_params.con_mutex);
    #endif
    #ifdef SPINLOCK
    pthread_spin_destroy(&producer_params.prod_spinlock);
    pthread_spin_destroy(&consumer_params.con_spinlock);
    #endif
    sem_destroy(&shared.can_produce);
    sem_destroy(&shared.can_consume);

    // Free the allocated buffer
    free(shared.buffer);

    #ifdef EXPERIMENTAL
    clock_gettime(CLOCK_REALTIME, &end);
    print_results(&start, &end, &params);
    #endif
    return 0;
}
