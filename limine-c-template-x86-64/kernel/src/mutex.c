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


futex_queue_t** global_futex_array;
uint64_t global_futex_array_size;



int futex_enqueue(int* pointer, thread_context* thread) {//HERE use hash table instead
    bool irq;
    irq_disable_save(&irq);
    futex_queue_t* queue = NULL;
    for (int i = 0; i < sizeof(global_futex_array) / sizeof(futex_queue_t*); i++) {
        if (((futex_queue_t*) global_futex_array[i])->pointer == pointer) {
            if (i = global_futex_array_size - 1) {
                global_futex_array = (futex_queue_t**) krealloc_byte((uint64_t*) global_futex_array, global_futex_array_size + 128);//i think this should work. hopefully realloc doesn't break anything
            }
            queue = (futex_queue_t*) global_futex_array[i];
            break;
        }
    }

    if (queue == NULL) {
        queue = (futex_queue_t*) kmalloc_byte_interruptable(sizeof(futex_queue_t));
        queue->pointer = pointer;
        queue->size = 0;
        queue->max_size = 64;
        queue->threads = (thread_context**) kmalloc_byte_interruptable(queue->max_size * sizeof(uint64_t*));
        global_futex_array[0] = (futex_queue_t*) queue;
    }

    if (queue->size == queue->max_size) {
        queue->max_size += 64;
        queue->threads = (thread_context**) krealloc_byte((uint64_t*) queue->threads, queue->max_size * sizeof(uint64_t*));//krealloc uses irq save disable so no need to worry about sti'ing when you're not supposed to
    }
    
    queue->threads[queue->size] = thread;//not size+1 because index 0 is 1st element
    queue->size++;


    irq_restore(&irq);
    return 0;
}

void init_futex() {
    global_futex_array = (futex_queue_t**) kmalloc_byte(128 * sizeof(int*));
    global_futex_array_size = 128;
}