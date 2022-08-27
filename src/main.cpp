
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

template <Param enum_entry, class VALUE_TYPE>
struct ParamEntry
{
   static_assert(std::is_enum<Param>::value, "ENUM_TYPE must be an enum in ParamEntry!");

   using value_type = VALUE_TYPE;

   static constexpr Param key = enum_entry;

   constexpr ParamEntry(value_type defaultvalue, const char* description)
       : defaultvalue(defaultvalue), description(description){};

   value_type defaultvalue;
   const char* description;
};

template <class T>
constexpr auto make_param_tuple(T t)
{
   return std::make_tuple(std::integral_constant<Param, T::key>(), t);
}

template <class T, class... Types>
constexpr auto make_param_tuple(T t, Types... args)
{
   return std::tuple_cat(std::make_tuple(std::integral_constant<Param, T::key>(), t), make_param_tuple(args...));
}

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

class MagicParams
{
public:
   static constexpr auto tuples =
       make_param_tuple(ParamEntry<Param::one, int>(0, "first parameter"),
                        ParamEntry<Param::two, const char*>("Default-String", "second parameter"));

   constexpr MagicParams() {}

   template <Param key>
   static constexpr auto getDefaultParamEntry()
   {
      constexpr auto index = detail::EnumToIndex<key, decltype(tuples)>::get();
      return std::get<index>(tuples);
   }
};

int main()
{
   constexpr auto entry = MagicParams::getDefaultParamEntry<Param::two>();

   std::cout << "Parameter " << static_cast<int>(entry.key) << " with description'" << entry.description
             << "' has default value '" << entry.defaultvalue << "'.";

   // WOULD FAIL with meaningful static_assert-message.
   // constexpr auto entry = MagicParams::getDefaultParamEntry<Param::three>();
}
