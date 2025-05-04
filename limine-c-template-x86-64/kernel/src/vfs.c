#include <vfs.h>

void init_vfs() {
    init_tmpfs();
}
void init_tmpfs() {
    //we load stuff from ustar and we just parse it and create our own custom filesystem
    tmpfs_directory_t* root_dir_pointer = (tmpfs_directory_t*) kmalloc_byte(sizeof(tmpfs_directory_t));
    tmpfs_directory_t* root_dir = tmpfs_create_directory(root_dir_pointer, "/");//not sure if i should actually do it with a root dir pointer
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
    tmpfs_file_t* test_dir_2 = tmpfs_create_directory(test_dir_1, "test dir 2");
    tmpfs_list_files(test_dir_1);
    tmpfs_create_file(test_dir_2, "test file 2", 1000);
    tmpfs_delete_directory(test_dir_1, "test dir 2");
    tmpfs_list_files(test_dir_1);
    for (int i = 0; i < 3; i++) {
        // tmpfs_create_file(test_dir_1, "test file", 4096);
    }
    tmpfs_list_files(test_dir_1);
    tmpfs_delete_file(root_dir, "test file 2");//can't be found because it's orphaned
    tmpfs_delete_file(root_dir, "test file 1");
    tmpfs_delete_file(root_dir, "test file 1");

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
    tmpfs_file_t* new_file = (tmpfs_file_t*) kmalloc_byte(size);
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
    return new_dir;
}

void tmpfs_delete_file(tmpfs_directory_t* dir, char* name) {//recursive search
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (dir->files[i] == NULL) {continue;}//can't put it in the if statement below because strcmp runs first so if it is null it'll page fault
        //if the names are the same and if it isn't null and if its type is a file
        if ((strcmp(((tmpfs_header_t*) dir->files[i])->name, name) == 0) && (((tmpfs_header_t*) dir->files[i])->type == 0)) {//we cast to header and not file because it could be either a file or a directory
            kfree(dir->files[i]);
            dir->files[i] = NULL;
            dir->probably_next_free_entry_index--;
            return;
        }
        else if ((((tmpfs_header_t*) dir->files[i])->type == 1)) {
            tmpfs_delete_file(((tmpfs_directory_t*) dir->files[i]), name);
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
            tmpfs_delete_file(((tmpfs_directory_t*) dir->files[i]), name);
        }
    }
    kprintf("tmpfs_delete_directory(): directory not found\n");
}
void tmpfs_list_files(tmpfs_directory_t* dir) {//remember that it's files and not file. also maybe make this list directoreis as well?
    kprintf("---\n");
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (dir->files[i] != NULL) {
            //remember to add file size as well
            kprintf("%s type:%d\n", (((tmpfs_header_t*) dir->files[i])->name), (((tmpfs_header_t*) dir->files[i])->type));//you can also get the string of the type by using an inline if block with the ? operator
        }
    }
    kprintf("---\n");
}
void tmpfs_write_to_file(tmpfs_file_t* file, char* msg, uint64_t size) {}//these should probably support fopen fseek ftell and such
void tmpfs_read_from_file(tmpfs_file_t* file, char* msg, uint64_t size) {}


void fopen() {}
void fread() {}
void fwrite() {}
void fclose() {}
void fstat() {}
void fseek() {}
void ftell() {}