#include <vfs.h>
#include <tmpfs.h>//i put this here instead of inside vfs.h because it was causing definition or redefinition? issues

void init_vfs(struct limine_module_request* module_request) {
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



    // map_page((uint64_t*) pml4_address_virt_glob, module_request->response->modules[0]->address, module_request->response->modules[0]->address, 0b11);
    // kprintf("%s", module_request->response->modules[0]->cmdline);
    // kprintf("%llx\n", module_request->response->modules[0]);
    // assert(module_request->response->modules[0]);
    for (int i = 0; i < module_request->response->module_count; i++) {
        kprintf("%s\n", module_request->response->modules[i]->path);
        if (strcmp(module_request->response->modules[i]->path, "/boot/tmpfs.tar") == 0) {
            void* tarball = module_request->response->modules[i]->address;
            char* file_data = (char*) kmalloc_byte(8192);//too lazy to compute for octal to decimal size again...
            uint64_t b_size = tar_lookup_bin(tarball, "hi.txt", &file_data);

            kprintf("%llu\n", b_size);

            for (int j = 0; j < b_size; j++) {
                kprintf("%c", file_data[j]);
            }

            root->vnode_ops->vnode_create(root, "hi2.txt", b_size);
            vnode_t* f = root->vnode_ops->vnode_lookup(root, "hi2.txt");
            vfs_fd_t* fd = (vfs_fd_t*) tmpfs_open(root->vnode_data, "hi2.txt", 0);
            f->vnode_ops->vnode_wr(fd, file_data, b_size, 0);

            char* buffer1 = (char*) kmalloc_byte(b_size);
            f->vnode_ops->vnode_rd(fd, buffer1, b_size, 0);//remember to always read with offset 0 if you wanna read from the beginning

            kprintf("%s\n", buffer1);


            char* file_data1 = (char*) kmalloc_byte(8192);
            b_size = tar_lookup_bin(tarball, "bye.txt", &file_data1);

            root->vnode_ops->vnode_create(root, "bye2.txt", b_size);
            vnode_t* f1 = root->vnode_ops->vnode_lookup(root, "bye2.txt");
            vfs_fd_t* fd1 = (vfs_fd_t*) tmpfs_open(root->vnode_data, "bye2.txt", 0);


            
            f1->vnode_ops->vnode_wr(fd1, file_data1, b_size, 0);

            buffer1 = krealloc_byte(buffer1, b_size);
            f1->vnode_ops->vnode_rd(fd1, buffer1, b_size, 0);

            kprintf("%s\n", buffer1);
            

            // f->vnode_ops->vnode_rd(fd, buffer, b_size, 0);
            

        }
    }




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