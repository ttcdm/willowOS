#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <vmm.h>
#include <tsc.h>
#include <vfs.h>
#include <mutex.h>

// #define TMPFS_MAX_FILES 1024

typedef struct tmpfs_header {
    char permissions[3];//rwx
    uint16_t user_id;
    uint16_t group_id;
    char name[128];//we cap the length of the name at 128 chars ig
    char path[1024];
    char fs_type[32];
    uint8_t type;//0 for file, 1 for directory
    uint64_t timestamps[3];//ctime(inode change time), mtime, atime;//use tsc to get/update timestamps
    mutex_t* mutex;
} tmpfs_header_t;

typedef struct tmpfs_file {
    tmpfs_header_t header;
    uint64_t size;
    void* data;
    uint64_t open_count;
} tmpfs_file_t;

typedef struct tmpfs_directory {
    tmpfs_header_t header;
    // void* files[TMPFS_MAX_FILES];//probably could use this to store directories as well
    void** files;
    uint64_t max_files;
    uint64_t num_files;
    // void* first_file;//implement linked list
    // uint64_t probably_next_free_entry_index;//uint32_t or even uint16_t would probably suffice but oh well
} tmpfs_directory_t;

//i guess always just have file as a param so we can check what kind of fs it is
typedef struct tmpfs_fd {
    void* data;
    uint64_t size;
    uint64_t position;
    uint8_t mode;//0 for read, 1 for write from beginning, 2 for append
    tmpfs_file_t* file;//pointer back to the original file
} tmpfs_fd_t;

vfs_t* init_tmpfs();


//remmeber to add wrappers for vfs so we can just use vnodes instead of fs specific variables
void* tmpfs_create_file(tmpfs_directory_t* dir, char* name, uint64_t size);//pointer to created object
void* tmpfs_create_directory(tmpfs_directory_t* dir, char* name);
void tmpfs_delete_file(tmpfs_directory_t* dir, char* name);
void tmpfs_delete_directory(tmpfs_directory_t* dir, char* name);
void tmpfs_delete_directory_no_orphan(tmpfs_directory_t* dir, char* name);

void tmpfs_list_files(tmpfs_directory_t* dir);//remember that it's files and not file. also maybe make this list directoreis as well?
void tmpfs_write_to_file(tmpfs_fd_t* file, void* data, uint64_t size, uint64_t offset);//these should probably support fopen fseek ftell and such
size_t tmpfs_read_from_file(tmpfs_fd_t* file, void* data, uint64_t size, uint64_t offset);

void tmpfs_not_available(void);

tmpfs_fd_t* tmpfs_open(tmpfs_directory_t* dir, char* name, uint8_t mode);
void tmpfs_close(tmpfs_fd_t* fd);
void* tmpfs_lookup(tmpfs_directory_t* dir, char* name);

vnode_t* vnode_tmpfs_lookup(vnode_t* vnode, char* name);
vnode_t* vnode_tmpfs_create_file(vnode_t* vnode, char* name, uint64_t size);
void vnode_tmpfs_delete_file(vnode_t* vnode, char* name);
void* vnode_tmpfs_create_directory(vnode_t* vnode, char* name);
void vnode_tmpfs_delete_directory(vnode_t* vnode, char* name);
void vnode_tmpfs_delete_directory_no_orphan(vnode_t* vnode, char* name);
void vnode_tmpfs_write_to_file(vfs_fd_t* file, void* data, uint64_t size, uint64_t offset);//these should probably support fopen fseek ftell and such
size_t vnode_tmpfs_read_from_file(vfs_fd_t* file, void* data, uint64_t size, uint64_t offset);

vnode_t* tmpfs_link_vnode(void* file_object, enum vtype type);

void tmpfs_fd_write_to_file(int fd, void* data, uint64_t size, uint64_t offset);
size_t tmpfs_fd_read_from_file(int fd, void* data, uint64_t size, uint64_t offset);