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
    return 0;
}

bool release_mutex(mutex_t* mutex) {

    __sync_bool_compare_and_swap(&mutex->locked, 1, 0);
    //we return whether or not the operation was successful
    return 0;
}

//qwinci said to use these instead of __sync since it's deprecated apparently
// void spin_lock(Spinlock* lock) {
//     while (true) {
//         if (!__atomic_exchange_n(&lock->value, true, __ATOMIC_ACQUIRE) break;
//         while (__atomic_load_n(&lock->value, __ATOMIC_RELAXED))  __builtin_ia32_pause();
//     }
// }

// void spin_unlock(Spinlock* lock) {
//     __atomic_store_n(&lock->value, false, __ATOMIC_RELEASE);
// }