#pragma once

#include "base/common/base.hpp"
#include "base/common/pp.hpp"
#include "base/common/hashmap.hpp"
#include "base/common/string.hpp"

namespace std {
template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
}  // namespace std

namespace Neko {

namespace reflection {

template <unsigned short N>
struct cstring {
    constexpr explicit cstring(std::string_view str) noexcept : cstring{str, std::make_integer_sequence<unsigned short, N>{}} {}
    constexpr const char* data() const noexcept { return chars_; }
    constexpr unsigned short size() const noexcept { return N; }
    constexpr operator std::string_view() const noexcept { return {data(), size()}; }
    template <unsigned short... I>
    constexpr cstring(std::string_view str, std::integer_sequence<unsigned short, I...>) noexcept : chars_{str[I]..., '\0'} {}
    char chars_[static_cast<size_t>(N) + 1];
};
template <>
struct cstring<0> {
    constexpr explicit cstring(std::string_view) noexcept {}
    constexpr const char* data() const noexcept { return nullptr; }
    constexpr unsigned short size() const noexcept { return 0; }
    constexpr operator std::string_view() const noexcept { return {}; }
};

template <typename T>
constexpr auto name_raw() noexcept {
#if defined(__clang__) || defined(__GNUC__)
    std::string_view name = __PRETTY_FUNCTION__;
    size_t start = name.find('=') + 2;
    size_t end = name.size() - 1;
    return std::string_view{name.data() + start, end - start};
#elif defined(_MSC_VER)
    std::string_view name = __FUNCSIG__;
    size_t start = name.find('<') + 1;
    size_t end = name.rfind(">(");
    name = std::string_view{name.data() + start, end - start};
    start = name.find(' ');
    return start == std::string_view::npos ? name : std::string_view{name.data() + start + 1, name.size() - start - 1};
#else
#error Unsupported compiler
#endif
}

template <typename T>
constexpr auto name() noexcept {
    constexpr auto name = name_raw<T>();
    return cstring<name.size()>{name};
}
template <typename T>
constexpr auto name_v = name<T>();

struct DummyFlag {};
template <typename Enum, typename T, Enum enumValue>
inline int get_enum_value(HashMap<String>& values) {
#if defined _MSC_VER && !defined __clang__
    std::string func(__FUNCSIG__);
    std::string mark = "DummyFlag";
    auto pos = func.find(mark) + mark.size();
    std::string enumStrRaw = func.substr(pos);

    auto start = enumStrRaw.find_first_not_of(", ");
    auto end = enumStrRaw.find('>');
    if (start != enumStrRaw.npos && end != enumStrRaw.npos && enumStrRaw[start] != '(') {
        enumStrRaw = enumStrRaw.substr(start, end - start);
        if (!values.get(enumValue)) {
            String enumStr = to_cstr(enumStrRaw);
            values[enumValue] = enumStr;
        }
    }

#else  // gcc, clang
    std::string func(__PRETTY_FUNCTION__);
    std::string mark = "enumValue = ";
    auto pos = func.find(mark) + mark.size();
    std::string enumStrRaw = func.substr(pos, func.size() - pos - 1);
    char ch = enumStrRaw[0];
    if (!(ch >= '0' && ch <= '9') && ch != '(') {
        if (!values.get(enumValue)) {
            String enumStr = to_cstr(enumStrRaw);
            values[enumValue] = enumStr;
        }
    }
#endif
    return 0;
}

template <typename Enum, int min_value, int... ints>
void guess_enum_range(HashMap<String>& values, const std::integer_sequence<int, ints...>&) {
    auto dummy = {get_enum_value<Enum, DummyFlag, (Enum)(ints + min_value)>(values)...};
}

template <typename Enum, int... ints>
void guess_enum_bit_range(HashMap<String>& values, const std::integer_sequence<int, ints...>&) {
    auto dummy = {get_enum_value<Enum, DummyFlag, (Enum)0>(values), get_enum_value<Enum, DummyFlag, (Enum)(1 << (int)ints)>(values)...};
}

template <typename T>
auto GetTypeName() {
    return reflection::name_v<T>.data();
}

template <typename T>
struct wrapper {
    T a;
};
template <typename T>
wrapper(T) -> wrapper<T>;

template <typename T>
static inline T storage = {};

template <auto T>
constexpr auto main_name_of_pointer() {
    constexpr auto is_identifier = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; };
#if __GNUC__ && (!__clang__) && (!_MSC_VER)
    std::string_view str = __PRETTY_FUNCTION__;
    std::size_t start = str.rfind("::") + 2;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
        1
    }
    return str.substr(start, end - start);
#elif __clang__
    std::string_view str = __PRETTY_FUNCTION__;
    std::size_t start = str.rfind(".") + 1;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
    }
    return str.substr(start, end - start);
