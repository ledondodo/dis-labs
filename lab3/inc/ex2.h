#ifndef EX2_H
#define EX2_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

typedef struct hash_node {
    Product_t* product;
    struct hash_node* next;
} hash_node_t;

typedef struct hash_index {
    int size;
    hash_node_t** buckets;
} hash_index_t;

unsigned hash(const char *str);

hash_index_t* h_init_index(int size);
void h_destroy_index(hash_index_t* index);
void h_insert_product(hash_index_t* index, Product_t* product);
int h_delete_product(hash_index_t* index, const char* name);
Product_t* h_get_product(hash_index_t* index, const char* name);

#ifdef __cplusplus
}
#endif
#endif
