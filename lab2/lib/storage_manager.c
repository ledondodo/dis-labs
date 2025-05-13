#include "storage_manager.h"
#include "bp_error.h"

#include "stdlib.h"
#include <string.h>
#include <assert.h>

int InitStorageManager(const char* filename, StorageManager **storage_manager){
    // Don't initialize it twice
    if (*storage_manager) return -ESTORAGEMANAGER;
    *storage_manager = malloc(sizeof(StorageManager));
    FILE* fptr = NULL;
    fptr = fopen(filename, "r+");

    if (fptr == NULL){
        fptr = fopen(filename, "w+");
        if (fptr == NULL) return -ESTORAGEMANAGER;
    }

    (*storage_manager)->file_ptr_ = fptr;
    (*storage_manager)->filename_ = malloc(strlen(filename) + 1);
    strcpy((*storage_manager)->filename_, filename); 
    return 0;
}

int StopStorageManager(StorageManager *storage_manager){
    if (storage_manager){
        int ret = fclose(storage_manager->file_ptr_);
        free(storage_manager->filename_);
        free(storage_manager);
        return ret;
    }
    return -ESTORAGEMANAGER;
}

int WritePage(block_id id, const char* page_data, StorageManager *storage_manager){
    // check formats
    if (storage_manager == NULL) return -ESTORAGEMANAGER;
    if (id<0 || page_data==NULL) return -ESTORAGEWRITE;

    // seek at file block location
    long offset = id*PAGE_SIZE; // compute offset
    int whence = SEEK_SET;      // position at start of the page
    if (fseek(storage_manager->file_ptr_, offset, whence) != 0) return -ESTORAGEWRITE;  // fseek fails

    // write at file block location
    if (fwrite(page_data, sizeof(char), PAGE_SIZE, storage_manager->file_ptr_) != PAGE_SIZE) {
        return -ESTORAGEWRITE; // fwrite fails
    }

    return 0;
}

int ReadPage(block_id id, char* page_data, StorageManager *storage_manager){
    // check formats
    if (storage_manager == NULL) return -ESTORAGEMANAGER;
    if (id<0 && page_data==NULL) return -ESTORAGEREAD;

    // check offset (max: end of file)
    int whence = SEEK_END; // position at end of the page
    if (fseek(storage_manager->file_ptr_, 0, whence) != 0) return -ESTORAGEREAD; // fseek fails
    long end_file = ftell(storage_manager->file_ptr_);
    if (end_file < 0) return -ESTORAGEREAD; // ftell fails
    long offset = id*PAGE_SIZE; // compute offset
    if (offset > end_file) return -ESTORAGEREAD; // wrong offset

    // read at file block location
    whence = SEEK_SET; // position at start of the page
    if (fseek(storage_manager->file_ptr_, offset, whence) != 0) return -ESTORAGEREAD; // fseek fails
    size_t items_read = fread(page_data, sizeof(char), PAGE_SIZE, storage_manager->file_ptr_);
    if (items_read == 0) return -ESTORAGEREAD; // fread fails

    // fill end of file with 0
    memset((void *)(page_data + items_read), 0, PAGE_SIZE - items_read);

    return 0;
}
