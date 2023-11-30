#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include "helpers.h"

/* Use these to make single call handled
 * by both cases
 */
void lock(shared_variables *shared) {
    #ifdef MUTEX
    pthread_mutex_lock(&shared->mutex);
    #endif
    #ifdef SPINLOCK
    pthread_spin_lock(&shared->spinlock);
    #endif
}
void unlock(shared_variables *shared) {
    #ifdef MUTEX
    pthread_mutex_unlock(&shared->mutex);
    #endif
    #ifdef SPINLOCK
    pthread_spin_unlock(&shared->spinlock);
    #endif
}

void *producer(void *param) {
    /* Get all the variables from the passed in struct */
    producer_shared_params *params = (producer_shared_params *)param;
    shared_variables *shared = params->shared;

    while (1) {
        /* This is the entry section.
         * No two producers should be in this section at the same time
         * because things like shared->items_in_buffer, params->in, and
         * params->next_produced are shared between all the producers
         * and some by consumers.
         */
        lock(shared);
        /* Make CS longer */
        #ifdef EXPERIMENTAL
        for (size_t i = 0; i < shared->cs_lenth; i++);
        #endif

        /* If the buffer is full, then we wait until a consumer
         * consumes an item and signals us that we can produce
         * again
         */
        if (sem_trywait(&shared->can_produce) != 0) {
            unlock(shared);
            sem_wait(&shared->can_produce);
            lock(shared);
        }
        
        /* +2 here because we place upper_limit + 1 to let
         * the consumers know that we are done producing
         */
        if (params->next_produced == shared->upper_limit + 2) {
            unlock(shared);
            sem_post(&shared->can_produce);
            sem_post(&shared->can_consume);
            break;
        }
        /* Place the next_produced item into the buffer */
        shared->buffer[params->in] = params->next_produced++;
        params->in = (params->in + 1) % shared->buffer_size;

        /* Exit section */
        unlock(shared);

        /* Signal the consumers that they can consume.
         * Needs to be outside of the critical section
         * in case this causes a context switch. If that
         * happened, the lock would still be held.
         */
        sem_post(&shared->can_consume);
    }
	
	return NULL;
}

void *consumer(void *param) {
    /* Get all the variables from the passed in struct */
    consumer_thread_params *thread_params = (consumer_thread_params *)param;
    consumer_shared_params *params = thread_params->params;
    shared_variables *shared = params->shared;
    #ifndef EXPERIMENTAL
    long consumer_id = thread_params->id;
    #endif

    while (1) {
        /* This is the entry section.
         * Similar to the producer, no two consumers should be in this
         * section at the same time because things like shared->items_in_buffer,
         * params->out, and shared->buffer are shared between all the consumers
         * and some by producers.
         */
        lock(shared);
        /* Make CS longer */
        #ifdef EXPERIMENTAL
        for (size_t i = 0; i < params->shared->cs_lenth; i++);
        #endif

        /* Same as producer, just wait until we can consume */
        if (sem_trywait(&shared->can_consume) != 0) {
            unlock(shared);
            sem_wait(&shared->can_consume);
            lock(shared);
        }

        int item = shared->buffer[params->out];
        /* If the item is upper_limit + 1, then we know that
         * the producer is done producing and we can exit
         */
        if (item == shared->upper_limit + 1) {
            unlock(shared);
            sem_post(&shared->can_consume);
            sem_post(&shared->can_produce);
            break;
        }
        params->out = (params->out + 1) % shared->buffer_size;

        #ifndef EXPERIMENTAL
        fprintf(stdout, "%d, %ld\n", item, consumer_id);
        #endif
        /* Exit section */
        unlock(shared);

        /* After unlock for same reason as producer */
        sem_post(&shared->can_produce);
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

    #ifdef MUTEX
    pthread_mutex_destroy(&shared.mutex);
    #endif
    #ifdef SPINLOCK
    pthread_spin_destroy(&shared.spinlock);
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
