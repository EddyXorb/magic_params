
#include <iostream>
#include <tuple>
#include <type_traits>

#include <magic_params.hpp>

enum class Param
{
   one = 0,
   two,
   three
};

namespace detail
{
template <class T, class Tuple>
struct Index
{
   static constexpr auto value = std::numeric_limits<int>::min();
};

template <class T, class... Types>
struct Index<T, const std::tuple<T, Types...>>
{
   static const int value = 0;
};

template <class T, class U, class... Types>
struct Index<T, const std::tuple<U, Types...>>
{
   static const int value = 1 + Index<T, const std::tuple<Types...>>::value;
};

template <Param key, bool exists>
struct PrintAssertIfKeyNotExistent
{
   static_assert(exists,
                 "Requested key is not present in default collection of magicparams. Look below for compilermessage "
                 "such as 'PrintAssertIfKeyNotExistent<Param::three,false>' to know which key was not defined.");
   static constexpr bool dummy = exists;
};

template <Param p, class Tuple>
struct EnumToIndex
{
   static constexpr auto get()
   {
      constexpr auto keyExists = value >= 0;
      constexpr auto dummy = PrintAssertIfKeyNotExistent<p, keyExists>::dummy;
      return value;
   }

private:
   static constexpr auto value = Index<std::integral_constant<Param, p>, Tuple>::value + 1;
};

}  // namespace detail

template <class ENUM, class DERIVED>
class MagicParams
{
private:
   static_assert(std::is_enum<ENUM>::value, "ENUM_TYPE must be an enum in ParamEntry!");

   struct ParamEntryBase
   {
   };

   template <ENUM enum_entry, class VALUE_TYPE>
   struct ParamEntry : ParamEntryBase
   {
      using value_type = VALUE_TYPE;
      static constexpr ENUM key = enum_entry;

      constexpr ParamEntry(value_type defaultvalue, const char* description)
          : defaultvalue(defaultvalue), description(description){};

      value_type defaultvalue;
      const char* description;
   };

protected:
   template <class T>
   static constexpr auto make_param_tuple(T t)
   {
      static_assert(std::is_base_of<ParamEntryBase, T>::value, "Parameter entries have to be of type ParamEntry!");
      return std::make_tuple(std::integral_constant<Param, T::key>(), t);
   }

   template <class T, class... Types>
   static constexpr auto make_param_tuple(T t, Types... args)
   {
      static_assert(std::is_base_of<ParamEntryBase, T>::value, "Parameter entries have to be of type ParamEntry!");
      return std::tuple_cat(std::make_tuple(std::integral_constant<Param, T::key>(), t), make_param_tuple(args...));
   }

public:
   constexpr MagicParams() {}

   template <Param key>
   static constexpr auto getDefaultParamEntry()
   {
      constexpr auto index = detail::EnumToIndex<key, decltype(DERIVED::settings)>::get();
      return std::get<index>(DERIVED::settings);
   }
};

struct MyMagicParams : MagicParams<Param, MyMagicParams>
{
   constexpr MyMagicParams(){};

   static constexpr auto settings =
       make_param_tuple(ParamEntry<Param::one, int>(0, "first parameter"),
                        ParamEntry<Param::two, const char*>("Default-String", "second parameter"));
};

int main()
{
   constexpr auto entry = MyMagicParams::getDefaultParamEntry<Param::two>();

   std::cout << "Parameter " << static_cast<int>(entry.key) << " with description'" << entry.description
             << "' has default value '" << entry.defaultvalue << "'.";

   // WOULD FAIL with meaningful static_assert-message.
   // constexpr auto entry = MagicParams::getDefaultParamEntry<Param::three>();
}
