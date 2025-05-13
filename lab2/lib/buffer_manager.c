#include "buffer_manager.h"
#include "replacer.h"
#include "bp_error.h"
#include <assert.h>


int InitBufferManager(size_t pool_size, size_t k,  StorageManager *sm, BufferManager **bm){
    if (*bm || pool_size == 0 || sm == NULL) return -EBUFFERMANAGER;
    *bm = malloc(sizeof(BufferManager));
    memset(*bm, 0, sizeof(BufferManager));
    (*bm)->next_block_id_ = 0;
    (*bm)->pool_size_ = pool_size;
    (*bm)->pages_ = malloc(sizeof(Page) * pool_size);
    for (size_t i = 0; i < pool_size; i++){
        (*bm)->pages_[i].pin_count_ = 0;
        (*bm)->pages_[i].is_dirty_ = 0;
        (*bm)->pages_[i].block_id_ = INVALID_BLOCK_ID;
        (*bm)->pages_[i].data_ = malloc(sizeof(char) * PAGE_SIZE);
    }
    (*bm)->storage_manager_ = sm;
    (*bm)->replacer_ = NULL;
    int ret = InitReplacer(pool_size, k, &((*bm)->replacer_));
    if (ret) {
        for (size_t i = 0; i < pool_size; i++){
            free((*bm)->pages_[i].data_);
        }
        free((*bm)->pages_);
        free(bm);
        return -EBUFFERMANAGER;
    }
    FreeListNode *node = malloc(sizeof(FreeListNode));
    node->page_id_ = 0;
    node->next_ = NULL;
    (*bm)->free_list_  = node;
    for (size_t i = 1; i < pool_size; i++){
        FreeListNode *new_node = malloc(sizeof(FreeListNode));
        new_node->page_id_ = i;
        new_node->next_ = NULL;
        node->next_ = new_node;
        node = new_node;
    }
    return 0;
}

int StopBufferManager(BufferManager *bm){
    if (!bm) return -EBUFFERMANAGER;
    // Storage Manager will be handled separately.
    // Delete the page_table_ first    
    PageTableNode *current_node, *tmp;

    HASH_ITER(hh, bm->page_table_, current_node, tmp) {
        HASH_DEL(bm->page_table_, current_node);
        free(current_node); 
    }
    FreeListNode *node = bm->free_list_;
    FreeListNode *next_node;
    while(node) {
        next_node = node->next_;
        free(node);
        node = next_node;
    }
    for(size_t i = 0; i < bm->pool_size_; i++){
        free(bm->pages_[i].data_);
    }
    free(bm->pages_);
    StopReplacer(bm->replacer_);
    free(bm);
    return 0;
}

Page* FetchPage(block_id id, BufferManager *bm){
    /* Look for the PageTableNode from the page_table_ */
    PageTableNode *page_node = NULL;
    HASH_FIND_INT(bm->page_table_, &id, page_node);

    if (page_node!=NULL) {
        /* If found, pin the page, record the access and set it inevictable */
        page_id pid = page_node->pid_;
        Page *page = &bm->pages_[pid];
        page->pin_count_++;
        SetEvictable(pid, false, bm->replacer_);
        return page;
    } else {
        /* 
        * If not found, replace a page or find a free page.
        * Read the page in, record the access and set it inevictable
        */
        block_id tmp_bid = id;
        Page *page = NewPage(&tmp_bid, bm);
        if (page == NULL) {
            return NULL;
        } else {
            if (ReadPage(id, page->data_, bm->storage_manager_)==0) {
                page->block_id_ = id;
                // delete hash for wrong bid
                PageTableNode *pt_node = NULL;
                HASH_FIND_INT(bm->page_table_, &tmp_bid, pt_node);
                page_id pid = pt_node->pid_;
                HASH_DEL(bm->page_table_, pt_node);
                free(pt_node);
                // change buffer manager next block, to be consistent
                bm->next_block_id_--;
                // hash to the right block, the one we fetch on
                PageTableNode *new_page = malloc(sizeof(PageTableNode));
                new_page->bid_ = id;
                new_page->pid_ = pid;
                HASH_ADD_INT(bm->page_table_, bid_, new_page);
                return page;
            } else {
                return NULL;
            }
        }
    }
}

