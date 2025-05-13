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

const size_t k_ntests = 2;


TEST(unittest, check_linked_list)
{
  hash_index_t* index = h_init_index(1);
  
  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;
  Product_t* product2 = (Product_t*) malloc(sizeof(Product_t));
  product2->id = 2; strcpy(product2->name, "product2"); product2->price=5; product2->next=nullptr;
  Product_t* product3 = (Product_t*) malloc(sizeof(Product_t));
  product3->id = 3; strcpy(product3->name, "product3"); product3->price=5; product3->next=nullptr;

  h_insert_product(index, product1);
  h_insert_product(index, product2);
  h_insert_product(index, product3);

  ASSERT_EQ(product1, index->buckets[0]->product);
  ASSERT_EQ(product2, index->buckets[0]->next->product);
  ASSERT_EQ(product3, index->buckets[0]->next->next->product);

  h_destroy_index(index);
}

TEST(unittest, check_linked_list_delete)
{
  hash_index_t* index = h_init_index(1);
  
  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;
  Product_t* product2 = (Product_t*) malloc(sizeof(Product_t));
  product2->id = 2; strcpy(product2->name, "product2"); product2->price=5; product2->next=nullptr;
  Product_t* product3 = (Product_t*) malloc(sizeof(Product_t));
  product3->id = 3; strcpy(product3->name, "product3"); product3->price=5; product3->next=nullptr;

  h_insert_product(index, product1);
  h_insert_product(index, product2);
  h_insert_product(index, product3);
  h_delete_product(index, product2->name);

  ASSERT_EQ(product1, index->buckets[0]->product);
  ASSERT_EQ(product3, index->buckets[0]->next->product);

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
