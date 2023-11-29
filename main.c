#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

# include "input.h"

input_params params;


// Define the buffer and shared variables
int *buffer;
int buffer_size = 0; // Current number of items in the buffer
int in = 0; // Next place to put the item in the buffer
int out = 0; // Next place to take the item from the buffer
int next_produced = 0; // Next value to be produced

// Mutex and condition variables
pthread_mutex_t mutex;
pthread_cond_t can_produce;
pthread_cond_t can_consume;

void *producer(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex); // Start of critical section
        
        while (buffer_size == params.buffer_size)
            pthread_cond_wait(&can_produce, &mutex);
        
        if (next_produced == params.upper_limit + 2) {
            pthread_cond_broadcast(&can_produce);
            pthread_cond_broadcast(&can_consume);
            pthread_mutex_unlock(&mutex);
            break;
        }
        buffer[in] = next_produced;
        next_produced++;
        in = (in + 1) % params.buffer_size;
        buffer_size++;

        pthread_cond_signal(&can_consume);
        pthread_mutex_unlock(&mutex); // End of critical section
    }
	
	return NULL;
}

void *consumer(void *id) {
    long consumer_id = (long)id;

    while (1) {
        pthread_mutex_lock(&mutex); // Start of critical section

        while (buffer_size == 0)
            pthread_cond_wait(&can_consume, &mutex);

        int item = buffer[out];
        if (item == params.upper_limit + 1) {
            pthread_cond_broadcast(&can_produce);
            pthread_cond_broadcast(&can_consume);   
            pthread_mutex_unlock(&mutex);
            break;
        }  
        out = (out + 1) % params.buffer_size;
        buffer_size--;

        fprintf(stdout, "%d, %ld\n", item, consumer_id);
        pthread_cond_signal(&can_produce);
        pthread_mutex_unlock(&mutex); // End of critical section
    }
	
	return NULL;
}


int main(int argc, char *argv[]) {
    get_input_params(argc, argv, &params);

    // Allocate memory for the buffer based on the determined size
    buffer = (int *)malloc(params.buffer_size * sizeof(int));
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for the buffer\n");
        return 1;
    }

    // Initialize mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&can_produce, NULL);
    pthread_cond_init(&can_consume, NULL);

    // Create producer and consumer threads
    pthread_t producers[params.num_producers];
    pthread_t consumers[params.num_consumers];

    for (long i = 0; i < params.num_producers; i++)
        pthread_create(&producers[i], NULL, producer, NULL);

    for (long i = 0; i < params.num_consumers; i++)
        pthread_create(&consumers[i], NULL, consumer, (void *)i);

    // Cleanup
    for (int i = 0; i < params.num_producers; i++) 
        pthread_join(producers[i], NULL);
    
    for (int i = 0; i < params.num_consumers; i++) 
        pthread_join(consumers[i], NULL);   
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&can_produce);
    pthread_cond_destroy(&can_consume);
    
    // Free the allocated buffer
    free(buffer);

    return 0;
}