Page* NewPage(block_id *id, BufferManager *bm) {
    /* Replace a page or find a free page. */
    Page *new_page = NULL;
    page_id pid;
    FreeListNode *free_page = bm->free_list_;
    if (free_page != NULL) {
        // get first free page
        pid = free_page->page_id_;
    } else {
        // find which page to replace with evict
        if (Evict(&pid, bm->replacer_)) {
            Page *page = &bm->pages_[pid];
            if (page->pin_count_>0) return NULL;
            if (!DeletePage(page->block_id_, bm)) return NULL;
            if (bm->free_list_ != NULL) {
                pid = bm->free_list_->page_id_;
            }
        } else {
            return NULL;
        }
    }
    new_page = &bm->pages_[pid];
    bm->free_list_ = bm->free_list_->next_;
    
    /* Allocate the block */
    *id = AllocateBlock(bm);

    /* Allocate a new PageTableNode */
    // new page
    PageTableNode *new_node = malloc(sizeof(PageTableNode));
    new_node->bid_ = *id;
    new_node->pid_ = pid;
    // add to page table
    HASH_ADD_INT(bm->page_table_, bid_, new_node);

    /* Record the access and mark it as evictable */
    new_page->pin_count_ = 1;
    new_page->block_id_ = *id;
    new_page->is_dirty_ = false;
    memset(new_page->data_, 0, PAGE_SIZE);
    RecordAccess(pid, bm->replacer_);
    SetEvictable(pid, false, bm->replacer_);

    return new_page;
}

bool UnpinPage(block_id id, bool is_dirty, BufferManager *bm){
    // find node
    PageTableNode *pt_node = NULL;
    HASH_FIND_INT(bm->page_table_, &id, pt_node);
    if (pt_node==NULL) return false;
    // find page
    page_id pid = pt_node->pid_;
    Page *page = &bm->pages_[pid];
    // unpin
    if (page->pin_count_==0) return false;
    else page->pin_count_--;
    if (page->pin_count_==0) SetEvictable(pid, true, bm->replacer_);
    // dirty bit
    if (is_dirty) page->is_dirty_=true;

    return true;
}

bool FlushPage(block_id id, BufferManager *bm) {
    // find node
    PageTableNode *pt_node = NULL;
    HASH_FIND_INT(bm->page_table_, &id, pt_node);
    if (pt_node==NULL) return false; // not found
    // find page
    page_id pid = pt_node->pid_;
    Page *page = &bm->pages_[pid];
    // write back
    WritePage(id, page->data_, bm->storage_manager_);
    page->is_dirty_ = false;
    return true;
}

bool DeletePage(block_id id, BufferManager *bm) {
    // find node
    PageTableNode *pt_node = NULL;
    HASH_FIND_INT(bm->page_table_, &id, pt_node);
    if (pt_node==NULL) return true; // not in the buffer
    // find page
    page_id pid = pt_node->pid_;
    Page *page = &bm->pages_[pid];
    // page is pinned and cannot be deleted
    if (page->pin_count_>0) return false;

    // flush if dirty
    if (page->is_dirty_) {
        if (!FlushPage(id, bm)) return false;
    }

    // delete from buffer
    HASH_DEL(bm->page_table_, pt_node);
    free(pt_node);
    // stop tracking the page in the replacer
    SetEvictable(pid, true, bm->replacer_);

    // add it back to the free list
    FreeListNode *free_page = bm->free_list_;
    FreeListNode *new_free_page = malloc(sizeof(FreeListNode));
    new_free_page->page_id_ = pid;
    new_free_page->next_ = free_page;
    bm->free_list_ = new_free_page;
    // reset page
    memset(page->data_, 0, PAGE_SIZE);
    page->block_id_ = INVALID_BLOCK_ID;
    page->is_dirty_ = false;
    page->pin_count_ = 0;
    
    return true;
}

block_id AllocateBlock(BufferManager *bm) {
    return bm->next_block_id_++;
}
