#include <vfs.h>
#include <tmpfs.h>//i put this here instead of inside vfs.h because it was causing definition or redefinition? issues
#include <loader.h>

#include <nanoprintf-0.5.4/nanoprintf.h>

#define OA_HASH_HEADER
#include "./oa_hash/oa_hash.h"

struct vfs_fd_table {
    OA_HASH_ATTRS(mut);

};

vnode_t* tmpfs_root;

void init_vfs(volatile struct limine_module_request* module_request) {

    //init oa_hash stuff

    


    vfs_t* tmpfs = init_tmpfs();

    kprintf("\n\n");

    
    vnode_t* root = tmpfs->vnode_covered;
    // root = tmpfs->vnode_covered;

    tmpfs_root = tmpfs->vnode_covered;
    
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
            // char* file_data = (char*) kmalloc_byte(8192);//too lazy to compute for octal to decimal size again...
            char* file_data = (char*) kmalloc_byte(8);//i don't think we actually need to allocate the size of the file. only the size of the pointer because it's a pointer to the file data and not the file data itself, and tar_lookup_bin() changes the value of the pointer via a double pointer
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


            char* file_data1 = (char*) kmalloc_byte(8);
            b_size = tar_lookup_bin(tarball, "bye.txt", &file_data1);


            for (int i = 0; i < 5; i++) {
                char strn[8];
                itoa(i, strn);
                root->vnode_ops->vnode_create(root, strn, 100);
                
                root->vnode_ops->vnode_remove(root, ((tmpfs_header_t*) root->vnode_ops->vnode_lookup(root, strn)->vnode_data)->name);
            }

            // for (int i = 0; i < 1200; i++) {
            //     if (i % 2 == 0) {
            //         char strn[8];
            //         itoa(i, strn);
            //         root->vnode_ops->vnode_remove(root, root->vnode_ops->vnode_lookup(root, strn));
            //     }
            // }

            root->vnode_ops->vnode_create(root, "bye2.txt", b_size);
            vnode_t* f1 = root->vnode_ops->vnode_lookup(root, "bye2.txt");
            vfs_fd_t* fd1 = (vfs_fd_t*) tmpfs_open(root->vnode_data, "bye2.txt", 0);


            
            f1->vnode_ops->vnode_wr(fd1, file_data1, b_size, 0);

            buffer1 = (char*) krealloc_byte((uint64_t*) buffer1, b_size);
            f1->vnode_ops->vnode_rd(fd1, buffer1, b_size, 0);

            tmpfs_close((tmpfs_fd_t*) fd1);

            //HERE always make sure that the string is null terminated when you print it. i think it works here because the memory was originally all zeros so the string was automatically null terminated
            kprintf("%s\n", buffer1);

            // f->vnode_ops->vnode_rd(fd, buffer, b_size, 0);



            void* exec_ptr = (void*) kmalloc_byte(8);
            b_size = tar_lookup_bin(tarball, "a.out", (char**) &exec_ptr);
            root->vnode_ops->vnode_create(root, "a.out", b_size);
            vnode_t* exec_file = root->vnode_ops->vnode_lookup(root, "a.out");

            vfs_fd_t* exec_fd = (vfs_fd_t*) tmpfs_open(root->vnode_data, "a.out", 0);
            exec_file->vnode_ops->vnode_wr(exec_fd, exec_ptr, b_size, 0);

            // init_loader(exec_fd);

            
            hot_exec_elf(0, exec_fd);
            hot_create_and_push_user_thread(1, test_a);
            // hot_create_and_push_user_thread(2, test_a);
            // hot_exec_elf(3, exec_fd);
            // hot_create_and_push_thread(4, gen2);
            // hot_create_and_push_thread(6, gen2);
            // hot_create_and_push_thread(12, gen3);
            // hot_create_and_push_thread(13, gen3);
            // hot_create_and_push_user_thread(14, gen3);

            for (int i = 0; i < 100; i++) {
                // hot_exec_elf(i, exec_fd);
                // hot_exec_elf(i+15, test_a);
                // hot_create_and_push_thread(i, gen2);
            }
            for (int i = 100; i < 200; i++) {
                hot_create_and_push_thread(i, gen2);
            }
            for (int i = 200; i < 300; i++) {
                hot_create_and_push_user_thread(i, test_a);
            }
            while (1) reschedule();
            

        }
    }




    // vnode_unmount_vfs
}

//make this non tmpfs specific
int vfs_fdopen(tmpfs_directory_t* dir, char* name, uint8_t mode) {

    tmpfs_fd_t* fd = (tmpfs_fd_t*) tmpfs_open(dir, name, mode);
    if (fd == NULL) {
        return -1;
    }

    assert(fd != NULL);//redundant but oh well
    
    if (scheduling_started) {
        //ht is fd_table

        struct oa_hash* ht = (struct oa_hash*) get_current_thread()->fd_table;
        if (ht->length == 33) {
            //HERE remember to recheck arithmetic
            struct oa_hash_entry* new_buckets = (struct oa_hash_entry*) kmalloc_byte(sizeof(*(ht->buckets)) * (ht->capacity + 32));//always remember to use capacity and not length where necessary
            struct oa_hash_entry* old_buckets = oa_hash_rehash(ht, new_buckets, ht->capacity + 32);
            if (old_buckets) {
                assert(old_buckets == ht->buckets);
                kfree((uint64_t*) old_buckets);
                ht->buckets = new_buckets;
            }
            else {
                kfree((uint64_t*) new_buckets);
            }
        }



        char* buf = (char*) kmalloc_byte(64);
        int htlen = ht->length;//we need to store the length before it gets changed by oa_hash_set() and prevent ourselves from using the modified value
        int len = npf_snprintf(buf, 64, "%d", htlen);


        if (oa_hash_get_entry(ht, buf, len) == NULL) {
            oa_hash_set(ht, buf, len, fd);
            return htlen;
        }
        else {
            for (int i = 0; i < ht->capacity; i++) {
                memset(buf, 0, 64);//figure out if snprintf will clear the entire buffer since we entered 64
                len = npf_snprintf(buf, 64, "%d", i);
                if (oa_hash_get(ht, buf, len) == NULL) {
                    oa_hash_set(ht, buf, len, fd);
                    kfree((uint64_t*) buf);
                    // break;//always remember to break
                    return i;
                }
            }
        }

        //and then add the int fd and stuff
            
        //if it's 33
        //remember to ask about memcpy and so on
        //hash fd to int
    }

    else {
        return -1;//make sure we don't incorrectly cast the returned val to an unsigned int causing an overflow
    }

    //return the int fd
}

// make this non tmpfs specific
int vfs_fdclose(int fd) {
    struct oa_hash* ht = (struct oa_hash*) get_current_thread()->fd_table;
    char* buf = (char*) kmalloc_byte(64);
    int len = npf_snprintf(buf, 64, "%d", fd);
    tmpfs_fd_t* file = (tmpfs_fd_t*) oa_hash_get(ht, buf, len);
    oa_hash_remove(ht, buf, len);
    kfree((uint64_t*) buf);
    tmpfs_close(file);
}

int vfs_close(vfs_fd_t* fd) {

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