#include <tmpfs.h>


vfs_t* vfs_tmpfs;//not sure if we should actually declare this as a global variable, but this is just for linking the vnodes to tmpfs
vfs_t* init_tmpfs() {
    //we load stuff from ustar and we just parse it and create our own custom filesystem
    tmpfs_directory_t* root_dir_pointer = (tmpfs_directory_t*) kmalloc_byte(sizeof(tmpfs_directory_t));
    //HERE always remember to initialize the stuff because we're manually creating the dir ourselves
    root_dir_pointer->max_files = 2;
    root_dir_pointer->num_files = 0;
    root_dir_pointer->header.mutex = (mutex_t*) kmalloc_byte(sizeof(mutex_t));
    root_dir_pointer->header.mutex->locked = 0;
    root_dir_pointer->files = (void**) kmalloc_byte(root_dir_pointer->max_files * root_dir_pointer->max_files * sizeof(void*));
    for (int i = 0; i < root_dir_pointer->max_files; i++) {
        root_dir_pointer->files[i] = NULL;
    }
    tmpfs_directory_t* root_dir = tmpfs_create_directory(root_dir_pointer, "TMPFS_ROOT");//not sure if i should actually do it with a root dir pointer
    

    vfs_tmpfs = (vfs_t*) kmalloc_byte(sizeof(vfs_t));//place inside init_tmpfs() and have it return this
    // vfs_ops_t* tmpfs_ops = (vfs_ops_t*) kmalloc_byte(sizeof(vfs_ops_t));

    // vfs_tmpfs->vnode_covered = tmpfs_link_vnode(root_dir, VDIR);
    vnode_t* root_vnode = tmpfs_link_vnode(root_dir, VDIR);
    vfs_tmpfs->vnode_covered = root_vnode;
    vfs_ops_t* vops = (vfs_ops_t*) kmalloc_byte(sizeof(vfs_ops_t));
    vfs_tmpfs->vfs_ops = vops;
    vfs_tmpfs->vfs_ops->vfs_mount = vnode_mount_vfs;


    // vnode_t* tmpfs_root_vnode = (vnode_t*) kmalloc_byte(sizeof(vnode_t));
    // tmpfs_root_vnode->vnode_data = (vnode_t*) root_dir;
    


    tmpfs_file_t* test_file = tmpfs_create_file(root_dir, "test file", 4096);
    tmpfs_directory_t* test_dir = tmpfs_create_directory(root_dir, "test dir");
    tmpfs_list_files(root_dir);
    tmpfs_delete_file(root_dir, "test file");
    tmpfs_list_files(root_dir);
    tmpfs_delete_directory(root_dir, "test dir");
    tmpfs_list_files(root_dir);
    tmpfs_directory_t* test_dir_1 = tmpfs_create_directory(root_dir, "test dir 1");
    tmpfs_list_files(test_dir_1);
    tmpfs_file_t* test_file_1 = tmpfs_create_file(test_dir_1, "test file 1", 1000);
    tmpfs_list_files(test_dir_1);
    tmpfs_directory_t* test_dir_2 = tmpfs_create_directory(test_dir_1, "test dir 2");
    tmpfs_list_files(test_dir_1);
    tmpfs_create_file(test_dir_2, "test file 2", 1000);
    tmpfs_delete_directory_no_orphan(test_dir_1, "test dir 2");
    tmpfs_list_files(test_dir_1);
    for (int i = 0; i < 3; i++) {
        // tmpfs_create_file(test_dir_1, "test file", 4096);
    }
    tmpfs_list_files(test_dir_1);
    tmpfs_delete_file(root_dir, "test file 2");//can't be found because it's orphaned
    tmpfs_delete_file(root_dir, "test file 1");
    tmpfs_delete_file(root_dir, "test file 1");


    tmpfs_list_files(root_dir);
    tmpfs_list_files(test_dir_1);

    tmpfs_create_file(test_dir_1, "test file", 4096);

    tmpfs_fd_t* f = tmpfs_open(test_dir_1, "test file", 0);
    tmpfs_write_to_file(f, "hello world", 11, 0);

    char* buffer = (char*) kmalloc_byte(4096);
    
    tmpfs_read_from_file(f, buffer, 11, 0);
    kprintf("%s\n", buffer);
    tmpfs_write_to_file(f, "hello world", 11, 11);
    tmpfs_read_from_file(f, buffer, 64, 0);
    kprintf("%s\n", buffer);
    tmpfs_close(f);
    tmpfs_read_from_file(f, buffer, 64, 0);

    tmpfs_directory_t* temp = tmpfs_lookup(root_dir, "test dir 1");
    kprintf("%s\n", temp->header.name);
    tmpfs_lookup(root_dir, "test dir");
    tmpfs_file_t* temp1 = tmpfs_lookup(test_dir_1, "test file");
    kprintf("%s\n", temp1->header.name);
    
    
    return vfs_tmpfs;
}

