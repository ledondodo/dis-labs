#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "unistd.h"
extern "C"
{
    #include "ex1.h"
    #include "common.h"
}

const size_t k_ntests = 3;

TEST(unittest, search_by_product_name)
{
  Product_t product1 = {1, "product1", 100, nullptr};
  Product_t product2 = {2, "product2", 200, &product1};
  Product_t product3 = {3, "product3", 300, &product2};

  Product_t* p = search_product_by_name(&product3, "product1");
  assert(p != nullptr);
  EXPECT_EQ(p->price, 100);
}

TEST(unittest, search_by_price)
{
  Product_t product1 = {1, "product1", 100, nullptr};
  Product_t product2 = {2, "product2", 200, &product1};
  Product_t product3 = {3, "product3", 300, &product2};

  Product_t* p = search_product_by_price(&product3, 200);
  assert(p != nullptr);
  EXPECT_EQ(p->price, 200);

  p = search_product_by_price(&product3, 100);
  assert(p != nullptr);
  EXPECT_EQ(p->price, 100);  
}

TEST(unittest, search_by_id)
{
  Product_t product1 = {1, "product1", 100, nullptr};
  Product_t product2 = {2, "product2", 200, &product1};
  Product_t product3 = {3, "product3", 300, &product2};

  Product_t* p = search_product_by_id(&product3, 1);
  assert(p != nullptr);
  EXPECT_EQ(p, &product1);

  p = search_product_by_price(&product3, 12);
  EXPECT_EQ(p, nullptr);  
}

TEST(unittest, empty_handles)
{
  Product_t* p = search_product_by_name(nullptr, "product1");
  EXPECT_EQ(p, nullptr);

  p = search_product_by_price(nullptr, 100);
  EXPECT_EQ(p, nullptr);
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
