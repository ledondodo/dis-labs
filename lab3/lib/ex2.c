#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ex2.h"
#include "common.h"

// djb2
unsigned hash(const char *str)
{
    unsigned h = 5381;
    unsigned c = 0;
    while ((c = *str++)) {
        h = ((h << 5) + h) + c;
    }
    return h;
}

hash_index_t *h_init_index(int size)
{
    if (size==0) return NULL;

    // allocate space for hash and buckets
    hash_index_t* hash = (hash_index_t*)malloc(sizeof(hash_index_t));
    hash->size = size;
    hash->buckets = (hash_node_t**)malloc(size * sizeof(hash_node_t*));
    // initialize buckets to NULL
    for (int i = 0; i < size; ++i) {
        hash->buckets[i] = NULL;
    }
    return hash;
}

void h_destroy_index(hash_index_t *index)
{
    if (index == NULL) return;

    // free each entry
    for (int i = 0; i < index->size; ++i) {
        hash_node_t* bucket = index->buckets[i];
        while (bucket) {
            hash_node_t* tmp = bucket;
            bucket = bucket->next;
            free(tmp->product);
            free(tmp);
        }
    }

    // free the index
    free(index->buckets);
    free(index);

}

Product_t *h_get_product(hash_index_t *index, const char *name)
{
    // bucket id
    unsigned b = hash(name) % index->size;
    hash_node_t *bucket = index->buckets[b];
    if (!bucket) return NULL;

    // search by name in bucket
    while (bucket) {    
        // name found
        if (!strcmp(bucket->product->name, name)) return bucket->product;
        bucket = bucket->next;
    }
    return NULL;
}

void h_insert_product(hash_index_t *index, Product_t *product)
{
    // node to insert
    hash_node_t *new_node = (hash_node_t *)malloc(sizeof(hash_node_t));
    new_node->product = product;
    new_node->next = NULL;
    
    // bucket id
    unsigned b = hash(product->name) % index->size;
    hash_node_t *node = index->buckets[b];

    // insert product
    if (!node) {
        // bucket is empty
        index->buckets[b] = new_node;
    } else {
        while (node) {
            // add to last node
            if (!node->next) {
                node->next = new_node;
                break;
            }
            node = node->next;
        }
    }
}

int h_delete_product(hash_index_t *index, const char *name)
{
    // bucket id
    unsigned b = hash(name) % index->size;
    hash_node_t *bucket = index->buckets[b];

    // search by name in bucket
    hash_node_t *prev = NULL;
    while (bucket) {
        if (!strcmp(bucket->product->name, name)) {
            // delete product, and link previous node to next node
            hash_node_t *next_node = bucket->next;
            free(bucket->product);
            if (prev) {
                prev->next = next_node;
            } else {
                // first element
                index->buckets[b] = next_node;
            }
            return 0; // successful
        }
        prev = bucket;
        bucket = bucket->next;
    }
    return 1; // unsuccessful
}
