#include "replacer.h"
#include <assert.h>
#include <stdio.h>



int InitReplacer(size_t pool_size, size_t k, Replacer **r){
    if (!pool_size || !k || *r) return -EREPLACER;
    *r = (Replacer*) malloc(sizeof(Replacer));
    memset(*r, 0, sizeof(**r));
    (*r)->replacer_size_ = pool_size;
    (*r)->k_ = k;
    return 0;
}

int StopReplacer(Replacer *r){
    if (!r) return -EREPLACER;
    KLRUListNode *node = r->list_;
    KLRUListNode *next_node = NULL;
    while (node){
        next_node = node->next_;
        page_id pid = node->id_;
        ReplacerHashNode *hash_node = NULL;
        HASH_FIND_INT(r->node_store_, &pid, hash_node);
        assert(hash_node != NULL);
        HistoryNode *hist_node = hash_node->lru_node_.history_;
        HistoryNode *tmp = NULL;
        while (hist_node){
            tmp = hist_node -> next_;
            free(hist_node);
            hist_node = tmp;
        }
        HASH_DEL(r->node_store_, hash_node);
        free(hash_node);
        free(node);
        node = next_node;
    }
    free(r);
    return 0;
}

bool Evict(page_id *id, Replacer *r){
    if (r==NULL) return false;

    /* Traverse the list_ to find the evictable node */

    // search best candidate
    KLRUListNode *node = r->list_;
    KLRUListNode *best_node = NULL;
    while (node) {
        page_id pid = node->id_;
        // get hash node
        ReplacerHashNode *hash_node = NULL;
        HASH_FIND_INT(r->node_store_, &pid, hash_node);
        if (hash_node == NULL) return false;
        // update best parameters: 1st node in the list that is evictable
        if (hash_node->lru_node_.is_evictable_) {
            best_node = node;
            break;
        }
        node = node->next_;
    }
    // best node not found
    if (best_node==NULL) return false;
    
    /* Evict it from both node_store_ and list_ */
    
    // best candidate
    page_id best_pid = best_node->id_;
    ReplacerHashNode *hash_node = NULL;
    HASH_FIND_INT(r->node_store_, &best_pid, hash_node);
    
    // evict node
    if (best_node==NULL) {
        return false;
    } else {
        *id = best_node->id_;
        Remove(*id, r);
        return true;
    }
}