#elif _MSC_VER
    std::string_view str = __FUNCSIG__;
    std::size_t start = str.rfind("->") + 2;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
    }
    return str.substr(start, end - start);
#else
    static_assert(false, "Not supported compiler");
#endif
}

// 无定义 我们需要一个可以转换为任何类型的在以下特殊语境中使用的辅助类
struct __Any {
    constexpr __Any() {}
    constexpr __Any(int) {}
    template <typename T>
        requires std::is_copy_constructible_v<T>
    constexpr operator T&() const;
    template <typename T>
        requires std::is_move_constructible_v<T>
    constexpr operator T&&() const;
    template <typename T>
        requires(!std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>)
    constexpr operator T() const;
};

// 计算结构体成员数量
#if !defined(_MSC_VER) || 0  // 我不知道为什么 if constexpr (!requires { T{Args...}; }) {...} 方法会导致目前版本的vs代码感知非常卡

template <typename T>
consteval size_t struct_size(auto&&... Args) {
    if constexpr (!requires { T{Args...}; }) {
        return sizeof...(Args) - 1;
    } else {
        return struct_size<T>(Args..., __Any{1});
    }
}

template <class T>
struct struct_member_count : std::integral_constant<std::size_t, struct_size<T>()> {};

#else

template <typename T, typename = void, typename... Ts>
struct struct_size {
    constexpr static size_t value = sizeof...(Ts) - 1;
};

template <typename T, typename... Ts>
struct struct_size<T, std::void_t<decltype(T{Ts{}...})>, Ts...> {
    constexpr static size_t value = struct_size<T, void, Ts..., __Any>::value;
};

template <class T>
struct struct_member_count : std::integral_constant<std::size_t, struct_size<T>::value> {};

#endif

template <typename T, std::size_t N>
constexpr std::size_t try_initialize_with_n() {
    return []<std::size_t... Is>(std::index_sequence<Is...>) { return requires { T{__Any(Is)...}; }; }(std::make_index_sequence<N>{});
}

template <typename T, std::size_t N = 0>
constexpr auto field_count_impl() {
    if constexpr (try_initialize_with_n<T, N>() && !try_initialize_with_n<T, N + 1>()) {
        return N;
    } else {
        return field_count_impl<T, N + 1>();
    }
}

struct UniversalType {
    template <typename T>
    operator T();
};

template <typename T, typename... Args>
consteval auto memberCount() {
    static_assert(std::is_aggregate_v<std::remove_cvref_t<T>>);

    if constexpr (requires { T{{Args{}}..., {UniversalType{}}}; } == false) {
        return sizeof...(Args);
    } else {
        return memberCount<T, Args..., UniversalType>();
    }
}

// template <typename T>
//     requires std::is_aggregate_v<T>
// static constexpr auto field_count = field_count_impl<T>();

template <typename T>
    requires std::is_aggregate_v<T>
static constexpr auto field_count = memberCount<T>();

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4101)
#endif

#define STRUCT_FIELD_TYPE_DEF(N)                                                                               \
    template <typename T, std::size_t I>                                                                       \
    constexpr auto __struct_field_type_impl(T my_struct, std::integral_constant<std::size_t, N>) {             \
        auto [NEKO_PP_PARAMS(x, N)] = my_struct;                                                               \
        return std::type_identity<std::tuple_element_t<I, std::tuple<NEKO_PP_CALL_PARAMS(decltype, x, N)>>>{}; \
    }

NEKO_PP_FOR_EACH(STRUCT_FIELD_TYPE_DEF, 63)

template <typename T, std::size_t I>
constexpr auto field_type_impl(T object) {
    constexpr auto N = field_count<T>;
    return __struct_field_type_impl<T, I>(object, std::integral_constant<std::size_t, N>{});
}

#define STRUCT_FIELD_ACCESS_DEF(N)                                                                          \
    template <std::size_t I>                                                                                \
    constexpr auto&& __struct_field_access_impl(auto&& my_struct, std::integral_constant<std::size_t, N>) { \
        auto&& [NEKO_PP_PARAMS(x, N)] = std::forward<decltype(my_struct)>(my_struct);                       \
        return std::get<I>(std::forward_as_tuple(NEKO_PP_PARAMS(x, N)));                                    \
    }

NEKO_PP_FOR_EACH(STRUCT_FIELD_ACCESS_DEF, 63)

