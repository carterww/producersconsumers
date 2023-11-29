#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

# include "helpers.h"

void *producer(void *param) {
    producer_shared_params *params = (producer_shared_params *)param;
    shared_variables *shared = params->shared;
    while (1) {
        pthread_mutex_lock(&shared->mutex); // Start of critical section
        
        /* If the buffer is full, wait until the consumers
         * consume an item
         */
        while (shared->items_in_buffer == shared->buffer_size)
            pthread_cond_wait(&shared->can_produce, &shared->mutex);
        
        /* +2 here because we place upper_limit + 1 to let
         * the consumers know that we are done producing
         */
        if (params->next_produced == shared->upper_limit + 2) {
            pthread_cond_signal(&shared->can_produce);
            pthread_cond_signal(&shared->can_consume);
            pthread_mutex_unlock(&shared->mutex);
            break;
        }
        shared->buffer[params->in] = params->next_produced++;
        params->in = (params->in + 1) % shared->buffer_size;
        shared->items_in_buffer++;

        pthread_cond_signal(&shared->can_consume);
        pthread_mutex_unlock(&shared->mutex); // End of critical section
    }
	
	return NULL;
}

void *consumer(void *param) {
    consumer_thread_params *thread_params = (consumer_thread_params *)param;
    consumer_shared_params *params = thread_params->params;
    shared_variables *shared = params->shared;
    long consumer_id = thread_params->id;

    while (1) {
        pthread_mutex_lock(&shared->mutex); // Start of critical section

        while (shared->items_in_buffer == 0)
            pthread_cond_wait(&shared->can_consume, &shared->mutex);

        int item = shared->buffer[params->out];
        /* If the item is upper_limit + 1, then we know that
         * the producer is done producing and we can exit
         */
        if (item == shared->upper_limit + 1) {
            pthread_cond_broadcast(&shared->can_produce);
            pthread_cond_broadcast(&shared->can_consume);   
            pthread_mutex_unlock(&shared->mutex);
            break;
        }  
        params->out = (params->out + 1) % shared->buffer_size;
        shared->items_in_buffer--;

        fprintf(stdout, "%d, %ld\n", item, consumer_id);
        pthread_cond_signal(&shared->can_produce);
        pthread_mutex_unlock(&shared->mutex); // End of critical section
    }
	
	return NULL;
}


int main(int argc, char *argv[]) {
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
    pthread_mutex_destroy(&shared.mutex);
    pthread_cond_destroy(&shared.can_produce);
    pthread_cond_destroy(&shared.can_consume);
    
    // Free the allocated buffer
    free(shared.buffer);

    return 0;
}
