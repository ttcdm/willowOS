#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>
#include <vmm.h>
#include <scheduler.h>

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

int syscall0(void);
int syscall1(void);
int syscall2(void);
int syscall3(void);
int syscall4(size_t num, char* str);
int syscall5(uint64_t num);
int syscall6(uint64_t num);
int syscall7(void);
int syscall8(void);
int syscall9(void);


int syscall_log(char* str);
int syscall_test(void);
int syscall_yield(void);