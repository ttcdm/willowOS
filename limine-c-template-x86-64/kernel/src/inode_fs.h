#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <vmm.h>
#include <tsc.h>



typedef struct inode_header {
    char permissions[3];//rwx
    uint16_t user_id;
    uint16_t group_id;
    char name[128];//we cap the length of the name at 128 chars ig
    uint8_t type;//0 for file, 1 for directory
    uint64_t size;//size of file
    uint64_t timestamps[3];//ctime(inode change time), mtime, atime;//use tsc to get/update timestamps
    uint16_t io_block_size;
    uint64_t allocated_blocks;//not sure what type to use but idk it shouldn't really matter
    
} inode_header_t;

typedef struct inode {//maybe void is better than uint64_t in this case
    inode_header_t header;
    void* direct_pointers[12];
    void** indirect_pointer0;
    void*** indirect_pointer1;
    void**** indirect_pointer2;
    void* next_free_entry;//not sure if i should just put the raw address here instead
} inode_t;

void init_inode_fs(void);
