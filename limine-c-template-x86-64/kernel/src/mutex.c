#include <mutex.h>

/*
basically, we only need mutexes for multithreading i think
so what we do is if a thread tries to acquire a mutex on an object that's already locked,
it'll just spin until the mutex is released

*/


bool acquire_mutex(mutex_t* mutex) {

    //(object, old val, new val)
    while(!__sync_bool_compare_and_swap(&mutex->locked, 0, 1)) {
        asm volatile ("pause");
    }

    
    //we return whether or not the operation was successful
}

bool release_mutex(mutex_t* mutex) {

    __sync_bool_compare_and_swap(&mutex->locked, 1, 0);
    //we return whether or not the operation was successful
}