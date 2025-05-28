#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>

extern uint64_t* top;
extern void* user_code;

void init_syscalls(void);

void test_a(void);

void jump_to_user(void);

void syscall1(void);