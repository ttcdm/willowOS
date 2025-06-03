#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>
#include <vmm.h>

extern uint64_t* usermode_stack_base;
extern void* user_code;

void init_syscalls(void);

void test_a(void);

void jump_to_user(void);

void syscall_handler(uint64_t num);

void syscall_switcher(uint64_t num);

void syscall0(void);
void syscall1(void);
void syscall2(void);
void syscall3(void);
void syscall4(void);
void syscall5(void);
void syscall6(void);
void syscall7(void);
void syscall8(void);
void syscall9(void);