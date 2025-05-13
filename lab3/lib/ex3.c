#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ex3.h"
#include "common.h"



void bp_destroy_node(bp_tree_node_t *node)
{
    // ending condition
    if (node==NULL) return;

    // recursive call
     for (int i=0; i<=node->n; i++) {
        if (!node->is_leaf_node) bp_destroy_node(node->children[i]);
     }

    // free node
     free(node);
}

void bp_destroy_tree(bp_tree_t *tree)
{
    if (tree==NULL) return;
    bp_destroy_node(tree->root);
    free(tree);
}

bp_tree_t *bp_init_index()
{
    bp_tree_t* tree = (bp_tree_t*)malloc(sizeof(bp_tree_t));
    tree->root = NULL;
    tree->m = BP_TREE_ORDER;
    return tree;
}

Product_t *bp_get_product(bp_tree_t *tree, int key)
{
    if (tree==NULL) return NULL;

    // browse the tree nodes
    bp_tree_node_t *node = tree->root;
    while (node) {
        for (int i=0; i<node->n; i++) {
            if (node->keys[i]) {
                // leaf node: return product
                if (node->is_leaf_node) {
                    if (node->keys[i]==key) {
                        return node->products[i];
                    } else if (i == node->n-1) {
                        // last key, product not found
                        return NULL;
                    }
                // not leaf node: browse children
                } else {
                    if (node->keys[i] > key) {
                        node = node->children[i];
                        break;
                    } else if (i == node->n-1)
                    {
                        // get last child
                        node = node->children[i+1];
                    }
                    
                }
            } else {
                // node not full: get last child
                node = node->children[i];
                break;
            }
        }
    }

    // not found
    return NULL;
}

int insert_leaf(bp_tree_node_t *leaf, int key, Product_t *product)
{
    // browse keys
    bool inserted = false;
    Product_t *tmp_prod;
    int tmp_key = 0;
    for (int i=0; i<leaf->n; i++) {
        // insert new product, update key
        if (key < leaf->keys[i]) {
            inserted = true;
            tmp_prod = leaf->products[i];
            tmp_key = leaf->keys[i];
            leaf->products[i] = product;
            leaf->keys[i] = key;
        // move next products and keys
        } else if (inserted) {
            // next product is empty
            if (leaf->products[i]==NULL) {
                leaf->products[i] = tmp_prod;
                leaf->keys[i] = tmp_key;
                break;
            // next product is not empty
            } else {
                Product_t *tmp_prod2;
                int tmp_key2;
                tmp_prod2 = leaf->products[i];
                tmp_key2 = leaf->keys[i];
                leaf->products[i] = tmp_prod;
                leaf->keys[i] = tmp_key;
                tmp_prod = tmp_prod2;
                tmp_key = tmp_key2;
            }
        }
    }
    if (!inserted) {
        inserted = true;
        leaf->products[leaf->n] = product;
        leaf->keys[leaf->n] = key;
    }
    leaf->n++;
    return 0;
}

