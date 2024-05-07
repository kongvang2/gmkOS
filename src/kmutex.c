/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2022
 *
 * Kernel Mutexes
 */

#include <spede/string.h>

#include "kernel.h"
#include "kmutex.h"
#include "queue.h"
#include "scheduler.h"

// Table of all mutexes
mutex_t mutexes[MUTEX_MAX];

// Mutex ids to be allocated
queue_t mutex_queue;

/**
 * Initializes kernel mutex data structures
 * @return -1 on error, 0 on success
 */
int kmutexes_init() {
    kernel_log_info("Initializing kernel mutexes");

    // Initialize the mutex table
    for (int i=0; i<MUTEX_MAX; i++){
        mutexes[i].allocated = 0; //set to 1 to indicate mutex allocated
        mutexes[i].locks = 0; //initial mutex lock value should always be 0
        mutexes[i].owner = NULL; //mutex owner should be a NULL ptr
        kernel_log_info("Initializing wait queues in mutex");
        queue_init(&mutexes[i].wait_queue);
    }
    // Initialize the mutex queue
    queue_init(&mutex_queue);
    // Fill the mutex queue
    for (int i=0; i<MUTEX_MAX; i++){
        if (queue_in(&mutex_queue, i) != 0 ){
            kernel_log_info("Fill Mutex Queue error");
            return -1;
        }
    }
    return 0;
}

/**
 * Allocates a mutex
 * @return -1 on error, otherwise the mutex id that was allocated
 */
int kmutex_init(void) {
    // Obtain a mutex id from the mutex queue
    int id;
    queue_out(&mutex_queue, &id);
    // Ensure that the id is within the valid range
    if (id > MUTEX_MAX){
        kernel_log_error("kmutex_init: mutex id %d invalid range", id);
        return -1;
    }
    // Pointer to the mutex table entry
    mutex_t *mutex_entry_ptr = &mutexes[id];
    // Initialize the mutex data structure (mutex_t + all members)
    if (mutex_entry_ptr){
        mutex_entry_ptr->allocated = 1;
        //mutex_t members should be initialized already from kmutexes_int()
        return id;
    }
    // return the mutex id
    else{
        return -1;
    }
}

/**
 * Frees the specified mutex
 * @param id - the mutex id
 * @return 0 on success, -1 on error
 */
int kmutex_destroy(int id) {
    // look up the mutex in the mutex table
    mutex_t *mutex_ptr = &mutexes[id];
    if (mutex_ptr){
        if (mutex_ptr->locks > 0){
            kernel_log_error("Cannot destroy locked mutex ");
            return -1; //error
        }

        // Add the id back into the mutex queue to be re-used later
        if (queue_in(&mutex_queue, id) != 0){
            kernel_log_error("error adding id back into mutex queue ");
            return -1;
        }
        // Clear the memory for the data structure
        memset(mutex_ptr, 0, sizeof(mutex_t));
        kernel_log_info("Mutex cleared/destroyed");
        return 0;
    }
    kernel_log_error("Failed to destroy Mutex");
    return -1;
}

/**
 * Locks the specified mutex
 * @param id - the mutex id
 * @return -1 on error, otherwise the current lock count
 */
int kmutex_lock(int id) {
    // look up the mutex in the mutex table
    mutex_t *mutex_ptr = &mutexes[id];
    proc_t *proc = active_proc;
    if (!proc){
        kernel_panic("Invalid process - called from kmutex_lock()");
        return -1;
    }
    if (mutex_ptr){
        // If the mutex is already locked
        //   1. Set the active process state to WAITING
        //   2. Add the process to the mutex wait queue (so it can take
        //      the mutex when it is unlocked)
        //   3. Remove the process from the scheduler, allow another
        //      process to be scheduled
        if (mutex_ptr->locks > 0){
            scheduler_remove(proc);

            proc->state = WAITING;
            proc->scheduler_queue = &mutex_ptr->wait_queue;
            queue_in(&mutex_ptr->wait_queue, proc->pid);
            
        }
        // If the mutex is not locked
        //   1. set the mutex owner to the active process
        if (mutex_ptr->locks == 0){
            mutex_ptr->owner = proc;
        }
        // Increment the lock count
        (mutex_ptr->locks)++;
        // Return the mutex lock count
        return mutex_ptr->locks;
    }
    return -1;
}

/**
 * Unlocks the specified mutex
 * @param id - the mutex id
 * @return -1 on error, otherwise the current lock count
 */
int kmutex_unlock(int id) {
    proc_t *proc;
    int temp_id;
    // look up the mutex in the mutex table
    mutex_t *mutex_ptr = &mutexes[id];
    // If the mutex is not locked, there is nothing to do
    if (mutex_ptr->locks == 0){
        kernel_log_info("mutex is not locked, nothing to do");
        return mutex_ptr->locks;
    }
    // Decrement the lock count
    (mutex_ptr->locks)--;
    // If there are no more locks held:
    //    1. clear the owner of the mutex
    if (mutex_ptr->locks == 0){
        mutex_ptr->owner = NULL;
        return mutex_ptr->locks;
    }
    // If there are still locks held:
    //    1. Obtain a process from the mutex wait queue
    //    2. Add the process back to the scheduler
    //    3. set the owner of the of the mutex to the process
    else {
        // 1.
        queue_out(&mutex_ptr->wait_queue, &temp_id);
        proc = pid_to_proc(temp_id);
        if (proc){
            // 2.
            scheduler_remove(proc);
            scheduler_add(proc);
            // 3.
            mutex_ptr->owner = proc;
        }
        return mutex_ptr->locks;
    }
    // return the mutex lock count
    //if we get here then error occurred
    kernel_log_error("mutex unlock error");
    return -1;
}
