#ifndef EX1_H
#define EX1_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

Product_t* search_product_by_name(Product_t* products, const char* name);
Product_t* search_product_by_price(Product_t* products,  int price);
Product_t* search_product_by_id(Product_t* products,  unsigned id);

#ifdef __cplusplus
}
#endif
#endif