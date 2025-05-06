#include <vfs.h>
#include <tmpfs.h>//i put this here instead of inside vfs.h because it was causing definition or redefinition? issues

void init_vfs() {
    vfs_t* tmpfs = init_tmpfs();

    kprintf("\n\n");

    
    vnode_t* root = tmpfs->vnode_covered;
    
    tmpfs_list_files(root->vnode_data);


    vnode_t* f = root->vnode_ops->vnode_create(root, "hi.txt", 4096);
    tmpfs_list_files(root->vnode_data);

    // vnode_t* f = root->vnode_ops->vnode_lookup(root, "hi.txt");
    // vnode_t* f = vnode_tmpfs_lookup(root, "test dir 1");
    // tmpfs_directory_t* dir = tmpfs_lookup(root->vnode_data, "test dir 1");
    // kprintf("c%s", dir->header.name);
    // kprintf("hi");
    kprintf("HIHIHI\n%s\n", ((tmpfs_header_t*) f->vnode_data)->name);


    vfs_fd_t* fd = tmpfs_open(root->vnode_data, "hi.txt", 0);
    root->vnode_ops->vnode_wr(fd, "asadf", 5, 0);
    char buffer[256];
    root->vnode_ops->vnode_rd(fd, buffer, 6, 1);
    kprintf("\n");
    kprintf("%s\n", buffer);

    


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