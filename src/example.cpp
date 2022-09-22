
#include <iostream>

#include <magic_params.hpp>

enum class Param
{
   one = 0,
   two,
   three
};

// derive from MagicParams; NOTE: the third template Parameter is the name of class you are defining (CRTP-Pattern, see
// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
// The second template parameter is used to specify which value types you want to map to, such as
// AllowedTypes<int,double,std::string>
struct MyMagicParams : magicparams::MagicParams<Param, magicparams::AllowedTypes<int, std::string>, MyMagicParams>
{
   // you have to define the constructor of the derived class constexpr like this
   constexpr MyMagicParams(){};

   // define a static constexpr member "settings" like that.
   static constexpr auto settings = create(
       Add<Param::one, int>(
           99,
           Description{"first parameter"}),  //<-- this is a declaration for one Parameter that is mapped from the enum
                                             //"Param::two", being of type int and having default value "99".
                                             // You don't have to define "settings" for every member of the enum class.
                                             // If you don't define it for one parameter and call getter/setter on that,
                                             // it won't compile.
                                             // NOTE: the second Parameter in Add (of type "Description") is optional
       Add<Param::two, std::string>("Default-String", Description("second parameter")),
       Add<Param::three, bool>(true, Description{"third parameter"}));
};

int main()
{
   using namespace magicparams;
   // you can get the default values and the description of parameters at compile-time!
   constexpr auto value = MyMagicParams::getDefault<Param::two>();
   constexpr auto desc = MyMagicParams::getDescription<Param::two>();

   std::cout << "Parameter 'two' has default value " << value << " and its description is '" << desc << "'\n";
   // Parameter 'two' has default value Default-String and its description is 'second parameter'
   
   // create runtime version
   auto myParams = MyMagicParams();
   auto valueBefore = myParams.get<Param::one>();
   // set variable for Param::one at runtime
   myParams.set<Param::one>(3);
   auto valueAfter = myParams.get<Param::one>();

   std::cout << "Parameter 'one' was set from " << valueBefore << " to " << valueAfter;
   // Parameter 'one' was set from 99 to 3
}
