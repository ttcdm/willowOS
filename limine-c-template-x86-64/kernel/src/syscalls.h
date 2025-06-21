#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>
#include <vmm.h>

extern void* user_code;

void init_syscalls(void);

void test_a(void);

// void jump_to_user(void (*entry)(void));
void jump_to_user(void* entry, void* rsp);
// void jump_to_user();


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
int syscall6(void);
int syscall7(void);
int syscall8(void);
int syscall9(void);


int syscall_log(char* str);
int syscall_test(void);