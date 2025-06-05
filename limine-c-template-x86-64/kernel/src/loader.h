#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>
#include <vmm.h>
#include <tmpfs.h>
#include <vfs.h>

void init_loader(vfs_fd_t* file);