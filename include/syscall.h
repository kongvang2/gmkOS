/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2022
 *
 * System call APIs
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "syscall_common.h"

/**
 * Gets the current system time (in seconds)
 * @return system time in seconds
 */
int sys_get_time(void);

/**
 * Gets the operating system name
 * @param name - pointer to a character buffer where the name will be copied
 * @return 0 on success, -1 or other non-zero value on error
 */
int sys_get_name(char *name);

/**
 * Gets the current process' id
 * @return process id
 */
int proc_get_pid(void);

/**
 * Gets the current process' name
 * @param name - pointer to a character buffer where the name will be copied
 * @return 0 on success, -1 or other non-zero value on error
 */
int proc_get_name(char *name);

/**
 * Puts the current process to sleep for the specified number of seconds
 * @param seconds - number of seconds the process should sleep
 */
void proc_sleep(int seconds);

/**
 * Exits the current process
 * @param exitcode An exit code to return to the parent process
 */
void proc_exit(int exitcode);

/**
 * Writes up to n bytes to the process' specified IO buffer
 * @param io - the IO buffer to write to
 * @param buf - the buffer to copy from
 * @param n - number of bytes to write
 * @return -1 on error or value indicating number of bytes copied
 */
int io_write(int io, char *buf, int n);

/**
 * Reads up to n bytes from the process' specified IO buffer
 * @param io - the IO buffer to read from
 * @param buf - the buffer to copy to
 * @param n - number of bytes to read
 * @return -1 on error or value indicating number of bytes copied
 */
int io_read(int io, char *buf, int n);

/**
 * Flushes (clears) the specified IO buffer
 * @param io - the IO buffer to flush
 * @return -1 on error or 0 on success
 */
int io_flush(int io);

/**
 * Allocates a mutex from the kernel
 * @return -1 on error, all other values indicate the mutex id
 */
int mutex_init(void);

/**
 * Detroys a mutex
 * @return -1 on error, 0 on sucecss
 */
int mutex_destroy(int mutex);

/**
 * Locks the mutex
 * @param mutex - mutex id
 * @return -1 on error, 0 on sucecss
 * @note If the mutex is already locked, process will block/wait.
 */
int mutex_lock(int mutex);

/**
 * Unlocks the mutex
 * @param mutex - mutex id
 * @return -1 on error, 0 on sucecss
 */
int mutex_unlock(int mutex);

/**
 * Allocates a semaphore from the kernel
 * @param value - initial semaphore value
 * @return -1 on error, all other values indicate the semaphore id
 */
int sem_init(int value);

/**
 * Destroys a semaphore
 * @param sem - semaphore id
 * @return -1 on error, 0 on success
 */
int sem_destroy(int sem);

/**
 * Waits on a semaphore
 * @param sem - semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int sem_wait(int sem);

/**
 * Posts a semaphore
 * @param sem - semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int sem_post(int sem);

#endif
