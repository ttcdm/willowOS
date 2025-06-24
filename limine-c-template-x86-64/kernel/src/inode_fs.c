#include <inode_fs.h>

void init_inode_fs() {
    // create_file();
    inode_t* root_inode = (inode_t*) kmalloc_byte(sizeof(inode_t));
    root_inode->header.permissions[0] = 'r';
    root_inode->header.permissions[1] = 'w';
    root_inode->header.permissions[2] = 'x';
    root_inode->header.user_id = 0;
    root_inode->header.group_id = 0;
    strncpy(root_inode->header.name, "root", 5);
    root_inode->header.type = 1;
    root_inode->header.size = 32768;//placeholder value. chatgpt said that it was supposed to reflect the total size of all the files inside the directory combined
    root_inode->header.io_block_size = 4096;
    root_inode->header.allocated_blocks = 1024;//idk

}

//these function names are just taken from the paper
inode_t* create_file(inode_t* directory_inode, char* name, uint64_t size) {
    inode_t* file_inode = (inode_t*) kmalloc_byte(sizeof(inode_t));
    file_inode->header.permissions[0] = 'r';
    file_inode->header.permissions[1] = 'w';
    file_inode->header.permissions[2] = 'x';
    file_inode->header.user_id = 0;
    file_inode->header.group_id = 0;
    strncpy(file_inode->header.name, name, kstrlen(name)+1);
    file_inode->header.type = 1;
    file_inode->header.size = size;
    file_inode->header.io_block_size = 4096;
    file_inode->header.allocated_blocks = 1024;

    directory_inode->next_free_entry++; directory_inode->next_free_entry--;//to make gcc happy about unused variable

    //INCOMPLETE

    return file_inode;
}



void open_file() {
    return;
}

void read_file() {
    return;
}

void write_file() {
    return;
}

void append_file() {
    return;
}

void close_file() {
    return;
}

void delete_file() {
    return;
}

void list_files() {
    return;
}

void list_open_files() {//it was in the paper but idk if it's necessary
    return;
}

void unmount() {//it was also in the paper but idk if it's necessary
    return;
}