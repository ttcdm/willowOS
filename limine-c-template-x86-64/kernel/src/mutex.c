#include <mutex.h>


void init_mutex() {
    mutex_t mutex;
    int a = 0;
    mutex.object = &a;
    acquire_mutex(&mutex);
}


bool acquire_mutex(mutex_t* mutex) {

    // __atomic_set_and_set(mutex->locked);

    
    //we return whether or not the operation was successful
}

bool release_mutex(mutex_t* mutex) {


    //we return whether or not the operation was successful
}