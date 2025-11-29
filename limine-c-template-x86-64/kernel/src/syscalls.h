#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>
#include <vmm.h>
#include <scheduler.h>
#include <mutex.h>


//we're currently aiming for some semblance of posix compatibility because linux's is more or less just an add on of that
//so it makes my life easier to have to do less and still be able to run the stuff i want

//HERE always remember to update syscalls.asm with each additional syscall

extern void* user_code;

void init_syscalls(void);

void test_a(void);
void test_b(void);

// void jump_to_user(void (*entry)(void));
void jump_to_user(void* entry, void* rsp);
// void jump_to_user();

void swap_to_user_gs(void);
void swap_to_kernel_gs(void);
void swap_to_user_or_kernel_gs(uint64_t cs, bool to_or_from_kernel);


void syscall_handler(uint64_t num);

void syscall_switcher(uint64_t num);

void push_syscall_args(void);
void pop_syscall_args(void);

void syscall_asm(uint64_t num);

struct map_page_bytes_args {
            uint64_t* cr3;
            // uint64_t phys_address;//not used
            uint64_t virt_address;
            uint64_t permissions;
            uint64_t size;
            uint64_t flag;
            int error;//exit code
            void* ret;//used for returning the actual mapped virtual address but it's unused rn
};

struct fmt_args {//for passing in from log into syscall kprintf
    char* fmt;
    va_list args;
};

int syscall0(uint64_t num);
int syscall1(uint64_t num);
int syscall2(uint64_t num);
int syscall3(uint64_t num);
int syscall4(size_t num);
int syscall5(uint64_t num);
int syscall6(uint64_t num);
int syscall7(uint64_t num, struct map_page_bytes_args* mmap_args);//map
int syscall8(uint64_t num, void *pointer, size_t size);//unmap
int syscall9(uint64_t num);
int syscall10(uint64_t num);
int syscall11(uint64_t num, int* pointer, int expected);
int syscall12(uint64_t num, int* pointer);
int syscall13(uint64_t num, size_t size, void **pointer);
int syscall14(uint64_t num, void *pointer);
int syscall15(uint64_t num, struct fmt_args* args_struct);
int syscall16(uint64_t num, thread_context** thread);//get_current_thread_syscall
int syscall17(uint64_t num, char* str, size_t len);//kprintf() wrapper



int syscall_log(char* fmt, ...);
int syscall_test(void);
int syscall_yield(void);
int get_current_thread_syscall(thread_context** thread);


struct timespec {//apparently this is fine for type replacements
    uint32_t tv_sec;
    int32_t tv_nsec;
};


//mlibc syscalls
int sys_futex_wait(int *pointer, int expected, const struct timespec *time);
int sys_futex_wake(int *pointer);

int sys_anon_allocate(size_t size, void **pointer);
int sys_anon_free(void *pointer, size_t size);

int sys_vm_map(void *hint, size_t size, int prot, int flags, int fd, int64_t offset, void **window);//we're using 8 byte signed int as a replacement for off_t. hopefully it's fine
int sys_vm_unmap(void *pointer, size_t size);

//we're replacing time_t with uint64_t
int sys_clock_get(int clock, uint64_t *secs, long *nanos);

int sys_tcb_set(void *pointer);



//temp linux abi stuff for mlibc

#define __MLIBC_LINUX_OPTION 1//HERE

#define PROT_NONE  0x00
#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define PROT_EXEC  0x04

#define MAP_FAILED ((void *)(-1))
#define MAP_FILE    0x00
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x10
#define MAP_ANON      0x20
#define MAP_ANONYMOUS 0x20

#if __MLIBC_LINUX_OPTION

#define MAP_GROWSDOWN 0x100
#define MAP_DENYWRITE 0x800
#define MAP_EXECUTABLE 0x1000
#define MAP_LOCKED    0x2000
#define MAP_NORESERVE 0x4000
#define MAP_POPULATE  0x8000
#define MAP_NONBLOCK  0x10000
#define MAP_STACK     0x20000
#define MAP_HUGETLB   0x40000
#define MAP_SYNC      0x80000
#define MAP_FIXED_NOREPLACE 0x100000

#endif /* __MLIBC_LINUX_OPTION */

#define MS_ASYNC 0x01
#define MS_INVALIDATE 0x02
#define MS_SYNC 0x04

#define MCL_CURRENT 0x01
#define MCL_FUTURE 0x02

#define POSIX_MADV_NORMAL 0
#define POSIX_MADV_RANDOM 1
#define POSIX_MADV_SEQUENTIAL 2
#define POSIX_MADV_WILLNEED 3
#define POSIX_MADV_DONTNEED 4

// #if __MLIBC_LINUX_OPTION

#define MADV_NORMAL 0
#define MADV_RANDOM 1
#define MADV_SEQUENTIAL 2
#define MADV_WILLNEED 3
#define MADV_DONTNEED 4
#define MADV_FREE 8
#define MADV_REMOVE 9
#define MADV_DONTFORK 10
#define MADV_DOFORK 11
#define MADV_MERGEABLE 12
#define MADV_UNMERGEABLE 13
#define MADV_HUGEPAGE 14
#define MADV_NOHUGEPAGE 15
#define MADV_DONTDUMP 16
#define MADV_DODUMP 17
#define MADV_WIPEONFORK 18
#define MADV_KEEPONFORK 19
#define MADV_COLD 20
#define MADV_PAGEOUT 21
#define MADV_HWPOISON 100
#define MADV_SOFT_OFFLINE 101

#define MREMAP_MAYMOVE 1
#define MREMAP_FIXED 2

#define MFD_CLOEXEC 1U
#define MFD_ALLOW_SEALING 2U
#define MFD_HUGETLB 4U