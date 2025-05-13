#ifndef COMMON_H
#define COMMON_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define NUM_PRODUCTS 100

typedef struct Product {
    unsigned id;
    char name [1024];
    int price; 
    struct Product* next;
} Product_t;


#ifdef __cplusplus
}
#endif
#endif