void RecordAccess(page_id id, Replacer *r){
    // new time stamp
    r->current_ts_++;

    /* Look for the node from node_store_ */
    ReplacerHashNode *new_hash_node = NULL;
    HASH_FIND_INT(r->node_store_, &id, new_hash_node);

    if (new_hash_node!=NULL) {
        /* If found, insert a new history to the header of the history list */
        HistoryNode *new_history = (HistoryNode *)malloc(sizeof(HistoryNode));
        new_history->ts_ = r->current_ts_;
        new_history->next_ = new_hash_node->lru_node_.history_;
        new_hash_node->lru_node_.history_ = new_history;
        new_hash_node->lru_node_.history_size_++;

        /* If found, remove the KLRUListNode from the list_ */
        KLRUListNode *node = r->list_;
        KLRUListNode *prev = NULL;
        while (node) {
            if (node->id_ == id) {
                if (prev == NULL) {
                    // skip first element of the list
                    r->list_ = node->next_;
                } else {
                    // skip nth element of the list
                    prev->next_ = node->next_;
                }
                break;
            }
            prev = node;
            node = node->next_;
        }
    } else {
        /* If not found, insert a new hash_node_ to the node_store_ */
        new_hash_node = (ReplacerHashNode*)malloc(sizeof(ReplacerHashNode));
        new_hash_node->id_ = id;

        // first history entry
        HistoryNode *new_history = (HistoryNode *)malloc(sizeof(HistoryNode));
        new_history->ts_ = r->current_ts_;
        new_history->next_ = NULL;
        new_hash_node->lru_node_.history_ = new_history;
        new_hash_node->lru_node_.history_size_ = 1;
        new_hash_node->lru_node_.is_evictable_ = true;
        if (r->k_ > 1) {
            new_hash_node->lru_node_.k_ = INT_FAST32_MAX; // +inf
        } else {
            new_hash_node->lru_node_.k_ = 0;
        }
        new_hash_node->lru_node_.pid_ = id;
        HASH_ADD_INT(r->node_store_, id_, new_hash_node);
        r->current_size_++;
    }

    /* For both cases, count the back-k distance of the page */
    // for each node in the list:
    KLRUListNode *node = r->list_;
    while (node) {
        size_t pid = node->id_;
        // get hash node
        ReplacerHashNode *hash_node = NULL;
        HASH_FIND_INT(r->node_store_, &pid, hash_node);
        // compute kth time stamp
        if (hash_node->lru_node_.k_<INT_FAST32_MAX) {
            // 1. node was not accessed at this time stamp, increment by 1
            hash_node->lru_node_.k_++;
        }
        node = node->next_;
    }
    
    // 2. node accessed at this time stamp, recompute k
    HistoryNode *new_node_history = new_hash_node->lru_node_.history_;
    size_t count_k = 0;
    size_t new_ts_k = 0;
    while (new_node_history) {
        count_k++;
        if (count_k==r->k_) {
            // k_dist = current_ts - node_kth_ts
            new_ts_k = new_node_history->ts_;
            new_hash_node->lru_node_.k_ = r->current_ts_ - new_ts_k;
            break;
        }
        if (new_node_history->next_==NULL) new_ts_k = new_node_history->ts_;
        new_node_history = new_node_history->next_;
    }
    if (count_k < r->k_) {
        new_hash_node->lru_node_.k_ = INT_FAST32_MAX; // +inf
    }
        

    /* Insert the page (pid) to the proper place of list_ */
    KLRUListNode *new_node = (KLRUListNode *)malloc(sizeof(KLRUListNode));
    new_node->id_ = id;
    new_node->next_ = NULL;
    node = r->list_;
    if (node == NULL) {
        r->list_ = new_node;
    } else {
        // go through the list
        KLRUListNode *prev = NULL;
        bool first_element = true;
        while (node) {
            size_t pid = node->id_;
            // get hash node
            ReplacerHashNode *hash_node = NULL;
            HASH_FIND_INT(r->node_store_, &pid, hash_node);
            // insert if k > node.k
            if (new_hash_node->lru_node_.k_ >= hash_node->lru_node_.k_) {

                // in case node.k = +inf, compare timestamps_k
                if (hash_node->lru_node_.k_ == INT_FAST32_MAX) {
                    HistoryNode *node_history = hash_node->lru_node_.history_;
                    size_t count_hist = 0;
                    size_t ts_k = 0;
                    while (node_history) {
                        count_hist++;
                        if (count_hist==r->k_) {
                            // store timestamp k
                            ts_k = node_history->ts_;
                            break;
                        }
                        if (node_history->next_ == NULL) ts_k = node_history->ts_;
                        node_history = node_history->next_;
                    }
                    if (new_ts_k < ts_k) {
                        new_node->next_ = node;
                        if (first_element) r->list_ = new_node;
                        else prev->next_ = new_node;
                        break;
                    } else if (node->next_==NULL) {
                        node->next_ = new_node;
                        break;
                    }

                } else {
                    new_node->next_ = node;
                    if (first_element) r->list_ = new_node;
                    else prev->next_ = new_node;
                    break;
                }
            } else if (node->next_==NULL) {
                node->next_ = new_node;
                break;
            }
            
            // next element of the list
            first_element = false;
            prev = node;
            node = node->next_;
        }
    }
    return;
}

void SetEvictable(page_id id, bool set_evictable, Replacer *r){
    // search node with page id
    page_id pid = id;
    ReplacerHashNode *hash_node = NULL;
    HASH_FIND_INT(r->node_store_, &pid, hash_node);
    if (hash_node == NULL) return; // node not found
    // node evictablility
    bool was_evictable = hash_node->lru_node_.is_evictable_;
    hash_node->lru_node_.is_evictable_ = set_evictable;

    // adjust replacer size
    if (was_evictable == set_evictable) {
        return;
    } else {
        if (set_evictable) {
            r->current_size_++;
        } else {
            r->current_size_--;
        }
        return;
    }
}

void Remove(page_id id, Replacer *r){
    // find node with id
    ReplacerHashNode *hash_node = NULL;
    HASH_FIND_INT(r->node_store_, &id, hash_node);
    if (hash_node==NULL) return;
    // abord if node is not evictable
    if (!hash_node->lru_node_.is_evictable_) return;
    // delete history
    HistoryNode *hist_node = hash_node->lru_node_.history_;
    HistoryNode *tmp = NULL;
    while (hist_node){
        tmp = hist_node -> next_;
        free(hist_node);
        hist_node = tmp;
    }
    // delete in list
    KLRUListNode *node = r->list_;
    KLRUListNode *prev = NULL;
    while (node) {
        if (node->id_ == id) {
            if (prev == NULL) {
                // skip first element of the list
                r->list_ = node->next_;
            } else {
                // skip nth element of the list
                prev->next_ = node->next_;
            }
            break;
        }
        prev = node;
        node = node->next_;
    }
    // delete node
    HASH_DEL(r->node_store_, hash_node);
    free(hash_node);
    // update current size
    r->current_size_--;
    return;
}

size_t ReplacerSize(Replacer *r) {
    return r->current_size_;
}
