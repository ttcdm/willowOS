#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <vmm.h>
#include <tsc.h>
#include <tmpfs.h>


typedef struct vnode vnode_t;
typedef struct vnode_ops vnode_ops_t;
typedef struct vfs vfs_t;
typedef struct vfs_ops vfs_ops_t;

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

};

struct vnode_ops {
    int (*vnode_rd)();
    int (*vnode_wr)();
    int (*vnode_ioctl)();
    int (*vnode_lookup)();
    int (*vnode_create)();
    int (*vnode_remove)();
    int (*vnode_mkdir)();
    int (*vnode_rmdir)();
    int (*vnode_rmdir_no_orphan)();
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



void init_vfs();


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