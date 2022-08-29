#include "gtest/gtest.h"

#include "magic_params.hpp"

using namespace magicparams;

enum class Params
{
   myDouble,
   myString,
   myShort,
   myConstCharPtr,
   myChar,
   myBool,
   myInt,
   myOtherString,
   myOtherBool,
   myOtherInt
};

class TestParams
    : public MagicParams<Params, AllowedTypes<double, std::string, short, char, const char*, bool, int>, TestParams>
{
public:
   constexpr TestParams(){};

   static constexpr auto settings = create(
       Add<Params::myDouble, double>(3.5, Description("myDouble parameter")),
       Add<Params::myString, std::string>("myString default value", Description("MyString parameter")),
       Add<Params::myConstCharPtr, const char*>("myConstChar default value", Description("MyConstChar parameter")),
       Add<Params::myChar, char>(3, Description("MyChar parameter")),
       Add<Params::myShort, short>(33, Description("MyShort parameter")),
       Add<Params::myBool, bool>(false, Description("MyBool parameter")),
       Add<Params::myInt, int>(-10304, Description("MyInt parameter")));
};

class MagicParamsTester : public ::testing::Test
{
public:
   TestParams myParams = TestParams();
};

TEST_F(MagicParamsTester, getConstexprDefaultValues)
{
   constexpr auto myDefaultDouble = TestParams::getDefault<Params::myDouble>();
   constexpr auto myDefaultString = TestParams::getDefault<Params::myString>();
   constexpr auto myDefaultConstCharPtr = TestParams::getDefault<Params::myConstCharPtr>();
   constexpr auto myDefaultChar = TestParams::getDefault<Params::myChar>();
   constexpr auto myDefaultShort = TestParams::getDefault<Params::myShort>();
   constexpr auto myDefaultBool = TestParams::getDefault<Params::myBool>();
   constexpr auto myDefaultInt = TestParams::getDefault<Params::myInt>();
};

TEST_F(MagicParamsTester, getConstexprDescriptionValues)
{
   constexpr auto myDefaultDouble = TestParams::getDescription<Params::myDouble>();
   constexpr auto myDefaultString = TestParams::getDescription<Params::myString>();
   constexpr auto myDefaultConstCharPtr = TestParams::getDescription<Params::myConstCharPtr>();
   constexpr auto myDefaultChar = TestParams::getDescription<Params::myChar>();
   constexpr auto myDefaultShort = TestParams::getDescription<Params::myShort>();
   constexpr auto myDefaultBool = TestParams::getDescription<Params::myBool>();
   constexpr auto myDefaultInt = TestParams::getDescription<Params::myInt>();
};

TEST_F(MagicParamsTester, setValuesWorksAtRuntime)
{
   myParams.set<Params::myDouble>(1.1);
   EXPECT_EQ(1.1, myParams.get<Params::myDouble>());

   myParams.set<Params::myString>("This is the new string!");
   EXPECT_EQ("This is the new string!", myParams.get<Params::myString>());

   myParams.set<Params::myConstCharPtr>("This is the new const char ptr!");
   EXPECT_EQ("This is the new const char ptr!", myParams.get<Params::myConstCharPtr>());

   myParams.set<Params::myChar>(-127);
   EXPECT_EQ(-127, myParams.get<Params::myChar>());

   myParams.set<Params::myShort>(12);
   EXPECT_EQ(12, myParams.get<Params::myShort>());

   myParams.set<Params::myBool>(true);
   EXPECT_EQ(true, myParams.get<Params::myBool>());

   myParams.set<Params::myInt>(-9999);
   EXPECT_EQ(-9999, myParams.get<Params::myInt>());
};

TEST_F(MagicParamsTester, getRetrievesDefaultValueIfUnchanged)
{
   auto toTest = myParams.get<Params::myInt>();
   EXPECT_EQ(toTest, myParams.getDefault<Params::myInt>());
}

TEST_F(MagicParamsTester, stringIsResultOfGetWithoutImplicitConversion)
{
   constexpr bool works =
       std::is_same<std::string, decltype(std::declval<TestParams>().get<Params::myString>())>::value;

   EXPECT_TRUE(works);
}

TEST_F(MagicParamsTester, constCharPtrIsNotTreatedAsString)
{
   constexpr bool works =
       std::is_same<const char*, decltype(std::declval<TestParams>().get<Params::myConstCharPtr>())>::value;

   EXPECT_TRUE(works);
}

// Uncomment and try to compile -> it shouldn't work
// TEST_F(MagicParamsTester, doesNotCompileIfUndeclaredTypeAccessed)
// {
//    constexpr auto thisShouldFailToCompile = myParams.get<Params::myOtherBool>();
// }