//HERE remember to use mutexes and lock around all file related io stuff to prevent weird race conditions and other bugs

void* tmpfs_create_file(tmpfs_directory_t* dir, char* name, uint64_t size) {
    bool irq;
    irq_disable_save(&irq);


    //dir->probably_next_free_entry_index is just used for indexing and not used for any probably free next stuff
    // dir->probably_next_free_entry_index = 0;
    uint64_t free_index = 0;
    if (dir->files[free_index] != NULL) {
        for (int i = 0; i < dir->max_files; i++) {
            if (dir->files[i] == NULL) {
                free_index = i;
                break;
            }
        }
    }
    tmpfs_file_t* new_file = (tmpfs_file_t*) kmalloc_byte(sizeof(tmpfs_file_t));
    new_file->header.permissions[0] = 'r';
    new_file->header.permissions[1] = 'w';
    new_file->header.permissions[2] = 'x';
    new_file->header.user_id = 0;
    new_file->header.group_id = 0;
    new_file->header.type = 0;
    new_file->header.mutex = (mutex_t*) kmalloc_byte(sizeof(mutex_t));
    new_file->header.mutex->locked = 0;
    strcpy(new_file->header.fs_type, "tmpfs");
    new_file->open_count = 0;
    for (int i = 0; i < 3; i++) {new_file->header.timestamps[i] = 0;}//remmeber to switch to tsc
    strncpy(new_file->header.name, name, kstrlen(name)+1);
    new_file->size = size;

    acquire_mutex(dir->header.mutex);
    dir->files[free_index] = new_file;

    new_file->data = kmalloc_byte(size);

    dir->num_files++;
    if (dir->num_files == dir->max_files) {
        dir->max_files += 32;
        dir->files = (void**) krealloc_byte((uint64_t*) dir->files, dir->max_files * sizeof(void*));//remember to do sizeof
        for (int i = dir->max_files - 32; i < dir->max_files; i++) {//hopefully there's no off by one error
            dir->files[i] = NULL;
        }
    }
    release_mutex(dir->header.mutex);

    // new_file->header.path;//do something about the path

    // tmpfs_link_vnode(new_file, VREG);//not 100% sure how i'm supposed to go about this

    //maybe use a mutex instead idk
    irq_restore(&irq);
    return new_file;

}



void* tmpfs_create_directory(tmpfs_directory_t* dir, char* name) {
    bool irq;
    irq_disable_save(&irq);
    
    //not needed because i don't think we can actually run out
    // if (dir->probably_next_free_entry_index == dir->max_files) {//because len-1
    //     kprintf("tmpfs_create_directory(): out of space\n");
    //     return NULL;
    // }

    uint64_t free_index = 0;
    if (dir->files[free_index] != NULL) {
        for (int i = 0; i < dir->max_files; i++) {
            if (dir->files[i] == NULL) {
                free_index = i;
                break;
            }
        }
    }
    tmpfs_directory_t* new_dir = (tmpfs_directory_t*) kmalloc_byte(sizeof(tmpfs_directory_t));
    new_dir->header.permissions[0] = 'r';
    new_dir->header.permissions[1] = 'w';
    new_dir->header.permissions[2] = 'x';
    new_dir->header.user_id = 0;
    new_dir->header.group_id = 0;
    new_dir->header.type = 1;
    new_dir->header.mutex = (mutex_t*) kmalloc_byte(sizeof(mutex_t));
    new_dir->header.mutex->locked = 0;
    strcpy(new_dir->header.fs_type, "tmpfs");
    new_dir->max_files = 32;
    new_dir->num_files = 0;
    new_dir->files = (void**) kmalloc_byte(new_dir->max_files * sizeof(void*));
    for (int i = 0; i < new_dir->max_files; i++) {
        new_dir->files[i] = NULL;
    }


    for (int i = 0; i < 3; i++) {new_dir->header.timestamps[i] = 0;}//remmeber to switch to tsc
    strncpy(new_dir->header.name, name, kstrlen(name)+1);
    for (int i = 0; i < dir->max_files; i++) {new_dir->files[i] = NULL;}//not sure if this is necessary

    acquire_mutex(dir->header.mutex);
    dir->files[free_index] = new_dir;

    //i should probably put this in a separate function since i'm copy pasting this multiple times
    dir->num_files++;
    if (dir->num_files == dir->max_files) {
        dir->max_files += 32;
        dir->files = (void**) krealloc_byte((uint64_t*) dir->files, dir->max_files * sizeof(void*));//remember to do sizeof
        for (int i = dir->max_files - 32; i < dir->max_files; i++) {//hopefully there's no off by one error
            dir->files[i] = NULL;
        }
    }
    release_mutex(dir->header.mutex);
    
    // tmpfs_link_vnode(new_dir, VDIR);//not 100% sure how i'm supposed to go about this

    irq_restore(&irq);
    return new_dir;
}

