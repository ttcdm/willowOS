#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <vmm.h>
#include <tsc.h>

void fopen();
void fread();
void fwrite();
void fclose();
void fstat();
void fseek();
void ftell();

//maybe add EOF??