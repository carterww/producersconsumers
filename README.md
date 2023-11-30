# Producers-Consumers Problem
## Problem Description
Design a program that uses n producers and m consumers to write/read from a buffer of integers up to a certain number. The buffer can only fit k items. This needs to be done while avoiding race condidtions, buffer overflow/underflow, and minimizing the number of shared variables.
## Compiling
Use make to compile the program. There are four different commands to make different versions of the program:
1. *"make mutex"*: Makes the version of the program that uses pthread's "mutex" type for locking and unlocking.
2. *"make spinlock"*: Makes the version of the program that uses pthread's "spinlock" type for locking and unlocking.
3. *"make mutex-exp"*: This is the same as the "make mutex" except it is used for running experiments. It does not print to stdout, times itself, and writes to a csv file, "results-mutex.csv."
4. *"make spinlock-exp"*: This is the same as "make mutex-exp" except it uses spinlock and writes to "results-spinlock.csv."
There is also a *"make all"* to compile all of these.
## Analysis
1. Analyze the relationship between producer-consumer problem and critical section problem. Relationship includes similarity and difference, more importantly, the hierachy.
    - The critical section problem describes an issue where multiple concurrently running processes are accessing and writing to shared variables. The sections of code that use these shared  variables are called "critical sections." If no measures are taken to enforce mutual exclusion, then a race condition can occur and it will lead to unpredicatble results in some circumstances.
    - The producer-consumer problem is an instance of the critical section problem. It describes a problem where two classes of processes, producer and consumer, are sharing a section of memory. The producer is placing things into this memory and the consumer is reading from the memory. Both processes can run concurrently. They also share a number of variables like the size of memory, number of items in the memory, and the actual memory space they are reading/writing to.
    - The producer-consumer problem is a specific instance of the critical section problem; therefore, they are very similar. The critical section problem is generalized version of the producer-consumer problem. Differences are that the producer-consumer problem splits the processes into distinct two classes based on the type of operations they perform. Also, the
2. Analyze the difference between the producers-consumers problem and the producer-consumer problem.
    - In the producer-consumer problem, there is 1 producer and 1 consumer. The producers-consumers problem does not have this restriction so there are an arbitrary number of producer and consumer processes. In the producer-consumer problem, each class of process did not have to worry about competing with processes of the same class to write/read from the shared memory region. In the original 1-1 problem, a producer didn't have to worry about another producer overwriting the region of memory it just wrote to. This means we must add shared variables between the producers and consumers respectively. The producers must share some variable(s) to indicate where they are placing items into the memory and possibly what items they placed. The consumers must share some variable(s) to indicate where they read from the memory.
## Design
1. Design shared variables and non-shared variables. Also pay attention to the difference
between constant and variables. In the textbook, BUFFER SIZE is a constant. But in this project,
it should be designed as a variable.
    - There are three types of shared variables: shared by producer & consumers, shared only by producers, and shared only by consumers.
        - Shared by Both:
            1. buffer of ints: A shared region of memory that the producers place integers into and the consumers read integers from. It's size is determined at runtime, but it does not change after its instantiation. Acts as a circular array of ints.
            2. size of buffer: Both need to know the size of buffer in order to properly increment their index that points to the next region of memory to read/write from.
            3. upper limit: number that the producer will produce integers up to. This number is used by both in order to know when to stop writing/reading from the buffer.
            4. semaphore for producer: Indicates if the producer threads can place items into the buffer. It'svalue is the current number of "free" places in the buffer. It should be initalized to the buffer size. The consumer should increment the semaphore when it consumes, and the producer should decrement it when it adds an item.
            5. semaphore for consumer: Indicates the number of items to be consumed. It's value is initialized to 0 and the producer increments it when it places an item into the buffer. The consumer then decrements it when it reads from the buffer.
        - Shared by Producers:
            1. mutex: This mutex will ensure no two producers will be in their critical section at the same time. This ensures no items in the buffer will be overwritten, written twice, or some other erroneous behavior.
            2. next item to be produced: Integer value that reprents the next integer to be placed into the buffer. All producers need to know this value and it cannot be operated on at the same time.
            3. index: index into the buffer that indicates the next spot to place an item into the buffer. This value should also not be operated on at the same time.
        - Shared by Consumers:
            1. mutex: This mutex will ensure no two consumers will be in their critical section at the same time. This ensures no items are read twice, skipped, etc.
            2. index: index into the buffer that indicates next spot to read from. This value cannot be operated on at the same time.
