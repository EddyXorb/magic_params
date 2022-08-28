
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

template <class ENUM>
struct EnumClassHash
{
   auto operator()(ENUM t) const { return static_cast<std::size_t>(t); }
};
}  // namespace detail

template <class... ALLOWED_TYPES>
struct AllowedTypes
{
   using type = std::tuple<ALLOWED_TYPES...>;
};

template <class ENUM, class AllowedTypes, class DERIVED>
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

      constexpr ParamEntry(value_type defaultValue, const char* description)
          : defaultValue(defaultValue), description(description){};

      value_type defaultValue;
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

   template <Param key>
   static constexpr auto getDefaultParamEntry()
   {
      constexpr auto index = detail::EnumToIndex<key, decltype(DERIVED::settings)>::get();
      return std::get<index>(DERIVED::settings);
   }

public:
   constexpr MagicParams() {}

   template <Param key>
   constexpr auto getDescription() const
   {
      return getDefaultParamEntry<key>().description;
   }

   template <Param key>
   constexpr auto getDefault() const
   {
      return getDefaultParamEntry<key>().defaultValue;
   }

   template <Param key>
   auto get() const
   {
      constexpr auto defaultValue = getDefaultParamEntry<Param::one>();

      using value_type_searched = typename decltype(defaultValue)::value_type;
      constexpr auto indexOfRuntimeMaps = detail::Index<value_type_searched, const AllowedTypes::type>::value;

      auto& store = std::get<indexOfRuntimeMaps>(runtimestore_);

      auto it = store.find(key);
      if (it == store.end())
         return defaultValue.defaultValue;
      else
         return it->second;
   }

   template <Param key>
   bool set(decltype(std::declval<MagicParams>().get<key>()) value)
   {
      constexpr auto defaultValue = getDefaultParamEntry<Param::one>();
      using value_type_searched = typename decltype(defaultValue)::value_type;
      constexpr auto indexOfRuntimeMaps = detail::Index<value_type_searched, const AllowedTypes::type>::value;

      auto& store = std::get<indexOfRuntimeMaps>(runtimestore_);

      auto it = store.find(key);
      if (it == store.end())
      {
         if (value == defaultValue.defaultValue)
            return false;
         store[key] = value;
         return true;
      }
      else
      {
         if (value == defaultValue.defaultValue)
         {
            store.erase(it);
            return true;
         }
         else if (value != it->second)
         {
            it->second = value;
            return true;
         }
         return false;
      }
   }

private:
   template <class ValueType>
   using StoreSingleType = std::unordered_map<ENUM, ValueType, detail::EnumClassHash<ENUM>>;

   template <typename... Types>
   using tuple_cat_t = decltype(std::tuple_cat(std::declval<Types>()...));

   template <class Tuple>
   struct RuntimeStore;

   template <class T>
   struct RuntimeStore<std::tuple<T>>
   {
      static_assert(std::is_trivial<T>::value,
                    "Allowed value types for magic_params are only trivial types that can be constexpr.");
      using type = std::tuple<StoreSingleType<T>>;
   };

   template <class T, class... Types>
   struct RuntimeStore<std::tuple<T, Types...>>
   {
      static_assert(std::is_trivial<T>::value,
                    "Allowed value types for magic_params are only trivial types that can be constexpr.");
      using type = tuple_cat_t<std::tuple<StoreSingleType<T>>, typename RuntimeStore<std::tuple<Types...>>::type>;
   };

public:
   typename RuntimeStore<typename AllowedTypes::type>::type runtimestore_;
};

struct MyMagicParams : MagicParams<Param, AllowedTypes<int, const char*, double>, MyMagicParams>
{
   constexpr MyMagicParams(){};

   static constexpr auto settings =
       make_param_tuple(ParamEntry<Param::one, int>(0, "first parameter"),
                        ParamEntry<Param::two, const char*>("Default-String", "second parameter"));
};

int main()
{
   // constexpr auto entry = MyMagicParams::getDefaultParamEntry<Param::two>();

   // std::cout << "Parameter " << static_cast<int>(entry.key) << " with description'" << entry.description
   //           << "' has default value '" << entry.defaultValue << "'.";

   auto myParams = MyMagicParams();
   const auto& m = std::get<0>(myParams.runtimestore_);
   auto i = myParams.get<Param::one>();

   // constexpr auto defaultValue = myParams.getDefaultParamEntry<Param::one>();
   // using value_type_searched = typename decltype(defaultValue)::value_type;
   // constexpr auto indexOfRuntimeMaps = detail::Index<value_type_searched, typename allowed::type>::value;

   std::cout << i;
   std::cout << "before" << myParams.get<Param::one>() << "\n";
   myParams.set<Param::one>(2);
   std::cout << myParams.get<Param::one>();
   // WOULD FAIL with meaningful static_assert-message.
   // constexpr auto entry = MagicParams::getDefaultParamEntry<Param::three>();
}