template <std::size_t I>
constexpr auto&& field_access(auto&& object) {
    using T = std::remove_cvref_t<decltype(object)>;
    constexpr auto N = field_count<T>;
    return __struct_field_access_impl<I>(object, std::integral_constant<std::size_t, N>{});
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

template <typename T, std::size_t I>
constexpr auto field_name_impl() noexcept {
    constexpr auto name = main_name_of_pointer<wrapper{&field_access<I>(storage<T>)}>();
    return cstring<name.size()>{name};
}

template <typename T, std::size_t I>
    requires std::is_aggregate_v<T>
using field_type = typename decltype(field_type_impl<T, I>(std::declval<T>()))::type;

template <typename T, std::size_t I>
    requires std::is_aggregate_v<T>
static constexpr auto field_name = field_name_impl<T, I>();

struct MetaEnumStringView {
    constexpr MetaEnumStringView() : _data{}, _length(0) {}
    constexpr MetaEnumStringView(const char* _buffer, size_t _lenght) : _data(_buffer), _length(_lenght) {}
    constexpr const char* data() const { return _data; }
    constexpr size_t size() const { return _length; }
    constexpr char operator[](std::size_t _pos) const { return _data[_pos]; }
    const char* _data;
    size_t _length = 0;
};

template <typename T, size_t N>
struct MetaEnumArray {
    constexpr const T& operator[](std::size_t _pos) const { return _data[_pos]; }
    constexpr T& operator[](std::size_t _pos) { return _data[_pos]; }
    constexpr size_t size() const { return N; }
    T _data[N];
};

template <typename EnumType>
struct MetaEnumMember {
    EnumType value = {};
    MetaEnumStringView name;
    size_t index = {};
};

template <typename EnumType, typename UnderlyingTypeIn, size_t size>
struct MetaEnum {
    using UnderlyingType = UnderlyingTypeIn;
    MetaEnumArray<MetaEnumMember<EnumType>, size> members = {};
};

template <typename E>
struct MetaEnumTraits;

namespace details {
constexpr bool isNested(size_t brackets, bool quote) { return brackets != 0 || quote; }

constexpr size_t nextEnumCommaOrEnd(size_t start, MetaEnumStringView enumString) {
    size_t brackets = 0;  //()[]{}
    bool quote = false;   //""
    char lastChar = '\0';
    char nextChar = '\0';

    auto feedCounters = [&brackets, &quote, &lastChar, &nextChar](char c) {
        if (quote) {
            if (lastChar != '\\' && c == '"')  // ignore " if they are backslashed
                quote = false;
            return;
        }

        switch (c) {
            case '"':
                if (lastChar != '\\')  // ignore " if they are backslashed
                    quote = true;
                break;
            case '(':
            case '<':
                if (lastChar == '<' || nextChar == '<') break;
                [[fallthrough]];
            case '{':
                ++brackets;
                break;
            case ')':
            case '>':
                if (lastChar == '>' || nextChar == '>') break;
                [[fallthrough]];
            case '}':
                --brackets;
                break;
            default:
                break;
        }
    };

    size_t current = start;
    for (; current < enumString.size() && (isNested(brackets, quote) || (enumString[current] != ',')); ++current) {
        feedCounters(enumString[current]);
        lastChar = enumString[current];
        nextChar = current + 2 < enumString.size() ? enumString[current + 2] : '\0';
    }

    return current;
}

constexpr bool isAllowedIdentifierChar(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; }

constexpr MetaEnumStringView parseEnumMemberName(MetaEnumStringView memberString) {
    size_t nameStart = 0;
    while (!isAllowedIdentifierChar(memberString[nameStart])) {
        ++nameStart;
    }

    size_t nameSize = 0;

    while ((nameStart + nameSize) < memberString.size() && isAllowedIdentifierChar(memberString[nameStart + nameSize])) {
        ++nameSize;
    }

    return MetaEnumStringView(memberString.data() + nameStart, nameSize);
}

int constexpr length(const char* str) { return *str ? 1 + length(str + 1) : 0; }

template <typename EnumType, typename UnderlyingType, size_t size>
constexpr MetaEnum<EnumType, UnderlyingType, size> parseMetaEnum(const char* _in, int _length, const MetaEnumArray<EnumType, size>& values) {
    MetaEnumStringView in(_in, _length);
    MetaEnum<EnumType, UnderlyingType, size> result;

    MetaEnumArray<MetaEnumStringView, size> memberStrings;
    size_t amountFilled = 0;

    size_t currentStringStart = 0;

    while (amountFilled < size) {
        size_t currentStringEnd = nextEnumCommaOrEnd(currentStringStart + 1, in);
        size_t currentStringSize = currentStringEnd - currentStringStart;

        if (currentStringStart != 0) {
            ++currentStringStart;
            --currentStringSize;
        }

        memberStrings[amountFilled] = MetaEnumStringView(in.data() + currentStringStart, currentStringSize);
        ++amountFilled;
        currentStringStart = currentStringEnd;
    }

    for (size_t i = 0; i < memberStrings.size(); ++i) {
        result.members[i].name = parseEnumMemberName(memberStrings[i]);
        result.members[i].value = values[i];
        result.members[i].index = i;
    }

    return result;
}

template <typename EnumUnderlyingType>
struct IntWrapper {
    constexpr IntWrapper() : value(0), empty(true) {}
    constexpr IntWrapper(EnumUnderlyingType in) : value(in), empty(false) {}
    constexpr IntWrapper operator=(EnumUnderlyingType in) {
        value = in;
        empty = false;
        return *this;
    }
    EnumUnderlyingType value;
    bool empty;
};

template <typename EnumType, typename EnumUnderlyingType, size_t size>
constexpr MetaEnumArray<EnumType, size> resolveEnumValuesArray(const std::initializer_list<IntWrapper<EnumUnderlyingType>>& in) {
    MetaEnumArray<EnumType, size> result{};

    EnumUnderlyingType nextValue = 0;
    for (size_t i = 0; i < size; ++i) {
        auto wrapper = *(in.begin() + i);
        EnumUnderlyingType newValue = wrapper.empty ? nextValue : wrapper.value;
        nextValue = newValue + 1;
        result[i] = static_cast<EnumType>(newValue);
    }

    return result;
}
}  // namespace details

#define meta_enum(Type, UnderlyingType, ...)                                                                                                            \
    enum Type : UnderlyingType { __VA_ARGS__ };                                                                                                         \
    constexpr static auto Type##_internal_size = []() constexpr {                                                                                       \
        using IntWrapperType = details::IntWrapper<UnderlyingType>;                                                                                     \
        IntWrapperType __VA_ARGS__;                                                                                                                     \
        return std::initializer_list<IntWrapperType>{__VA_ARGS__}.size();                                                                               \
    };                                                                                                                                                  \
    constexpr static auto Type##_meta = details::parseMetaEnum<Type, UnderlyingType, Type##_internal_size()>(#__VA_ARGS__, sizeof(#__VA_ARGS__), []() { \
        using IntWrapperType = details::IntWrapper<UnderlyingType>;                                                                                     \
        IntWrapperType __VA_ARGS__;                                                                                                                     \
        return details::resolveEnumValuesArray<Type, UnderlyingType, Type##_internal_size()>({__VA_ARGS__});                                            \
    }());                                                                                                                                               \
    template <>                                                                                                                                         \
    struct ::MetaEnumTraits<Type> {                                                                                                                     \
        static const inline MetaEnum<Type, std::underlying_type_t<Type>, Type##_meta.members.size()> Meta = Type##_meta;                                \
    };

