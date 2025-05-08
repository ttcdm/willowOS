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



    root->vnode_ops->vnode_mkdir(root, "test dir 3");

    vnode_t* d = root->vnode_ops->vnode_lookup(root, "test dir 3");

    vnode_mount_vfs(d, tmpfs);

    vnode_t* f2 = d->vnode_ops->vnode_lookup(d, "TMPFS_ROOT");
    kprintf("%s\n", ((tmpfs_header_t*)f2->vnode_data)->name);


    tmpfs_list_files(f2->vnode_data);

    // vnode_unmount_vfs
}



void vnode_mount_vfs(vnode_t* parent_vnode, vfs_t* vfs) {//this most likely isn't the correct way to implement it...
    if (vfs == NULL) {
        kprintf("vnode_mount_vfs(): vfs is null\n");
    }
    parent_vnode->mounted_vfs = vfs;
}

void vnode_unmount_vfs(vnode_t* parent_vnode, char* name) {
    //need to lookup() inside the vnode
    //we can either use a linked list approach and remove the element from the linked list and link the previous vfs to the next one
    //or we can use an array approach and set the vfs to null but it'll also mean we need to implement realloc first
}




// IMPLEMENT THESE AS SYSCALLS
// void fopen() {}
// void fread() {}
// void fwrite() {}
// void fclose() {}
// void fstat() {}
// void fseek() {}
// void ftell() {}