#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ex1.h"
#include "common.h"

Product_t *search_product_by_name(Product_t *product_list, const char *name)
{
    Product_t *product = product_list;
    while (product) {
        // compare names
        if (!strcmp(product->name, name)) {
            return product;
        } else {
            // next product
            product = product->next;
        }
    }
    // no product found
    return NULL;
}

Product_t *search_product_by_price(Product_t * product_list, int price)
{
    Product_t *product = product_list;
    while (product) {
        // compare prices
        if (product->price == price) {
            return product;
        } else {
            // next product
            product = product->next;
        }
    }
    // no product found
    return NULL;
}

Product_t *search_product_by_id(Product_t * product_list, unsigned id)
{
    Product_t *product = product_list;
    while (product) {
        // compare id
        if (product->id == id) {
            return product;
        } else {
            // next product
            product = product->next;
        }
    }
    // no product found
    return NULL;
}