#define meta_enum_class(Type, UnderlyingType, ...)                                                                                                      \
    enum class Type : UnderlyingType { __VA_ARGS__ };                                                                                                   \
    constexpr static auto Type##_internal_size = []() constexpr {                                                                                       \
        using IntWrapperType = details::IntWrapper<UnderlyingType>;                                                                                     \
        IntWrapperType __VA_ARGS__;                                                                                                                     \
        return std::initializer_list<IntWrapperType>{__VA_ARGS__}.size();                                                                               \
    };                                                                                                                                                  \
    constexpr static auto Type##_meta = details::parseMetaEnum<Type, UnderlyingType, Type##_internal_size()>(#__VA_ARGS__, sizeof(#__VA_ARGS__), []() { \
        using IntWrapperType = details::IntWrapper<UnderlyingType>;                                                                                     \
        IntWrapperType __VA_ARGS__;                                                                                                                     \
        return details::resolveEnumValuesArray<Type, UnderlyingType, Type##_internal_size()>({__VA_ARGS__});                                            \
    }());                                                                                                                                               \
    template <>                                                                                                                                         \
    struct ::MetaEnumTraits<Type> {                                                                                                                     \
        static const inline MetaEnum<Type, std::underlying_type_t<Type>, Type##_meta.members.size()> Meta = Type##_meta;                                \
    };

template <typename Type>
constexpr const auto& getEnumMembers() {
    return MetaEnumTraits<Type>::Meta.members;
}

template <typename Type>
constexpr const size_t getEnumSize() {
    return MetaEnumTraits<Type>::Meta.members.size();
}

template <typename Type>
constexpr const std::string getEnumString(Type e) {
    const auto& members = getEnumMembers<Type>();
    for (auto i = 0; i < members.size(); ++i) {
        const auto& member = members[i];
        if (member.value == e) return std::string(member.name.data(), member.name.size());
    }
    return std::string{};
}

template <typename Type>
constexpr Type getEnumValue(unsigned int index) {
    const auto& members = getEnumMembers<Type>();
    if (index < members.size())
        return members[index].value;
    else
        return (Type)0;
}

}  // namespace reflection

}  // namespace Neko
