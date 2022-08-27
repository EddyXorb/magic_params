#include "gtest/gtest.h"

#include "magic_params.hpp"
class MagicParamsTester : public ::testing::Test
{
};

TEST_F(MagicParamsTester, simpletest)
{
   EXPECT_TRUE(true);
};