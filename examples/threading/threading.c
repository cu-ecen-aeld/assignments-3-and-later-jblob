#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

//struct thread_data thd;

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // sleep @param wait_to_obtain_ms number of milliseconds
    usleep(1000*thread_func_args->wait_to_obtain_ms);
    // obtain the mutex in @param mutex
    if (pthread_mutex_lock(thread_func_args->mutex_lock) != 0)
    {
	    thread_func_args->thread_complete_success = false;
	    return thread_param;
    }
    // hold the mutex for @param wait_to_release_ms milliseconds
    usleep(1000*thread_func_args->wait_to_release_ms);
    // release mutex
    pthread_mutex_unlock(thread_func_args->mutex_lock);
    thread_func_args->thread_complete_success = true;
    
    
    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    int retval;
    struct thread_data *thd = (struct thread_data *)malloc(sizeof(struct thread_data));
    if (thd == NULL) 
    	return false;
    thd->wait_to_obtain_ms = wait_to_obtain_ms;
    thd->wait_to_release_ms = wait_to_release_ms;
    thd->mutex_lock = mutex;
    thd->thread_complete_success = false;
    
    retval = pthread_create(thread, NULL, threadfunc, thd);
    if (retval != 0)
    {
    	printf("thread creation failed, retval = %d\n", retval);
        free(thd);
    	return false;
    }
    
    return true; //retval; // retval doesnt pass unit test :/
}

