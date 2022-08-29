// MagicParams - C. Roehl 2022

#include <unordered_map>

#include <tuple>
#include <type_traits>

namespace magicparams
{
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

template <class enum_type, enum_type key, bool exists>
struct PrintAssertIfKeyNotExistent
{
   static_assert(exists,
                 "Requested key is not present in default collection of magicparams. Look below for compilermessage "
                 "such as 'PrintAssertIfKeyNotExistent<Param::three,false>' to know which key was not defined.");
   static constexpr bool dummy = exists;
};

template <class enum_type, enum_type key, class Tuple>
struct EnumToIndex
{
   static constexpr auto get()
   {
      constexpr auto keyExists = value >= 0;
      constexpr auto dummy = PrintAssertIfKeyNotExistent<enum_type, key, keyExists>::dummy;
      return value;
   }

private:
   static constexpr auto value = Index<std::integral_constant<enum_type, key>, Tuple>::value + 1;
};

template <class ENUM>
struct EnumClassHash
{
   auto operator()(ENUM t) const { return static_cast<std::size_t>(t); }
};
}  // namespace detail

template <typename... Types>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<Types>()...));

// this struct reduces doubled arguments and converts std::string -> const char*
template <class... Types>
struct AllowedTypes;

template <class... Types>
struct AllowedTypes<std::string, Types...>
{
   using type = typename AllowedTypes<const char*, Types...>::type;
};

template <class T, class U, class... Types>
struct AllowedTypes<T, U, Types...>
{
   static constexpr auto sameTypeWillAppearLater = detail::Index<T, const std::tuple<U, Types...>>::value >= 0;
   static constexpr auto shouldOmitBecauseOfString =
       std::is_same<const char*, T>::value && (detail::Index<std::string, const std::tuple<U, Types...>>::value >= 0);

   using type = typename std::conditional<sameTypeWillAppearLater || shouldOmitBecauseOfString,  // type does not appear
                                                                                                 // in remaining types.
                                          typename AllowedTypes<U, Types...>::type,  // If it exists, jump over it
                                          tuple_cat_t<std::tuple<T>, typename AllowedTypes<U, Types...>::type>>::type;
};

template <>
struct AllowedTypes<std::string>
{
   using type = std::tuple<const char*>;
};

template <class T>
struct AllowedTypes<T>
{
   using type = std::tuple<T>;
};

template <class ENUM, class ALLOWED_TYPES, class DERIVED>
class MagicParams
{
private:
   static_assert(std::is_enum<ENUM>::value, "You need to provide an enum to MagicParams first template parameter!");

   struct ParamEntryBase
   {
   };

protected:
   // strong types to improve readability
   struct Description
   {
      constexpr explicit Description(const char* description) : value(description) {}
      const char* value;
   };

   template <ENUM enum_entry, class VALUE_TYPE, typename = void>
   struct Add : ParamEntryBase
   {
      using value_type = VALUE_TYPE;
      static constexpr bool is_string_actually = false;

      static constexpr ENUM key = enum_entry;

      constexpr Add(value_type defaultValue, Description description = Description(""))
          : defaultValue(defaultValue), description(description.value){};

      value_type defaultValue;
      const char* description;
   };

   template <ENUM enum_entry>
   struct Add<enum_entry, std::string, void> : ParamEntryBase
   {
      using value_type = const char*;
      static constexpr bool is_string_actually = true;

      static constexpr ENUM key = enum_entry;

      constexpr Add(value_type defaultValue, Description description = Description(""))
          : defaultValue(defaultValue), description(description.value){};

      value_type defaultValue;
      const char* description;
   };

   template <class T>
   static constexpr auto create(T t)
   {
      static_assert(std::is_base_of<ParamEntryBase, T>::value, "Parameter entries have to be of type ParamEntry!");
      return std::make_tuple(std::integral_constant<ENUM, T::key>(), t);
   }

