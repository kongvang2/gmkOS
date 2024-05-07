/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2022
 *
 * Kernel Semaphores
 */

#include <spede/string.h>

#include "kernel.h"
#include "ksem.h"
#include "queue.h"
#include "scheduler.h"

// Table of all semephores
sem_t semaphores[SEM_MAX];

// semaphore ids to be allocated
queue_t sem_queue;

/**
 * Initializes kernel semaphore data structures
 * @return -1 on error, 0 on success
 */
int ksemaphores_init() {
    kernel_log_info("Initializing kernel semaphores");

    // Initialize the semaphore table
    for(int i=0; i < SEM_MAX; i++){
        semaphores[i].allocated=0;
        semaphores[i].count =0;
        //initializes the queues?
        kernel_log_info("Initializing the wait_queues in sem_t");
        queue_init(&semaphores[i].wait_queue);
    }
    // Initialize the semaphore queue
    queue_init(&sem_queue);
    // Fill the semaphore queue
    for(int i=0; i < SEM_MAX; i++){
        if(queue_in(&sem_queue, i) !=0){
            kernel_log_info("WARNING!!! At initialization unable to fill semaphore queue");
            return -1;
        }
    }
    return 0;
}

/**
 * Allocates a semaphore
 * @param value - initial semaphore value
 * @return -1 on error, otherwise the semaphore id that was allocated
 */
int ksem_init(int value) {
    // Obtain a semaphore id from the semaphore queue
    int id;
    queue_out(&sem_queue, &id);

    // Ensure that the id is within the valid range
    if (id > SEM_MAX){
        kernel_log_error("ksem_init: semaphore id %d invalid range", id);
        return -1;
    }
    // Initialize the semaphore data structure
    // sempohare table + all members (wait queue, allocated, count)

    sem_t *sem_entry_ptr = &semaphores[id];

    if (sem_entry_ptr){
        sem_entry_ptr->allocated = 1;
        //count should be set to 0 in  initialization
        //wait_queue should already be initialized
        return id;
    }
    return -1;
}

/**
 * Frees the specified semaphore
 * @param id - the semaphore id
 * @return 0 on success, -1 on error
 */
int ksem_destroy(int id) {
    // look up the sempaphore in the semaphore table
    sem_t *sem_ptr = &semaphores[id];

    if(sem_ptr){
        // If the semaphore is locked, prevent it from being destroyed
        if(sem_ptr->count > 0){
            kernel_log_error("cannot destroy a locked semaphor");
            return -1;
        }

        // Add the id back into the semaphore queue to be re-used later
        if(queue_in(&sem_queue, id) != 0){
            kernel_log_error("error adding id back into mutex queue");
            return -1;
        }
        // Clear the memory for the data structure
        memset(sem_ptr, 0, sizeof(sem_t));
        kernel_log_info("semaphore cleared/destroyed");
        return 0;
    }
    kernel_log_error("Failed to destory semaphore");
    return -1;
}

/**
 * Waits on the specified semaphore if it is held
 * @param id - the semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int ksem_wait(int id) {
    // look up the sempaphore in the semaphore table
    sem_t *sem_ptr = &semaphores[id];
    proc_t *proc = active_proc;

    if (!proc){
        kernel_panic("invalid process - called from ksem_wait()");
        return -1;
    }

    // If the semaphore count is 0, then the process must wait
        // Set the state to WAITING
        // add to the semaphore's wait queue
        // remove from the scheduler
    if (sem_ptr->count == 0){
        proc->state = WAITING;
        proc->scheduler_queue = &sem_ptr->wait_queue;
        queue_in(&sem_ptr->wait_queue, proc->pid);
    }

    // If the semaphore count is > 0
        // Decrement the count
    if (sem_ptr->count > 0){
        sem_ptr->count --;
    }

    // Return the current semaphore count

    return sem_ptr->count;
}

/**
 * Posts the specified semaphore
 * @param id - the semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int ksem_post(int id) {

    // look up the semaphore in the semaphore table
    sem_t *sem_ptr = &semaphores[id];
    proc_t *proc;
    // incrememnt the semaphore count
    sem_ptr->count ++;

    // check if any processes are waiting on the semaphore (semaphore wait queue)
        // if so, queue out and add to the scheduler
        // decrement the semaphore count
    if (queue_out(&sem_ptr->wait_queue, &id)==0){
        proc = pid_to_proc(id);
        scheduler_add(proc);
        sem_ptr->count --;
    }
    // return current semaphore count

    return sem_ptr->count;
}
