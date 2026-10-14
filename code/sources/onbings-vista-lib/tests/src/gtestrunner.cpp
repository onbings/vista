
#include "gtestrunner.h"

int main(int argc, char *argv[])
{
  testing::InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG(filter) = "onbings_vista_engine_tests.*";
  return RUN_ALL_TESTS();
}