   template <class T, class... Types>
   static constexpr auto create(T t, Types... args)
   {
      static_assert(std::is_base_of<ParamEntryBase, T>::value, "Parameter entries have to be of type ParamEntry!");
      return std::tuple_cat(std::make_tuple(std::integral_constant<ENUM, T::key>(), t), create(args...));
   }

private:
   template <ENUM key>
   static constexpr auto getDefaultParamEntry()
   {
      constexpr auto index = detail::EnumToIndex<ENUM, key, decltype(DERIVED::settings)>::get();
      return std::get<index>(DERIVED::settings);
   }

   template <ENUM key>
   static constexpr auto getRuntimeStoreIndex()
   {
      constexpr auto defaultValue = getDefaultParamEntry<key>();

      using value_type_searched = typename decltype(defaultValue)::value_type;
      constexpr auto indexOfRuntimeMaps = detail::Index<value_type_searched, const ALLOWED_TYPES::type>::value;
      return indexOfRuntimeMaps;
   }

   template <ENUM key>
   auto get_intern() const
   {
      constexpr auto paramEntry = getDefaultParamEntry<key>();
      constexpr auto indexOfRuntimeMaps = getRuntimeStoreIndex<key>();

      if (storeIsEmpty)
         return paramEntry.defaultValue;

      auto& store = std::get<indexOfRuntimeMaps>(runtimestore_);

      auto it = store.find(key);
      if (it == store.end())
         return paramEntry.defaultValue;
      else
         return it->second;
   }

   template <ENUM key>
   using ValueTypeOf = decltype(std::declval<MagicParams>().get_intern<key>());

   template <ENUM key>
   bool set_intern(ValueTypeOf<key> value)
   {
      constexpr auto paramEntry = getDefaultParamEntry<key>();
      constexpr auto indexOfRuntimeMaps = getRuntimeStoreIndex<key>();

      auto& store = std::get<indexOfRuntimeMaps>(runtimestore_);

      auto it = store.find(key);
      if (it == store.end())
      {
         if (value == paramEntry.defaultValue)
            return false;

         store[key] = value;
         storeIsEmpty = false;
         return true;
      }

      if (value == it->second)
         return false;

      it->second = value;
      return true;
   }

   template <ENUM key>
   static constexpr bool isStringActually =
       decltype(std::declval<MagicParams>().getDefaultParamEntry<key>())::is_string_actually;

public:
   constexpr MagicParams() {}

   template <ENUM key>
   static constexpr auto getDescription()
   {
      return getDefaultParamEntry<key>().description;
   }

   template <ENUM key>
   static constexpr auto getDefault()
   {
      return getDefaultParamEntry<key>().defaultValue;
   }

   template <ENUM key, typename = void>
   auto get() const -> std::enable_if_t<!isStringActually<key>, ValueTypeOf<key>>
   {
      return get_intern<key>();
   }

   template <ENUM key>
   auto get() const -> std::enable_if_t<isStringActually<key>, std::string>
   {
      return std::string(get_intern<key>());
   }

   template <ENUM key, typename = void>
   auto set(ValueTypeOf<key> toSet) -> std::enable_if_t<!isStringActually<key>, bool>
   {
      return set_intern<key>(toSet);
   }

   template <ENUM key>
   auto set(const std::string& toSet) -> std::enable_if_t<isStringActually<key>, bool>
   {
      return set_intern<key>(toSet.c_str());
   }

private:
   template <class ValueType>
   using StoreSingleType = std::unordered_map<ENUM, ValueType, detail::EnumClassHash<ENUM>>;

   template <class Tuple>
   struct RuntimeStore;

   template <class T>
   struct RuntimeStore<std::tuple<T>>
   {
      static_assert(std::is_trivial<T>::value,
                    "Allowed value types for magic_params are only trivial types that can be constexpr");
      using type = std::tuple<StoreSingleType<T>>;
   };

   template <class T, class... Types>
   struct RuntimeStore<std::tuple<T, Types...>>
   {
      static_assert(std::is_trivial<T>::value,
                    "Allowed value types for magic_params are only trivial types that can be constexpr.");
      using type = tuple_cat_t<std::tuple<StoreSingleType<T>>, typename RuntimeStore<std::tuple<Types...>>::type>;
   };

   bool storeIsEmpty = true;
   typename RuntimeStore<typename ALLOWED_TYPES::type>::type runtimestore_;
};
}  // namespace magicparams
