# MagicParams
## Quickstart
This is a header-only C++14 compliant library that defines a class `MagicParams` that maps enums to types at compile time, and allows storing and retriving of values for every enum.

Look at `src/example.cpp` or the tests in `tests/test.cpp` to see them in action.

You can use this as conan dependency with 
```
requires = "magicparams/1.0"
```
in your conanfile if you want, or just include it normally.

## Motivation
Often one wants to map enum-keys to values that are used as parameters within bigger code projects. If all parameters are of the same type, one could just use a `unorered_map<key,value>` for example.
It gets trickier if one needs a mapping from the same enum to multiple value types. Take for example

```cpp
enum class Parameter
{
    weight,
    height,
    name,
    fractions
}
```
One clearly would need at least the types `string` and `double` (and maybe also `int`).
You could define your mapping by hand using different maps for different types - not nice, obviously. Another approach would be to make use of `boost::variant` or `std::variant`, if available. A call to this structure (call it '`VariantParameters`') would somehow involve a template parameter of the type you want to set or you want to get, such as

```cpp
VariantParameter p;
int height = p.get<int>(Parameter::height);
std::string name = p.get<int>(Parameter::name);
```
and so on. The problem with this approach is more subtle: if you set e.g. `height` as `int` and call it later with `double` such as 

```cpp
VariantParameters p;
 p.set<int>(Parameter::height,13424);
// ... long after you do:
double height = p.get<int>(Paremeter::height); // exception!!
```
you will get an exception, at runtime of course. So you have to remember which parameter was set with which valuetype, which is in the best case annoying, in the worst it can happen after days of calculation of something and the exception thrown could render all the calculatory work done useless.

## Solution
MagicParams, what else. Assume we have an enum class "`Param`" with entries "one","two" and so on.
Look:
```cpp
int main()
{
   using namespace magicparams;
   // you can get the default values and the description of parameters at compile-time!
   constexpr auto value = MyMagicParams::getDefault<Param::two>();
   constexpr auto desc = MyMagicParams::getDescription<Param::two>();

   std::cout << "Parameter 'two' has default value " << value << " and its description is '" << desc << "'\n";
   // create runtime version
   auto myParams = MyMagicParams();
   auto valueBefore = myParams.get<Param::one>();
   // set variable for Param::one at runtime
   myParams.set<Param::one>(3);
   auto valueAfter = myParams.get<Param::one>();

   std::cout << "Parameter 'one' was set from " << valueBefore << " to " << valueAfter;
}
```
This example is provided in `example.cpp`.
As you can see the connection between parameter and type is enforced during compiletime, otherwise the line
```cpp
constexpr auto value = MyMagicParams::getDefault<Param::two>();
```
would not compile.
How do you define such nice looking compile-time-type-safe-parameter-containers?
Given an enum class such as
```cpp
enum class Param
{
   one = 0,
   two,
   three
};
```
you define your MagicParams such as:
```cpp
using namespace magicparams;
struct MyMagicParams : MagicParams<Param, AllowedTypes<int, std::string>, MyMagicParams>
{
   constexpr MyMagicParams(){};

   static constexpr auto settings = create(
       Add<Param::one, int>(99,Description{"first parameter"}), 
       Add<Param::two, std::string>("Default-String", Description("second parameter")),
       Add<Param::three, bool>(true, Description{"third parameter"}));
};
```
As you can see the parameter `Param::one` is tied to the type `int` in the first `Add`.
In parenthesis you set a default value (e.g. 99) that can be retrieved at compile time as you saw before and the second parameter "`Description`" is only optional.
So the synopsis for the creation of map-entries is
```
static constexpr auto settings = create(Add<enum-value,type to map to>(default-value,Description("optional description")),
Add(...),
...);
```

**NOTE: only types T where this**
```cpp
static_assert(std::is_trivial<T>::value,"");
```
**compiles can be used to store values in MagicParams (the only exception to this rule are std::string's).**
This includes all plain old datatypes such as `int,const char*,short,double,unsigned,long,char,..` and so on.

If you call the runtime-getter and setter `get`/`set`, and you did not set a parameter before calling `get`, the default value will be returned.

## Build example and tests
Install conan and cmake, create a subfolder `/build`, change into it and run
```
conan install ..
conan build ..
```
and run the executables in the subfolder `build/bin/` such as
```
./bin/example.cpp
./bin/magicparamstests.exe
```
Thats it!
## Performance
Given that every call to the parameter-container can possibly be resolved at compile time if you only need default parameters, the resulting performance is unbeatable. 
When it comes to runtime calls there are certain things to consider.

* the implementation uses unordered_maps internally with constant access time, so they should be really fast.

* *if the parameter container is not changed during runtime, it should be even faster because in this case the constexpr default value will be returned and the only cost is to check whether a bool variable is not set.

* everything that could be precalculated at compile-time such as the index to the unordered_map that is responsible for a certain parameter is calculated at compile time, so there should be zero overhead to an implementation that only stores one value-type of parameters.

**If you want to store big amounts of strings** however, you better use `const char*`'s as every call to a `std::string`-based entries will convert (and thus copy) from an underlying `const char*`-entry in these maps. This is for convenience and as long as the strings are short, the compiler normally uses short-string optimization (for strings haveing less than 22 characters on a 64 bit machine), so this only gets an issue if you are storing long strings and call them *very* often.
## Implementation details
For runtime storing a
```cpp
 std::tuple<unordered_map<ENUM,v1>,unordered_map<ENUM,v2>,...>
``` 
 and so on is used. 

The `AllowedTypes`-struct is designed to shrink a list of types such as `<int,int,const char*,double,int,short,const char*,std::string>` internally to `<double,int,short,const char*>` making use of template metaprogramming. It converts `std::string ` to `const char*` internally. The result of this type-list is the basis for the internally used `runtimeStore_`, where every tuple-entry maps to a type of the `AllowedTypes`-list.


## Possible extension
It would be nice to think about what would be possible if we can assume the enum to be a [magic_enum](https://github.com/Neargye/magic_enum)! Up to now there is no restriction on the enum variant, also [BETTER_ENUM](https://github.com/aantron/better-enums) should work out of the box (I did not test it so far). Having these tools at hand one could e.g. check if every enum has a mapping and output nicer compile-time static assert messages, if this is not the case, listing every enum that is missing.