void tmpfs_delete_file(tmpfs_directory_t* dir, char* name) {//recursive search
    for (int i = 0; i < dir->max_files; i++) {
        if (dir->files[i] == NULL) {continue;}//can't put it in the if statement below because strcmp runs first so if it is null it'll page fault
        //if the names are the same and if it isn't null and if its type is a file
        if ((strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0) && (((tmpfs_header_t*) dir->files[i])->type == 0)) {//we cast to header and not file because it could be either a file or a directory
            if (((tmpfs_file_t*)dir->files[i])->open_count != 0) {//if there's still something using the file
                kprintf("tmpfs_delete_file(): cannot delete file. file in use\n");
                return;
            }
            kfree(((tmpfs_file_t*)dir->files[i])->data);
            kfree((uint64_t*) ((tmpfs_file_t*)dir->files[i])->header.mutex);
            kfree(dir->files[i]);
            acquire_mutex(dir->header.mutex);
            dir->files[i] = NULL;
            dir->num_files--;
            release_mutex(dir->header.mutex);
            //should probably add a shrinking realloc as well if max_files - num_files > 32
            return;
        }
        else if ((((tmpfs_header_t*) dir->files[i])->type == 1)) {
            // tmpfs_delete_file(((tmpfs_directory_t*) dir->files[i]), name);//we don't recursively search for file to delete. only the stuff inside the current directory
        }
    }
    kprintf("tmpfs_delete_file(): file not found\n");
}
void tmpfs_delete_directory(tmpfs_directory_t* dir, char* name) {//we orphan the files ig. also, if we do end up not doing a root dir pointer, i'm not actually sure how you would delete it with this function since you don't have a parent directory to parse through
    for (int i = 0; i < dir->max_files; i++) {
        if (dir->files[i] == NULL) {continue;}
        //if the names are the same and if it isn't null and if its type is a directory
        if ((strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0) && (((tmpfs_header_t*) dir->files[i])->type == 1)) {//we cast to header and not file because it could be either a file or a directory
            kfree((uint64_t*) ((tmpfs_directory_t*) dir->files[i])->header.mutex);
            kfree(dir->files[i]);
            acquire_mutex(dir->header.mutex);
            dir->files[i] = NULL;
            dir->num_files--;
            release_mutex(dir->header.mutex);
            return;
        }
        else if ((((tmpfs_header_t*) dir->files[i])->type == 1) && (strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0)) {
            // tmpfs_delete_directory(((tmpfs_directory_t*) dir->files[i]), name);//we don't recursively search for file. only the stuff inside the current directory
        }
    }
    kprintf("tmpfs_delete_directory(): directory not found\n");
}

void tmpfs_delete_directory_no_orphan(tmpfs_directory_t* dir, char* name) {//we orphan the files ig. also, if we do end up not doing a root dir pointer, i'm not actually sure how you would delete it with this function since you don't have a parent directory to parse through
    for (int i = 0; i < dir->max_files; i++) {
        if (dir->files[i] == NULL) {continue;}
        //if the names are the same and if it isn't null and if its type is a directory
        if ((strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0) && (((tmpfs_header_t*) dir->files[i])->type == 1)) {//we cast to header and not file because it could be either a file or a directory
            for (int j = 0; j < dir->max_files; j++) {

                //HERE
                break;

                if (((tmpfs_directory_t*) dir->files[i])->files[j] != NULL) {
                    //HERE remember to add recursive deletion since this only deletes the files in the current directory
                    //remember to delete file data as well
                    kfree(((tmpfs_directory_t*) dir->files[i])->files[j]);
                    ((tmpfs_directory_t*) dir->files[i])->files[j] = NULL;
                    ((tmpfs_directory_t*) dir->files[i])->num_files--;
                }
            }
            tmpfs_delete_directory_no_orphan(((tmpfs_directory_t*) dir->files[i]), name);
            acquire_mutex(dir->header.mutex);
            kfree((uint64_t*) ((tmpfs_file_t*)dir->files[i])->header.mutex);
            kfree(dir->files[i]);
            dir->files[i] = NULL;
            dir->num_files--;
            release_mutex(dir->header.mutex);
            return;
        }
        else if ((((tmpfs_header_t*) dir->files[i])->type == 0) && (strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0)) {
            kfree(((tmpfs_file_t*)dir->files[i])->data);
            kfree((uint64_t*) ((tmpfs_file_t*)dir->files[i])->header.mutex);
            kfree(dir->files[i]);
            acquire_mutex(dir->header.mutex);
            dir->files[i] = NULL;
            dir->num_files--;
            release_mutex(dir->header.mutex);
            return;
        }
        
        continue;

        // else if ((((tmpfs_header_t*) dir->files[i])->type == 1) && (strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0)) {
        //     tmpfs_delete_directory(((tmpfs_directory_t*) dir->files[i]), name);//we don't recursively search for file. only the stuff inside the current directory
        // }
    }
    kprintf("tmpfs_delete_directory(): directory not found\n");
}

