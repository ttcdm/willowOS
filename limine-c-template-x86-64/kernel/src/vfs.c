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
    // vfs_fd_t* fd = (vfs_fd_t*) tmpfs_open(root->vnode_data, "hi.txt", 0);
    vfs_fd_t* fd = NULL;
    assert(fd != NULL);
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
            // vnode_t* f = root->vnode_ops->vnode_lookup(root, "hi2.txt");
            // vnode_t* f = vfs_resolve_path(root, "hi2.txt");
            // vfs_fd_t* fd = (vfs_fd_t*) tmpfs_open(root->vnode_data, "hi2.txt", 0);
            // f->vnode_ops->vnode_wr(fd, file_data, b_size, 0);

            // char* buffer1 = (char*) kmalloc_byte(b_size);
            // f->vnode_ops->vnode_rd(fd, buffer1, b_size, 0);//remember to always read with offset 0 if you wanna read from the beginning





            

            kprintf("\n\n\n");

            vnode_t* current_dir = root;
            
            current_dir = root->vnode_ops->vnode_mkdir(current_dir, "test dir 4");
            vnode_t* fff = current_dir;
            current_dir = root->vnode_ops->vnode_mkdir(current_dir, "test dir 5");
            vnode_t* ff = root->vnode_ops->vnode_create(current_dir, "hihi.txt", 128);//always remember to include the size for it as well


            kprintf("%llx\n", ff->vnode_data);


            // vfs_fd_t* fd = (vfs_fd_t*) tmpfs_open(root->vnode_data, "hi2.txt", 0);
            // vfs_fd_t* fd = (vfs_fd_t*) tmpfs_open_file(ff->vnode_data, 0);
            vfs_fd_t* fd = NULL;
            assert(fd != NULL);
            f->vnode_ops->vnode_wr(fd, file_data, b_size, 0);


            vnode_t* f = vfs_resolve_path(root, "/");
            // kprintf("%llx\n", f->vnode_data);
            tmpfs_list_files(f->vnode_data);

            kprintf("\n\n\n");

            // char* buffer1 = (char*) kmalloc_byte(b_size);
            // f->vnode_ops->vnode_rd(fd, buffer1, b_size, 0);//remember to always read with offset 0 if you wanna read from the beginning


            for (int i = 0; i < 100; i++) {
                // kprintf("\n");
            }
            // kprintf("%s\n", buffer1);

            


            

            // while (1);


            // char* file_data1 = (char*) kmalloc_byte(8);
            // b_size = tar_lookup_bin(tarball, "bye.txt", &file_data1);


            // for (int i = 0; i < 5; i++) {
            //     char strn[8];
            //     itoa(i, strn);
            //     root->vnode_ops->vnode_create(root, strn, 100);
                
            //     root->vnode_ops->vnode_remove(root, ((tmpfs_header_t*) root->vnode_ops->vnode_lookup(root, strn)->vnode_data)->name);
            // }

            // // for (int i = 0; i < 1200; i++) {
            // //     if (i % 2 == 0) {
            // //         char strn[8];
            // //         itoa(i, strn);
            // //         root->vnode_ops->vnode_remove(root, root->vnode_ops->vnode_lookup(root, strn));
            // //     }
            // // }

            // root->vnode_ops->vnode_create(root, "bye2.txt", b_size);
            // vnode_t* f1 = root->vnode_ops->vnode_lookup(root, "bye2.txt");
            // vfs_fd_t* fd1 = (vfs_fd_t*) tmpfs_open(root->vnode_data, "bye2.txt", 0);


            
            // f1->vnode_ops->vnode_wr(fd1, file_data1, b_size, 0);

            // buffer1 = (char*) krealloc_byte((uint64_t*) buffer1, b_size);
            // f1->vnode_ops->vnode_rd(fd1, buffer1, b_size, 0);

            // tmpfs_close((tmpfs_fd_t*) fd1);

            // //HERE always make sure that the string is null terminated when you print it. i think it works here because the memory was originally all zeros so the string was automatically null terminated
            // kprintf("%s\n", buffer1);

            // // f->vnode_ops->vnode_rd(fd, buffer, b_size, 0);



            // void* exec_ptr = (void*) kmalloc_byte(8);
            // b_size = tar_lookup_bin(tarball, "a.out", (char**) &exec_ptr);
            // root->vnode_ops->vnode_create(root, "a.out", b_size);
            // vnode_t* exec_file = root->vnode_ops->vnode_lookup(root, "a.out");

            // vfs_fd_t* exec_fd = (vfs_fd_t*) tmpfs_open(root->vnode_data, "a.out", 0);
            // exec_file->vnode_ops->vnode_wr(exec_fd, exec_ptr, b_size, 0);

            // // init_loader(exec_fd);

            

            // void* testt_ptr;
            // b_size = tar_lookup_bin(tarball, "test", (char**) &testt_ptr);
            // root->vnode_ops->vnode_create(root, "testt", b_size);
            // vnode_t* testt_file = root->vnode_ops->vnode_lookup(root, "testt");

            // vfs_fd_t* testt_fd = (vfs_fd_t*) tmpfs_open(root->vnode_data, "testt", 0);
            // testt_file->vnode_ops->vnode_wr(testt_fd, testt_ptr, b_size, 0);



            // vnode_t* hi2 = vfs_resolve_path("hi2.txt", root);
            // ((tmpfs_file_t*) hi2->vnode_data)->
            // char* buf = (char*) kmalloc_byte(20);
            
            // root->vnode_ops->vnode_rd
            // for (int j = 0; j < b_size; j++) {
            //     kprintf("%c", buf[j]);
            // }



            // syscall_log("hi");

            // hot_exec_elf(480, testt_fd);
            // hot_exec_elf(481, testt_fd);
            // syscall17(2, "hi", 2);


            // hot_exec_elf(0, exec_fd);
            // hot_create_and_push_user_thread(1, test_a);
            // hot_create_and_push_user_thread(2, test_a);
            // hot_exec_elf(3, exec_fd);
            // hot_create_and_push_thread(4, gen2);
            // hot_create_and_push_thread(6, gen2);
            hot_create_and_push_thread(get_new_pid(), gen3);
            hot_create_and_push_thread(get_new_pid(), gen3);
            hot_create_and_push_thread(get_new_pid(), gen3);
            hot_create_and_push_thread(get_new_pid(), gen3);
            hot_create_and_push_thread(get_new_pid(), gen3);
            hot_create_and_push_thread(get_new_pid(), gen3);
            hot_create_and_push_thread(get_new_pid(), gen3);
            // hot_create_and_push_thread(13, gen3);
            // hot_create_and_push_user_thread(14, gen3);

            for (int i = 6; i < 100-90; i++) {//because 5 gets created
                // hot_exec_elf(i, exec_fd);
                // hot_exec_elf(i+15, test_a);
                // hot_create_and_push_thread(i, gen2);
            }
            for (int i = 100; i < 200-90-5; i++) {
                // hot_create_and_push_thread(i, gen2);
                // hot_exec_elf(i, testt_fd);
            }
            for (int i = 200; i < 300-90; i++) {
                // hot_create_and_push_user_thread(i, test_a);
            }
            while (1) reschedule();
            

        }
    }




    // vnode_unmount_vfs
}





