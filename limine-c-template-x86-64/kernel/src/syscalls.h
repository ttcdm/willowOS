#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>
#include <vmm.h>
#include <scheduler.h>
#include <mutex.h>

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

int syscall0(uint64_t num);
int syscall1(uint64_t num);
int syscall2(uint64_t num);
int syscall3(uint64_t num);
int syscall4(size_t num, char* str);
int syscall5(uint64_t num);
int syscall6(uint64_t num);
int syscall7(uint64_t num);
int syscall8(uint64_t num);
int syscall9(uint64_t num);
int syscall10(uint64_t num);
int syscall11(uint64_t num, int* pointer, int expected);
int syscall12(uint64_t num, int* pointer);
int syscall13(uint64_t num);
int syscall14(uint64_t num);
int syscall15(uint64_t num);



int syscall_log(char* str);
int syscall_test(void);
int syscall_yield(void);