void tmpfs_list_files(tmpfs_directory_t* dir) {//remember that it's files and not file. also maybe make this list directoreis as well?
    kprintf("-%s-\n", dir->header.name);
    for (int i = 0; i < dir->max_files; i++) {
        if (dir->files[i] != NULL) {
            //remember to add file size as well
            kprintf("%s type:%d\n", (((tmpfs_header_t*) dir->files[i])->name), (((tmpfs_header_t*) dir->files[i])->type));//you can also get the string of the type by using an inline if block with the ? operator
        }
    }
    kprintf("---\n");
}
void tmpfs_write_to_file(tmpfs_fd_t* file, void* data, uint64_t size, uint64_t offset) {//these should probably support fopen fseek ftell and such
    //remember to add support for different modes
    if (offset + size > file->size) {
        void* new_file = kmalloc_byte(offset+size);
        if (file->data) {
            acquire_mutex(file->file->header.mutex);
            memcpy(new_file, file->data, file->size);
            kfree(file->data);
        }
        file->data = new_file;
        file->size = offset+size;
        release_mutex(file->file->header.mutex);
    }

    acquire_mutex(file->file->header.mutex);
    memcpy((void*) ((uint64_t) file->data+offset), data, size);
    release_mutex(file->file->header.mutex);

}

size_t tmpfs_read_from_file(tmpfs_fd_t* file, void* data, uint64_t size, uint64_t offset) {//MUST INITIALIZE BUFFER TO FILL
    if (offset >= file->size) {
        return 1;
    }
    size_t count = file->size - offset;
    if (count > size) {
        count = size;
    }
    acquire_mutex(file->file->header.mutex);
    memcpy(data, (void*) ((uint64_t)(file->data)+offset), count);
    release_mutex(file->file->header.mutex);
    return count;
}


//HERE remember to use paths ig instead of just names and lookup maybe?
tmpfs_fd_t* tmpfs_open(tmpfs_directory_t* dir, char* name, uint8_t mode) {//maybe add safeguard against opening a file when it's already open
    tmpfs_fd_t* fd = (tmpfs_fd_t*) kmalloc_byte(sizeof(tmpfs_fd_t));//changed from kmalloc to kmalloc_byte. might've been an earlier typo
    tmpfs_file_t* file = (tmpfs_file_t*) tmpfs_lookup(dir, name);
    acquire_mutex(file->header.mutex);
    file->open_count++;
    release_mutex(file->header.mutex);
    fd->file = file;
    fd->data = file->data;
    fd->size = file->size;
    fd->mode = mode;
    fd->position = 0;
    //we always set the position to 0. since this returns a file descriptor, we leave it up to the user or other functions to modify the position
    return fd;
}

void tmpfs_close(tmpfs_fd_t* fd) {
    acquire_mutex(fd->file->header.mutex);
    fd->file->open_count--;
    release_mutex(fd->file->header.mutex);
    kfree((uint64_t*) fd);
}

void tmpfs_not_available() {
    kprintf("tmpfs function not available\n");
}


void* tmpfs_lookup(tmpfs_directory_t* dir, char* name) {
    for (int i = 0; i < dir->max_files; i++) {
        if (dir->files[i] == NULL) {continue;}
        //if the names are the same and if it isn't null and if its type is a directory
        if ((strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0)) {//we cast to header and not file because it could be either a file or a directory
            return dir->files[i];
        }
    }
    kprintf("tmpfs_lookup(): file or directory not found\n");
    return NULL;
}

