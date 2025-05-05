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
    void* data;
} tmpfs_file_t;

typedef struct tmpfs_directory {
    tmpfs_header_t header;
    void* files[TMPFS_MAX_FILES];//probably could use this to store directories as well
    uint64_t probably_next_free_entry_index;//uint32_t or even uint16_t would probably suffice but oh well
} tmpfs_directory_t;


enum vtype { VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VBAD };

typedef struct vnode {
    uint16_t vnode_flag; /* vnode flags */
    uint16_t vnode_count; /* reference count */
    uint16_t vnode_shlockc; /* # of shared locks */
    uint16_t vnode_exlockc; /* # of exclusive locks */
    vfs_t *vnode_vfsmountedhere; /* covering vfs */
    vnode_ops_t *vnode_ops; /* vnode operations */
    // union {
    // struct socket *v_Socket; /* unix ipc */
    // struct stdata *v_Stream; /* stream */
    // };
    vfs_t *vfs_vfsp;//vfs we're in
    enum vtype vnode_type;
    void* vnode_data; /* private data */

} vnode_t;

typedef struct vnode_ops {
    int (*vnode_open)();
    int (*vnode_close)();
    int (*vnode_rdwr)();
    int (*vnode_ioctl)();
    int (*vnode_select)();
    int (*vnode_getattr)();
    int (*vnode_setattr)();
    int (*vnode_access)();
    int (*vnode_lookup)();
    int (*vnode_create)();
    int (*vnode_remove)();
    int (*vnode_link)();
    int (*vnode_rename)();
    int (*vnode_mkdir)();
    int (*vnode_rmdir)();
    int (*vnode_rmdir_no_orphan)();
    int (*vnode_readdir)();
    int (*vnode_symlink)();
    int (*vnode_readlink)();
    int (*vnode_fsync)();
    int (*vnode_inactive)();
    int (*vnode_bmap)();
    int (*vnode_strategy)();
    int (*vnode_bread)();
    int (*vnode_brelse)();
} vnode_ops_t;

typedef struct vfs {
    vfs_t* next_vfs;
    vfs_ops_t* vfs_ops;
    vnode_t* vnode_covered;//figure out what this means/what it's for
    int vfs_flag;//add other attributes/data ig
    int vfs_bsize;
    void* vfs_data;
} vfs_t;

typedef struct vfs_ops {
    int (*vfs_mount)();
    int (*vfs_unmount)();
    int (*vfs_root)();
    int (*vfs_statfs)();
    int (*vfs_sync)();
    int (*vfs_fid)();
    int (*vfs_vget)();
} vfs_ops_t;



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