size_t pread(vfs_file_t* file, void* buf, uint64_t size, uint64_t offset) {
    return file->vnode->vnode_ops->vnode_rd(file->vnode, buf, size, offset);
}

size_t fd_pread(int fd, void* buf, uint64_t size, uint64_t offset) {
    vfs_file_t* file = vfs_int_fd_to_vfs_file(fd);
    return file->vnode->vnode_ops->vnode_rd(file->vnode, buf, size, offset);
}

void pwrite(vfs_file_t* file, void* data, uint64_t size, uint64_t offset) {
    file->vnode->vnode_ops->vnode_wr(file->vnode, data, size, offset);
}
void fd_pwrite(int fd, void* data, uint64_t size, uint64_t offset) {
    vfs_file_t* file = vfs_int_fd_to_vfs_file(fd);
    file->vnode->vnode_ops->vnode_wr(file->vnode, data, size, offset);
}

int fd_open(char* path, int flags, int mode) {
    return vfs_fdopen(path, flags, mode);
}
int fd_close(int fd) {
    vfs_fdclose(fd);
}






//make this non tmpfs specific
int vfs_fdopen(char* path, int flags, int mode) {
    if (!scheduling_started) {
        kprintf("vfs_fd_open(): scheduling not started\n");
        return -1;
    }
    //HERE we're using tmpfs root as our root fs/root vnode or something. i think that this is okay
    vnode_t* v;
    if (path[0] == '/') {
        v = vfs_resolve_path(tmpfs_root, path);
    }
    else {
        path = strdup(path);
        strcat(path, get_current_thread()->current_dir);
        kprintf("path: %s\n", path);
        v = vfs_resolve_path(tmpfs_root, path);
        kfree((uint64_t*) path);
        path = NULL;
    }
    assert(v != NULL);
    

    vfs_file_t* fd = (vfs_file_t*) kmalloc_byte(sizeof(vfs_file_t));

    fd->mode = mode;
    fd->flags = flags;
    fd->file_ops = (vfs_file_ops_t*) kmalloc_byte(sizeof(vfs_file_ops_t));
    fd->mutex = (mutex_t) {0, NULL};//i hope that this is allowed lmao
    fd->position = 0;
    fd->vnode = v;
    fd->abs_path = NULL;//not sure if we should use relative path or abs path. relative path would probably be better

    
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
    vfs_file_t* file = (vfs_file_t*) oa_hash_get(ht, buf, len);
    oa_hash_remove(ht, buf, len);
    kfree((uint64_t*) buf);
    
    if (file != NULL) {
        if (file->abs_path != NULL) kfree((uint64_t*) file->abs_path);
        if (file->file_ops != NULL) kfree((uint64_t*) file->file_ops);
        kfree((uint64_t*) file);
    }


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

//if the path starts with a '/' we assume that it's an absolute path and tmpfs_root (for now) will be used instead of root_dir
vnode_t* vfs_resolve_path(vnode_t* root_dir, char* path) {
    assert(path != NULL);
    //no check against non null terminated strings
    if (path[0] == '/') {//i think and i hope that the original value that was passed into this function doesn't get changed by changing the arg here
        root_dir = tmpfs_root;//HERE we're using tmpfs_root as our root vnode for lookups for now. i'm not sure if we have a root vnode for all vfs's if that's even a thing
    }
    
    vnode_t* ret = root_dir;
    char* save;
    //HERE use strtok_r instead of strtok because it's reenetrant and thread safe i think
    path = strdup(path);//not sure why but strdup was needed to make it work
    char* tok = strtok_r(path, "/", &save);
    while (tok != NULL) {
        //HERE remember to increase the reference count of each vnode
        ret = root_dir->vnode_ops->vnode_lookup(ret, tok);
        tok = strtok_r(NULL, "/", &save);
    }
    kfree((uint64_t*) path);
    return ret;
}

vfs_file_t* vfs_int_fd_to_vfs_file(int fd) {
    struct oa_hash* ht = (struct oa_hash*) get_current_thread()->fd_table;
    char* buf = (char*) kmalloc_byte(64);
    int len = npf_snprintf(buf, 64, "%d", fd);
    vfs_file_t* file = (vfs_file_t*) oa_hash_get(ht, buf, len);
    assert(file != NULL);
    kfree((uint64_t*) buf);
    return file;
}


// IMPLEMENT THESE AS SYSCALLS
// void fopen() {}
// void fread() {}
// void fwrite() {}
// void fclose() {}
// void fstat() {}
// void fseek() {}
// void ftell() {}