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