vnode_t* vnode_tmpfs_lookup(vnode_t* vnode, char* name) {

    //check for if we're looking for a mounted vfs as well
    if (vnode->mounted_vfs != NULL) {//i don't think we can put this all in one if statement because checking if it's null must happen first
        if (strcmp(((tmpfs_header_t*) vnode->mounted_vfs->vnode_covered->vnode_data)->name, name) == 0) {//we check if the mounted vfs' covered vnode which is the root vnode of that fs has a name that matches the vnode we're looking for. however, this only works for tmpfs because it uses the header which contains the name, so for other fs's we must either use the same header or use the local vnode's or vfs' type for their specific lookup function idk... also this only supports only one vfs mounted per vnode for now
            return vnode->mounted_vfs->vnode_covered;
        }
    }


    tmpfs_header_t* temp = tmpfs_lookup(vnode->vnode_data, name);
    if (temp == NULL) {return NULL;}
    vnode_t* ret;
    // ret = tmpfs_link_vnode(temp, temp->type+1);//HERE vtype has vnon, vreg, vdir, which is 0, 1, 2 but tmpfs_header_t has 0, 1 which corresponds to file and dir respectively, so we get an off by one error. we can fix it by just adding 1 but explicitly specifying it via if statements is probably safer
    if (temp->type == 0) {ret = tmpfs_link_vnode(temp, VREG);}
    else if (temp->type == 1) {ret = tmpfs_link_vnode(temp, VDIR);}
    return ret;
}


vnode_t* vnode_tmpfs_create_file(vnode_t* vnode, char* name, uint64_t size) {
    tmpfs_file_t* file = tmpfs_create_file((tmpfs_directory_t*) vnode->vnode_data, name, size);
    vnode_t* ret = tmpfs_link_vnode(file, VREG);
    return ret;
}

void vnode_tmpfs_delete_file(vnode_t* vnode, char* name) {
    tmpfs_delete_file((tmpfs_directory_t*) vnode->vnode_data, name);
}

void* vnode_tmpfs_create_directory(vnode_t* vnode, char* name) {
    tmpfs_directory_t* dir = tmpfs_create_directory((tmpfs_directory_t*) vnode->vnode_data, name);
    vnode_t* ret = tmpfs_link_vnode(dir, VDIR);
    return ret;
}

void vnode_tmpfs_delete_directory(vnode_t* vnode, char* name) {
    tmpfs_delete_directory((tmpfs_directory_t*) vnode->vnode_data, name);
}

void vnode_tmpfs_delete_directory_no_orphan(vnode_t* vnode, char* name) {
    tmpfs_delete_directory_no_orphan((tmpfs_directory_t*) vnode->vnode_data, name);
}

void vnode_tmpfs_write_to_file(vfs_fd_t* file, void* data, uint64_t size, uint64_t offset) {
    tmpfs_write_to_file((tmpfs_fd_t*) file, data, size, offset);
}

size_t vnode_tmpfs_read_from_file(vfs_fd_t* file, void* data, uint64_t size, uint64_t offset) {
    return tmpfs_read_from_file((tmpfs_fd_t*) file, data, size, offset);
}

vnode_t* tmpfs_link_vnode(void* file_object, enum vtype type) {
    vnode_t* new_vnode = (vnode_t*) kmalloc_byte(sizeof(vnode_t));
    new_vnode->vnode_type = type;
    //do something about storing paths either here or inside the file object
    //remember to add reference count as well and maybe the rest of the members
    //remember to add ioctl as well
    vnode_ops_t* ops = (vnode_ops_t*) kmalloc_byte(sizeof(vnode_ops_t));
    new_vnode->vnode_ops = ops;
    if (type == VDIR) {
        new_vnode->vnode_ops->vnode_create = vnode_tmpfs_create_file;
        new_vnode->vnode_ops->vnode_remove = vnode_tmpfs_delete_file;    
        new_vnode->vnode_ops->vnode_mkdir = vnode_tmpfs_create_directory;
        new_vnode->vnode_ops->vnode_rmdir = vnode_tmpfs_delete_directory;
        new_vnode->vnode_ops->vnode_rmdir_no_orphan = vnode_tmpfs_delete_directory_no_orphan;
        new_vnode->vnode_ops->vnode_lookup = vnode_tmpfs_lookup;
    }
    else if (type == VREG) {
        new_vnode->vnode_ops->vnode_rd = vnode_tmpfs_read_from_file;//MAYBE COMBINE READ AND WRITE???
        new_vnode->vnode_ops->vnode_wr = vnode_tmpfs_write_to_file;
    }

    // vnode_t* root_vnode = (vnode_t*) root_dir;
    new_vnode->vnode_vfsmountedhere = vfs_tmpfs;
    new_vnode->vnode_data = file_object;
    return new_vnode;
}