2. Design critical section of producer: how many critical sections should the producer function
have, why?
    - The producer should have two critical sections. The first one should enclose the entire operation of placing an item into the buffer to maintain the correct next produced and index value. The second should come right after the first and ensure the buffer is not full. If the buffer is full, the producer thread should wait util a consumer thread reads an item. The first section can be managed by a mutex (binary semaphore), and the second can be managed by a semaphore whose value represents the free slots in the buffer. In pseudo code, it should look like this:
    ```
    produce(params) {
        while (not at upper limit) {
            aquire lock
            wait on producer semaphore
            // Produce an item
            post to consumer semaphore
            release lock
        }
    }
    ```
3. Design critical section of consumer: how many critical section should the consumer function
have, why?
    - The consumer should also have two critical sections. The first one should enclose the entire operation of reading an item into the buffer to ensure no items are skipped or read 2+ times. The second should come right after the first and should ensure the buffer is not empty. This is similar to the producer one, but the opposite scenario. The first section can be managed by a mutex (binary semaphore), and the second can be managed by a semaphore whsoe value represents the number of unread items in the buffer. In pseudo code, it should look like this:
    ```
    consume(params) {
        while (not read upper limit) {
            aquire lock
            wait on consuemr semaphore
            // Read an item
            // print item
            post to producer semaphore
            release lock
        }
    }
    ```
4. Design semaphores. For example, a mutex semaphore is needed for some crtical section.
    - The producers should share a mutex and the consumers should share a mutex. These two are distinct semaphores not used by the other class of thread. These simply ensure no producers are placing items at the same time and no consumers are reading items at the same time. The next two semaphores are shared and disallow buffer overflow/underflow. These are not binary semaphores. When the producer semaphore is 0, the producers must wait, when the consumer semaphore is 0, the consumer must wait. The producer posts to the consumer semaphore to let the consumer know an item is ready. The consumer posts to the producer semaphore to let the producer know an item was freed up.
5. Design data structure that needs to be passed into producer and consumer.
    - Shared by both:
    ```
    structure shared {
        int buffer[];
        int buffer size;
        int upper limit;
        semaphore can_produce;
        semaphore can_consume;
    }
    ```
    - Shared by producers:
    ```
    structure shared_producers {
        shared *shared; // global instance of shared structure
        int next_produced;
        int in; // Shared index
    }
    ```
    - Shared by consumers:
    ```
    structure shared_consumers {
        shared *shared; // global instance of shared structure
        int out; // Shared index
        int id; // Unique to each consumer thread, so not shared
    }
    ```

## Comparison to the Readers-Writers Problem
### Analysis
- The readers-writers problem shares many similarities and differences with the producers-consumers problem. Similarly to the producers-consumers problem, we divide the kinds of processes sharing a critical section(s) into two classes: reader threads and writer threads. A difference, however, is that these two classes are not distinct. A writer has all the abilities of a reader, but it can also write to the memory, shared variable, etc. This means a reader thread is a subclass of a writer thread. In the producers-consumers problem, there are two distinct classes that exclusively compete with members of their own class for entry into their critical section. In the readers-writers problem, both classes are competing with each other for entry into their critical section. 
- Another major difference between the two problems is the critical sections of their classes of processes. In the producers-consumers problem, there are two critical sections for the producer and consumer. The layouts of these are very similar. The first ensures no two of the same class are modifying the buffer at the same time; the second ensures no overflow or underflow occurs for the producer and consumer respectively. In the readers-writers problem, the critical section(s) layout for the classes are very different. This enforces the idea that the reader is a subclass of a writer, so it must have the same critical section as the writer, but with extra steps. In the simplest form the implementations are:
    - Writer: The writer is very simple. It uses one mutex (binary semaphore) which allows it to enter its critical section and write to something. It then must release this once it is done writing.
    - Reader: The reader is slightly more complex so it is best to describe it in pseudo code. In this code, the readers all share the *readers_mutex* and the writers and readers share *read_write_mutex*. readcount indicates the number of current readers. If it is 1 in first entry section, we must wait on the *read_write_mutex* in case any writers are writing. If it is 0 in the entry section after reading, we let the writer know it can write by releasing the *read_write_mutex*.
    ```
    while (reading)
        wait(readers_mutex)
        readcount++
        if (readcount = 1)
            wait(read_write_mutex)
        signal(readers_mutex)
        // Read
        wait(readers_mutex)
        readcount--
        if (readcount = 0)
            signal(read_write_mutex)
        signal(readers_mutex)
    ```
- The above implementation is the simplest version, but it suffers from starvation. If there is a constant inflow of readers, such that *readcount* never drops to 0, the writer will never aquire the *read_write_mutex* from the readers. In most cases this is likely unacceptable.
### Design Without Writer Starvation
