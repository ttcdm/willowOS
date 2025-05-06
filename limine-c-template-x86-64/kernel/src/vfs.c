#include <vfs.h>

void init_vfs() {
    init_tmpfs();

    // vfs_t* tmpfs = (vfs_t*) kmalloc_byte(sizeof(vfs_t));//place inside init_tmpfs() and have it return this
    // vfs_ops_t* tmpfs_ops = (vfs_ops_t*) kmalloc_byte(sizeof(vfs_ops_t));
    // vnode_ops_t* tmpfs_vnode_ops = (vnode_ops_t*) kmalloc_byte(sizeof(vnode_ops_t));
    // tmpfs_vnode_ops->vnode_mkdir = tmpfs_create_directory;
    // tmpfs_vnode_ops->vnode_create = tmpfs_create_file;
    // tmpfs_vnode_ops->vnode_rmdir = tmpfs_delete_directory;
    // tmpfs_vnode_ops->vnode_rmdir_no_orphan = tmpfs_delete_directory_no_orphan;
    // tmpfs_vnode_ops->vnode_remove = tmpfs_delete_file;
    // tmpfs->vfs_ops = tmpfs_ops;
    // vnode_t* tmpfs_root_vnode = (vnode_t*) kmalloc_byte(sizeof(vnode_t));
    // tmpfs_root_vnode->vnode_data;


}





// IMPLEMENT THESE AS SYSCALLS
// void fopen() {}
// void fread() {}
// void fwrite() {}
// void fclose() {}
// void fstat() {}
// void fseek() {}
// void ftell() {}