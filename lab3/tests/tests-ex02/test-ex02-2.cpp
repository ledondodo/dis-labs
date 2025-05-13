#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "unistd.h"
extern "C"
{
    #include "ex2.h"
    #include "common.h"
}

const size_t k_ntests = 4;

TEST(unittest, insert_single_product)
{
  hash_index_t* index = h_init_index(5);
  assert(index != nullptr);

  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;

  h_insert_product(index, product1);
  Product_t* p = h_get_product(index, "product1");
  ASSERT_EQ(p, product1);

  h_destroy_index(index);
}

TEST(unittest, insert_multiple_products)
{
  hash_index_t* index = h_init_index(5);
  assert(index != nullptr);

  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;
  Product_t* product2 = (Product_t*) malloc(sizeof(Product_t));
  product2->id = 2; strcpy(product2->name, "product2"); product2->price=5; product2->next=nullptr;
  Product_t* product3 = (Product_t*) malloc(sizeof(Product_t));
  product3->id = 3; strcpy(product3->name, "product3"); product3->price=5; product3->next=nullptr;
  Product_t* product4 = (Product_t*) malloc(sizeof(Product_t));
  product4->id = 4; strcpy(product4->name, "product4"); product4->price=5; product4->next=nullptr;

  h_insert_product(index, product1);
  h_insert_product(index, product2);
  h_insert_product(index, product3);
  h_insert_product(index, product4);
  
  Product_t* p = h_get_product(index, "product1");
  ASSERT_EQ(p, product1);
  Product_t* p2 = h_get_product(index, "product2");
  ASSERT_EQ(p2, product2);
  Product_t* p3 = h_get_product(index, "product3");
  ASSERT_EQ(p3, product3);
  Product_t* p4 = h_get_product(index, "product4");
  ASSERT_EQ(p4, product4);
  
  h_destroy_index(index);
}

TEST(unittest, remove_test)
{
  hash_index_t* index = h_init_index(5);
  assert(index != nullptr);

  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;

  h_insert_product(index, product1);
  Product_t* p = h_get_product(index, "product1");
  ASSERT_EQ(p, product1);

  h_delete_product(index, product1->name);
  Product_t* p2 = h_get_product(index, "product1");
  ASSERT_EQ(p2, nullptr);

  h_destroy_index(index);
}

TEST(unittest, remove_test2)
{
  hash_index_t* index = h_init_index(5);
  assert(index != nullptr);

  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;

  h_insert_product(index, product1);

  int ret = h_delete_product(index, "product1");
  ASSERT_EQ(ret, 0);
  ret = h_delete_product(index, "product1");
  ASSERT_EQ(ret, 1);

  h_destroy_index(index);
}


int
main (int argc, char **argv)
{  
  for (size_t i = 0; i < k_ntests; i++){
    std::string filename = "test";
    filename += ('1' + i);
    if (access(filename.c_str(), F_OK) == 0){
      remove(filename.c_str());
    }
  }
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}
