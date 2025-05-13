#ifndef EX3_H
#define EX3_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

#define BP_TREE_ORDER 4

// B+ tree node
typedef struct bp_tree_node {
    int n;                                                      // Number of keys in the node
    bool is_leaf_node;                                           // Is this node a leaf?
    
    int keys[BP_TREE_ORDER];                                    // Keys
    Product_t* products[BP_TREE_ORDER];                         // Pointers to products

    struct bp_tree_node* children[BP_TREE_ORDER + 1];           // Pointers to children
    struct bp_tree_node* parent;
    
    struct bp_tree_node* next;
} bp_tree_node_t;

typedef struct bp_tree {
    bp_tree_node_t *root;
    int m;
} bp_tree_t;

bp_tree_t* bp_init_index(void);
Product_t * bp_get_product(bp_tree_t *tree, int key);
int insert_leaf(bp_tree_node_t *leaf, int key, Product_t *product);
int insert_split(bp_tree_t *tree, bp_tree_node_t *node, int key, Product_t *product);
int bp_insert_product(bp_tree_t *tree, int key, Product_t *product);
void bp_print_all(bp_tree_t *tree);
void printMWayTree(bp_tree_node_t *root, int level);
void bp_destroy_tree(bp_tree_t *tree);
void bp_destroy_node(bp_tree_node_t *node);

#ifdef __cplusplus
}
#endif
#endif