int insert_split(bp_tree_t *tree, bp_tree_node_t *node, int key, Product_t *product)
{
    bool split_leaf = !(!node->is_leaf_node && product==NULL);

    // gather all products and keys in order
    int k = 0;
    int all_keys[node->n+1];
    Product_t *all_products[node->n+1];
    int half = (node->n+1) / 2;
    for (int i=0; i<node->n; i++) {
        if (key < node->keys[i]) {
            // insert new product and new key
            all_keys[k] = key;
            if (split_leaf) all_products[k] = product;
            k++;
        }
        all_keys[k] = node->keys[i];
        if (split_leaf) all_products[k] = node->products[i];
        k++;
        if (i==node->n-1 && k!=node->n+1) {
            // new product and new key at last position
            all_keys[k] = key;
            if (split_leaf) all_products[k] = product;
        }
    }

    /* CHILDREN */


    // keep second half in child
    bp_tree_node_t* child = (bp_tree_node_t*)malloc(sizeof(bp_tree_node_t));
    if (split_leaf) {
        child->n = node->n + 1 - half;
        child->is_leaf_node = true;
    } else {
        child->n = half;
        child->is_leaf_node = false;
    }
    for (int i=0; i<tree->m; i++) {
        if (i<child->n) {
            if (split_leaf) {
                child->keys[i] = all_keys[half+i];
                child->products[i] = all_products[half+i];
            } else {
                child->keys[i] = all_keys[half+1+i];
            }
        } else {
            child->keys[i] = 0;
            child->products[i] = NULL;
        }
    }
    for (int i=0; i<=tree->m; i++) {
        child->children[i] = NULL;
        if (!split_leaf) {
            if (child->keys[i] && i<child->n) child->children[i] = node->children[half+i+1];
            else child->children[i] = node->children[node->n]->next;
        }
    }

    // clear node
    for (int i=0; i<node->n; i++) {
        node->keys[i] = 0;
        node->products[i] = NULL;
    }
    for (int i=0; i<=node->n; i++) {
        if (split_leaf || i > half) node->children[i] = NULL;
    }

    // keep first half in node
    node->n = half;
    if (split_leaf) node->is_leaf_node = true;
    else node->is_leaf_node = false;
    for (int i=0; i<half; i++) {
        node->keys[i] = all_keys[i];
        if (split_leaf) node->products[i] = all_products[i];
    }

    // link children
    node->next = child;
    child->next = NULL;
                                    /* TO DO: LINK LAST CHILD TO OTHER CHILD */

    /* PARENT */

    // 1. node has no parent (root)
    if (!node->parent) {
        // create parent
        bp_tree_node_t* parent = (bp_tree_node_t*)malloc(sizeof(bp_tree_node_t));
        parent->n = 1;
        parent->is_leaf_node = false;
        for (int i=0; i<tree->m; i++) {
            parent->keys[i] = 0;
            parent->products[i] = NULL;
        }
        for (int i=0; i<=tree->m; i++) {
            parent->children[i] = NULL;
        }
        parent->keys[0] = all_keys[half];
        parent->children[0] = node;
        parent->children[1] = child;
        parent->parent = NULL;
        parent->next = NULL;
        
        // link root to parent
        tree->root = parent;
        // link children to parent
        node->parent = parent;
        child->parent = parent;
        return 0;

    // 2. node has a parent
    } else {

        // parent not full
        if (node->parent->n < tree->m) {
            // add key and children
            node->parent->keys[node->parent->n] = all_keys[half];
            node->parent->children[node->parent->n+1] = child;
            child->parent = node->parent;
            node->parent->n++;
            return 0;

        // parent full
        } else {
             return insert_split(tree, node->parent, all_keys[half], NULL);
        }
    }
    return 1;
}

int bp_insert_product(bp_tree_t *tree, int key, Product_t *product)
{
    // Check empty case
    if (tree==NULL) return 1;
    if (tree->root == NULL) {
        bp_tree_node_t* node = (bp_tree_node_t*)malloc(sizeof(bp_tree_node_t));
        node->n = 1;
        node->is_leaf_node = true;
        node->keys[0] = key;
        node->products[0] = product;
        for (int i=1; i<tree->m; i++) {
            node->keys[i] = 0;
            node->products[i] = NULL;
        }
        node->parent = NULL;
        node->next = NULL;
        for (int i=0; i<=tree->m; i++) {
            node->children[i] = NULL;
        }
        tree->root = node;
        return 0;
    }

    // Check if product is already indexed
    if (bp_get_product(tree, key)) return 1;

    // Get the corresponding leaf node
    bp_tree_node_t *node = tree->root;
    while (node) {
        // browse non-leaf nodes
        if (!node->is_leaf_node) {
            for (int i=0; i<node->n; i++) {
                if (key < node->keys[i]) {
                    node = node->children[i];
                    break;
                } else if (i == node->n-1) {
                    // get last child
                    node = node->children[i+1];
                    break;
                }
            }
        // leaf found
        } else {
            break;
        }
    }

    // Insert
    if (node->n < tree->m) {
        // node is not full, simple insert
        return insert_leaf(node, key, product);
    // If there is split needed handle that (you may implement some helper functions then use it here.)
    } else {
        // node is full, handle split
        return insert_split(tree, node, key, product);
    }
}


// This is a helper function to printout the B+tree

void bp_print_all(bp_tree_t *tree) {
    printMWayTree(tree->root, 0);
}

void printMWayTree(bp_tree_node_t *root, int level)
{
    if (root == NULL)
        return;

    for (int i = 0; i < level; i++)
        printf("  ");
    for (int i = 0; i < root->n; i++)
        printf("%d ", root->keys[i]);

    printf("\n");
    for (int i = 0; i < root->n + 1; i++)
    {
        printMWayTree(root->children[i], level + 1);
    }
}