#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>



void init_syscalls(void);

void test_a(void);

void jump_to_user(void);

void syscall1(void);