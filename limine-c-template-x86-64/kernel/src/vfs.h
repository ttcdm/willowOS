#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <vmm.h>
#include <tsc.h>

#define TMPFS_MAX_FILES 1024

typedef struct tmpfs_header {
    char permissions[3];//rwx
    uint16_t user_id;
    uint16_t group_id;
    char name[128];//we cap the length of the name at 128 chars ig
    uint8_t type;//0 for file, 1 for directory
    uint64_t timestamps[3];//ctime(inode change time), mtime, atime;//use tsc to get/update timestamps
} tmpfs_header_t;

typedef struct tmpfs_file {
    tmpfs_header_t header;
    uint64_t size;
} tmpfs_file_t;

typedef struct tmpfs_directory {
    tmpfs_header_t header;
    void* files[TMPFS_MAX_FILES];//probably could use this to store directories as well
    uint64_t probably_next_free_entry_index;//uint32_t or even uint16_t would probably suffice but oh well
} tmpfs_directory_t;

void init_vfs();
void init_tmpfs();

void* tmpfs_create_file(tmpfs_directory_t* dir, char* name, uint64_t size);//pointer to created object
void* tmpfs_create_directory(tmpfs_directory_t* dir, char* name);
void tmpfs_delete_file(tmpfs_directory_t* dir, char* name);
void tmpfs_delete_directory(tmpfs_directory_t* dir, char* name);
void tmpfs_delete_directory_no_orphan(tmpfs_directory_t* dir, char* name);

void tmpfs_list_files(tmpfs_directory_t* dir);//remember that it's files and not file. also maybe make this list directoreis as well?
void tmpfs_write_to_file(tmpfs_file_t* file, char* msg, uint64_t size);//these should probably support fopen fseek ftell and such
void tmpfs_read_from_file(tmpfs_file_t* file, char* msg, uint64_t size);


void fopen();
void fread();
void fwrite();
void fclose();
void fstat();
void fseek();
void ftell();

//maybe add EOF??