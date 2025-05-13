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

const size_t k_ntests = 1;

TEST(unittest, check_bp_size)
{
  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);
  ASSERT_EQ(bp_tree->m, 4);

  bp_destroy_tree(bp_tree);
}

TEST(unittest, single_add)
{
  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;

  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);
  ASSERT_EQ(bp_insert_product(bp_tree, 2, product1), 0);

  bp_destroy_tree(bp_tree);
}

TEST(unittest, check_same_key)
{
  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;

  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);

  bp_insert_product(bp_tree, 1, product1);
  ASSERT_EQ(bp_insert_product(bp_tree, 1, product1), 1);

  bp_destroy_tree(bp_tree);
}

TEST(unittest, insert_4keys)
{ 
  Product_t* product1 = (Product_t*) malloc(sizeof(Product_t));
  product1->id = 1; strcpy(product1->name, "product1"); product1->price=5; product1->next=nullptr;
  Product_t* product2 = (Product_t*) malloc(sizeof(Product_t));
  product2->id = 2; strcpy(product2->name, "product2"); product2->price=5; product2->next=nullptr;
  Product_t* product3 = (Product_t*) malloc(sizeof(Product_t));
  product3->id = 3; strcpy(product3->name, "product3"); product3->price=5; product3->next=nullptr;
  Product_t* product4 = (Product_t*) malloc(sizeof(Product_t));
  product4->id = 4; strcpy(product4->name, "product4"); product4->price=5; product4->next=nullptr;

  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);

  ASSERT_EQ(bp_insert_product(bp_tree, 1, product1), 0);
  ASSERT_NE(bp_tree->root, nullptr);
  ASSERT_EQ(bp_insert_product(bp_tree, 2, product2), 0);
  ASSERT_EQ(bp_insert_product(bp_tree, 3, product3), 0);
  ASSERT_EQ(bp_insert_product(bp_tree, 4, product4), 0);

  ASSERT_EQ(bp_tree->root->n, 4);

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
