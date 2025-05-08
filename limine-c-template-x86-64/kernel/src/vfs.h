#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <vmm.h>
#include <tsc.h>
// #include <tmpfs.h>

//taken from https://www.cs.fsu.edu/~awang/courses/cop5611_s2024/vnode.pdf

typedef struct vnode vnode_t;
typedef struct vnode_ops vnode_ops_t;
typedef struct vfs vfs_t;
typedef struct vfs_ops vfs_ops_t;
typedef struct vfs_fd vfs_fd_t;

enum vtype { VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VBAD };

struct vnode {
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
    vfs_t* mounted_vfs;

};

struct vnode_ops {//HERE remember to always match the return types to prevent errors
    size_t (*vnode_rd)();//not sure if i'm supposed to populate the args
    void (*vnode_wr)();
    int (*vnode_ioctl)();
    vnode_t* (*vnode_lookup)();
    vnode_t* (*vnode_create)();
    void (*vnode_remove)();
    void* (*vnode_mkdir)();
    void (*vnode_rmdir)();
    void (*vnode_rmdir_no_orphan)();
};

struct vfs {
    vfs_t* next_vfs;
    vfs_ops_t* vfs_ops;
    vnode_t* vnode_covered;//figure out what this means/what it's for
    int vfs_flag;//add other attributes/data ig
    int vfs_bsize;
    void* vfs_data;
};

struct vfs_ops {
    int (*vfs_mount)();
    int (*vfs_unmount)();
    int (*vfs_root)();
    int (*vfs_statfs)();
    int (*vfs_sync)();
    int (*vfs_fid)();
    int (*vfs_vget)();
};

struct vfs_fd {
    void* data;
    uint64_t size;
    uint64_t position;
    uint8_t mode;//0 for read, 1 for write from beginning, 2 for append
};


void init_vfs();

void vnode_mount_vfs(vnode_t* parent_vnode, vfs_t* vfs);

void vnode_unmount_vfs(vnode_t* vnode, char* name);

//IMPLEMENT THESE AS SYSCALLS
// void fopen();
// void fread();
// void fwrite();
// void fclose();
// void fstat();
// void fseek();
// void ftell();

//THIS IS FOR USERSPACE TO HANDLE
//maybe add EOF??