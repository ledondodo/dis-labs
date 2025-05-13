#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "unistd.h"
extern "C"
{
    #include "ex3.h"
    #include "common.h"
}

size_t k_ntests = 3;

TEST(unittest, get_simple)
{
  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);

  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;
  Product_t* product2 = (Product_t*) malloc(sizeof(Product_t));
  product2->id = 2; strcpy(product2->name, "product2"); product2->price=5; product2->next=nullptr;

  bp_insert_product(bp_tree, 1, product1);
  bp_insert_product(bp_tree, 2, product2);

  Product_t* p1 = bp_get_product(bp_tree, 1);
  ASSERT_EQ(p1, product1);
  Product_t* p2 = bp_get_product(bp_tree, 2);
  ASSERT_EQ(p2, product2);
  
  bp_destroy_tree(bp_tree);
}

TEST(unittest, get_test2)
{
  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);

  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;
  Product_t* product2 = (Product_t*) malloc(sizeof(Product_t));
  product2->id = 2; strcpy(product2->name, "product2"); product2->price=5; product2->next=nullptr;
  Product_t* product3 = (Product_t*) malloc(sizeof(Product_t));
  product3->id = 3; strcpy(product3->name, "product3"); product3->price=5; product3->next=nullptr;
  Product_t* product4 = (Product_t*) malloc(sizeof(Product_t));
  product4->id = 4; strcpy(product4->name, "product4"); product4->price=5; product4->next=nullptr;
  Product_t* product5 = (Product_t*) malloc(sizeof(Product_t));
  product5->id = 5; strcpy(product5->name, "product5"); product5->price=5; product5->next=nullptr;
  Product_t* product6 = (Product_t*) malloc(sizeof(Product_t));
  product6->id = 6; strcpy(product6->name, "product6"); product6->price=5; product6->next=nullptr;
  Product_t* product7 = (Product_t*) malloc(sizeof(Product_t));
  product7->id = 7; strcpy(product7->name, "product7"); product7->price=5; product7->next=nullptr;

  bp_insert_product(bp_tree, 1, product1);
  bp_insert_product(bp_tree, 2, product2);
  bp_insert_product(bp_tree, 3, product3);
  bp_insert_product(bp_tree, 4, product4);
  bp_insert_product(bp_tree, 5, product5);
  bp_insert_product(bp_tree, 6, product6);
  bp_insert_product(bp_tree, 7, product7);

  ASSERT_EQ(bp_get_product(bp_tree, 1), product1);
  ASSERT_EQ(bp_get_product(bp_tree, 4), product4);
  ASSERT_EQ(bp_get_product(bp_tree, 7), product7);

  bp_destroy_tree(bp_tree);
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
