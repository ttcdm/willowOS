#include <tmpfs.h>


vfs_t* vfs_tmpfs;//not sure if we should actually declare this as a global variable, but this is just for linking the vnodes to tmpfs
vfs_t* init_tmpfs() {
    //we load stuff from ustar and we just parse it and create our own custom filesystem
    tmpfs_directory_t* root_dir_pointer = (tmpfs_directory_t*) kmalloc_byte(sizeof(tmpfs_directory_t));
    tmpfs_directory_t* root_dir = tmpfs_create_directory(root_dir_pointer, "TMPFS_ROOT");//not sure if i should actually do it with a root dir pointer
    
    vfs_tmpfs = (vfs_t*) kmalloc_byte(sizeof(vfs_t));//place inside init_tmpfs() and have it return this
    // vfs_ops_t* tmpfs_ops = (vfs_ops_t*) kmalloc_byte(sizeof(vfs_ops_t));

    // vfs_tmpfs->vnode_covered = tmpfs_link_vnode(root_dir, VDIR);
    vnode_t* root_vnode = tmpfs_link_vnode(root_dir, VDIR);
    vfs_tmpfs->vnode_covered = root_vnode;

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

    char* buffer = kmalloc_byte(4096);
    
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

void* tmpfs_create_file(tmpfs_directory_t* dir, char* name, uint64_t size) {
    if (dir->probably_next_free_entry_index == TMPFS_MAX_FILES) {//because len-1
        kprintf("tmpfs_create_file(): out of space\n");
        return NULL;
    }
    if (dir->files[dir->probably_next_free_entry_index] != NULL) {
        for (int i = 0; i < TMPFS_MAX_FILES; i++) {
            if (dir->files[i] == NULL) {
                dir->probably_next_free_entry_index = i;
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
    for (int i = 0; i < 3; i++) {new_file->header.timestamps[i] = 0;}//remmeber to switch to tsc
    strncpy(new_file->header.name, name, kstrlen(name)+1);
    new_file->size = size;
    dir->files[dir->probably_next_free_entry_index] = new_file;
    dir->probably_next_free_entry_index++;

    new_file->data = kmalloc_byte(size);

    // new_file->header.path;//do something about the path

    // tmpfs_link_vnode(new_file, VREG);//not 100% sure how i'm supposed to go about this
    return new_file;

}



void* tmpfs_create_directory(tmpfs_directory_t* dir, char* name) {
    if (dir->probably_next_free_entry_index == TMPFS_MAX_FILES) {//because len-1
        kprintf("tmpfs_create_directory(): out of space\n");
        return NULL;
    }
    if (dir->files[dir->probably_next_free_entry_index] != NULL) {
        for (int i = 0; i < TMPFS_MAX_FILES; i++) {
            if (dir->files[i] == NULL) {
                dir->probably_next_free_entry_index = i;
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
    for (int i = 0; i < 3; i++) {new_dir->header.timestamps[i] = 0;}//remmeber to switch to tsc
    strncpy(new_dir->header.name, name, kstrlen(name)+1);
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {new_dir->files[i] = NULL;}//not sure if this is necessary
    new_dir->probably_next_free_entry_index = 0;
    dir->files[dir->probably_next_free_entry_index] = new_dir;
    dir->probably_next_free_entry_index++;
    
    // tmpfs_link_vnode(new_dir, VDIR);//not 100% sure how i'm supposed to go about this
    return new_dir;
}

void tmpfs_delete_file(tmpfs_directory_t* dir, char* name) {//recursive search
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (dir->files[i] == NULL) {continue;}//can't put it in the if statement below because strcmp runs first so if it is null it'll page fault
        //if the names are the same and if it isn't null and if its type is a file
        if ((strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0) && (((tmpfs_header_t*) dir->files[i])->type == 0)) {//we cast to header and not file because it could be either a file or a directory
            kfree(((tmpfs_file_t*)dir->files[i])->data);
            kfree(dir->files[i]);
            dir->files[i] = NULL;
            dir->probably_next_free_entry_index--;
            return;
        }
        else if ((((tmpfs_header_t*) dir->files[i])->type == 1)) {
            // tmpfs_delete_file(((tmpfs_directory_t*) dir->files[i]), name);//we don't recursively search for file to delete. only the stuff inside the current directory
        }
    }
    kprintf("tmpfs_delete_file(): file not found\n");
}
void tmpfs_delete_directory(tmpfs_directory_t* dir, char* name) {//we orphan the files ig. also, if we do end up not doing a root dir pointer, i'm not actually sure how you would delete it with this function since you don't have a parent directory to parse through
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (dir->files[i] == NULL) {continue;}
        //if the names are the same and if it isn't null and if its type is a directory
        if ((strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0) && (((tmpfs_header_t*) dir->files[i])->type == 1)) {//we cast to header and not file because it could be either a file or a directory
            kfree(dir->files[i]);
            dir->files[i] = NULL;
            dir->probably_next_free_entry_index--;
            return;
        }
        else if ((((tmpfs_header_t*) dir->files[i])->type == 1) && (strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0)) {
            // tmpfs_delete_directory(((tmpfs_directory_t*) dir->files[i]), name);//we don't recursively search for file. only the stuff inside the current directory
        }
    }
    kprintf("tmpfs_delete_directory(): directory not found\n");
}

void tmpfs_delete_directory_no_orphan(tmpfs_directory_t* dir, char* name) {//we orphan the files ig. also, if we do end up not doing a root dir pointer, i'm not actually sure how you would delete it with this function since you don't have a parent directory to parse through
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (dir->files[i] == NULL) {continue;}
        //if the names are the same and if it isn't null and if its type is a directory
        if ((strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0) && (((tmpfs_header_t*) dir->files[i])->type == 1)) {//we cast to header and not file because it could be either a file or a directory
            for (int j = 0; j < TMPFS_MAX_FILES; j++) {
                if (((tmpfs_directory_t*) dir->files[i])->files[j] != NULL) {
                    //HERE remember to add recursive deletion since this only deletes the files in the current directory
                    kfree(((tmpfs_directory_t*) dir->files[i])->files[j]);
                }
            }
            kfree(dir->files[i]);
            dir->files[i] = NULL;
            dir->probably_next_free_entry_index--;
            return;
        }
        else if ((((tmpfs_header_t*) dir->files[i])->type == 1) && (strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0)) {
            tmpfs_delete_directory(((tmpfs_directory_t*) dir->files[i]), name);//we don't recursively search for file. only the stuff inside the current directory
        }
    }
    kprintf("tmpfs_delete_directory(): directory not found\n");
}

void tmpfs_list_files(tmpfs_directory_t* dir) {//remember that it's files and not file. also maybe make this list directoreis as well?
    kprintf("-%s-\n", dir->header.name);
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
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
            memcpy(new_file, file->data, file->size);
            kfree(file->data);
        }
        file->data = new_file;
        file->size = offset+size;
    }

    memcpy((uint64_t) file->data+offset, data, size);

}

size_t tmpfs_read_from_file(tmpfs_fd_t* file, void* data, uint64_t size, uint64_t offset) {//MUST INITIALIZE BUFFER TO FILL
    if (offset >= file->size) {
        return 1;
    }
    size_t count = file->size - offset;
    if (count > size) {
        count = size;
    }
    memcpy(data, (uint64_t)(file->data)+offset, count);
    return count;
}

tmpfs_fd_t* tmpfs_open(tmpfs_directory_t* dir, char* name, uint8_t mode) {
    tmpfs_fd_t* fd = kmalloc(sizeof(tmpfs_fd_t));
    tmpfs_file_t* file = (tmpfs_file_t*) tmpfs_lookup(dir, name);
    fd->data = file->data;
    fd->size = file->size;
    fd->mode = mode;
    fd->position = 0;
    //we always set the position to 0. since this returns a file descriptor, we leave it up to the user or other functions to modify the position
    return fd;
}

void tmpfs_close(tmpfs_fd_t* fd) {
    kfree(fd);
}


void* tmpfs_lookup(tmpfs_directory_t* dir, char* name) {
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
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
    tmpfs_header_t* temp = tmpfs_lookup(vnode->vnode_data, name);
    vnode_t* ret = tmpfs_link_vnode(temp, temp->type);
    return ret;
}

vnode_t* tmpfs_link_vnode(void* file_object, enum vtype type) {
    vnode_t* new_vnode = (vnode_t*) kmalloc_byte(sizeof(vnode_t));
    new_vnode->vnode_type = type;
    //do something about storing paths either here or inside the file object
    //remember to add reference count as well and maybe the rest of the members
    //remember to add ioctl as well
    vnode_ops_t* ops = kmalloc_byte(sizeof(vnode_ops_t));
    new_vnode->vnode_ops = ops;
    if (type == VDIR) {
        new_vnode->vnode_ops->vnode_create = vnode_tmpfs_create_file;
        new_vnode->vnode_ops->vnode_remove = tmpfs_delete_file;    
        new_vnode->vnode_ops->vnode_mkdir = tmpfs_create_directory;
        new_vnode->vnode_ops->vnode_rmdir = tmpfs_delete_directory;
        new_vnode->vnode_ops->vnode_rmdir_no_orphan = tmpfs_delete_directory_no_orphan;
        new_vnode->vnode_ops->vnode_lookup = vnode_tmpfs_lookup;
    }
    else if (type == VREG) {
        new_vnode->vnode_ops->vnode_rd = tmpfs_read_from_file;//MAYBE COMBINE READ AND WRITE???
        new_vnode->vnode_ops->vnode_wr = tmpfs_write_to_file;
    }

    // vnode_t* root_vnode = (vnode_t*) root_dir;
    new_vnode->vnode_vfsmountedhere = vfs_tmpfs;
    new_vnode->vnode_data = file_object;
    return new_vnode;
}

vnode_t* vnode_tmpfs_create_file(vnode_t* vnode, char* name, uint64_t size) {
    tmpfs_file_t* file = tmpfs_create_file((tmpfs_directory_t*) vnode->vnode_data, name, size);
    vnode_t* ret = tmpfs_link_vnode(file, VREG);
    return ret;
}