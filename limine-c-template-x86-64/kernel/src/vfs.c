#include <vfs.h>
#include <tmpfs.h>//i put this here instead of inside vfs.h because it was causing definition or redefinition? issues

void init_vfs() {
    vfs_t* tmpfs = init_tmpfs();

    kprintf("\n\n");

    
    vnode_t* root = tmpfs->vnode_covered;
    
    tmpfs_list_files(root->vnode_data);
    root->vnode_ops->vnode_create(root, "hi.txt", 4096);
    tmpfs_list_files(root->vnode_data);
    vnode_t* f = root->vnode_ops->vnode_lookup(root, "hi.txt");
    vfs_fd_t* fd = (vfs_fd_t*) tmpfs_open(root->vnode_data, "hi.txt", 0);
    f->vnode_ops->vnode_wr(fd, "asadf", 6, 0);
    char buffer[256];
    f->vnode_ops->vnode_rd(fd, buffer, 6, 1);//REMEMBER TO USE THE APPROPRIATE VNODE FOR THE VNODE OPS OR IT WILL PAGE FAULT
    kprintf("%s\n", buffer);


}





// IMPLEMENT THESE AS SYSCALLS
// void fopen() {}
// void fread() {}
// void fwrite() {}
// void fclose() {}
// void fstat() {}
// void fseek() {}
// void ftell() {}