
#ifndef NEKO_TYPES_REFLECT_H
#define NEKO_TYPES_REFLECT_H

#include "engine/neko.h"

namespace neko {

#define CONCATENATE(ARG1, ARG2) _CONCATENATE(ARG1, ARG2)
#define _CONCATENATE(ARG1, ARG2) ARG1##ARG2

#define IF(CONDITION) CONCATENATE(_IF_, CONDITION)
#define _IF_0(TRUE, FALSE) FALSE
#define _IF_1(TRUE, FALSE) TRUE

#define HAS_PARENTHESIS(ARG) _HAS_PARENTHESIS(_HAS_PARENTHESIS_PROBE ARG)
#define _HAS_PARENTHESIS(...) _HAS_PARENTHESIS_CHECK_N(__VA_ARGS__, 0)
#define _HAS_PARENTHESIS_CHECK_N(ARG, N, ...) N
#define _HAS_PARENTHESIS_PROBE(...) 0, 1

#define COUNT_OF(...) _COUNT_OF(__VA_ARGS__, _COUNT_OF_RSEQ())
#define _COUNT_OF(...) _COUNT_OF_CHECK_N(__VA_ARGS__)
#define _COUNT_OF_CHECK_N(_01, _02, _03, _04, _05, _06, _07, _08, _09, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                          _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, N, ...)             \
    N
#define _COUNT_OF_RSEQ()                                                                                                                                                                            \
    64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
            16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00

#define FOR_EACH(ACTION, ...) CONCATENATE(_FOR_EACH, COUNT_OF(__VA_ARGS__))(ACTION, __VA_ARGS__)
#define _FOR_EACH16(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH15(ACTION, __VA_ARGS__)
#define _FOR_EACH15(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH14(ACTION, __VA_ARGS__)
#define _FOR_EACH14(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH13(ACTION, __VA_ARGS__)
#define _FOR_EACH13(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH12(ACTION, __VA_ARGS__)
#define _FOR_EACH12(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH11(ACTION, __VA_ARGS__)
#define _FOR_EACH11(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH10(ACTION, __VA_ARGS__)
#define _FOR_EACH10(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH09(ACTION, __VA_ARGS__)
#define _FOR_EACH09(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH08(ACTION, __VA_ARGS__)
#define _FOR_EACH08(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH07(ACTION, __VA_ARGS__)
#define _FOR_EACH07(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH06(ACTION, __VA_ARGS__)
#define _FOR_EACH06(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH05(ACTION, __VA_ARGS__)
#define _FOR_EACH05(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH04(ACTION, __VA_ARGS__)
#define _FOR_EACH04(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH03(ACTION, __VA_ARGS__)
#define _FOR_EACH03(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH02(ACTION, __VA_ARGS__)
#define _FOR_EACH02(ACTION, ARG, ...) ACTION(ARG), _FOR_EACH01(ACTION, __VA_ARGS__)
#define _FOR_EACH01(ACTION, ARG, ...) ACTION(ARG)

inline static constexpr const u64 __neko_reflect_max_name_length = 128;
inline static constexpr const s32 __neko_reflect_min_enum_value = -32;
inline static constexpr const s32 __neko_reflect_max_enum_value = 32;
inline static constexpr const s32 __neko_reflect_max_enum_value_count = __neko_reflect_max_enum_value - __neko_reflect_min_enum_value;

struct neko_type_enum_v {
    s32 index;
    const char *name;
};

struct neko_type_field_t {
    const char *name;
    u64 offset;
    const struct neko_type_meta_t *type;
    const char *tag;
};

struct neko_type_meta_t {
    const char *name;
    neko_type_kind kind;
    u64 size;
    u64 align;
    union {
        struct {
            const neko_type_meta_t *pointee;
        } as_pointer;
        struct {
            const neko_type_meta_t *element;
            u64 element_count;
        } as_array;
        struct {
            const neko_type_enum_v *values;
            u64 value_count;
        } as_enum;
        struct {
            const neko_type_field_t *fields;
            u64 field_count;
        } as_struct;
    };
};

struct neko_value {
    const void *data;
    const neko_type_meta_t *type;
};

inline static constexpr void __neko_reflect_append_name(char *name, u64 &count, std::string_view type_name) {
    constexpr auto string_append = [](char *string, const char *to_append, u64 &count) {
        while (*to_append != '\0' && count < __neko_reflect_max_name_length - 1) string[count++] = *to_append++;
    };

    constexpr auto append_type_name_prettified = [string_append](char *name, std::string_view type_name, u64 &count) {
        if (type_name.starts_with(' ')) type_name.remove_prefix(1);

        bool add_pointer = false;
        if (type_name.starts_with("const ")) {
            string_append(name, "const ", count);
            type_name.remove_prefix(6);
        } else if (type_name.ends_with(" const *")) {
            string_append(name, "const ", count);
            type_name.remove_suffix(8);
            add_pointer = true;
        }

#if defined(_MSC_VER)
        if (type_name.starts_with("enum "))
            type_name.remove_prefix(5);
        else if (type_name.starts_with("class "))
            type_name.remove_prefix(6);
        else if (type_name.starts_with("struct "))
            type_name.remove_prefix(7);
#endif

        if (type_name.starts_with("signed char")) {
            string_append(name, "s8", count);
            type_name.remove_prefix(11);
        } else if (type_name.starts_with("short int")) {
            string_append(name, "s16", count);
            type_name.remove_prefix(9);
        } else if (type_name.starts_with("short") && !type_name.starts_with("short unsigned int")) {
            string_append(name, "s16", count);
            type_name.remove_prefix(5);
        } else if (type_name.starts_with("int")) {
            string_append(name, "s32", count);
            type_name.remove_prefix(3);
        } else if (type_name.starts_with("__int64")) {
            string_append(name, "s64", count);
            type_name.remove_prefix(7);
        } else if (type_name.starts_with("long int")) {
            string_append(name, "s64", count);
            type_name.remove_prefix(8);
        } else if (type_name.starts_with("unsigned char")) {
            string_append(name, "u8", count);
            type_name.remove_prefix(13);
        } else if (type_name.starts_with("unsigned short")) {
            string_append(name, "u16", count);
            type_name.remove_prefix(14);
        } else if (type_name.starts_with("short unsigned int")) {
            string_append(name, "u16", count);
            type_name.remove_prefix(18);
        } else if (type_name.starts_with("unsigned int")) {
            string_append(name, "u32", count);
            type_name.remove_prefix(12);
        } else if (type_name.starts_with("unsigned __int64")) {
            string_append(name, "u64", count);
            type_name.remove_prefix(16);
        } else if (type_name.starts_with("long unsigned int")) {
            string_append(name, "u64", count);
            type_name.remove_prefix(17);
        } else if (type_name.starts_with("float")) {
            string_append(name, "f32", count);
            type_name.remove_prefix(5);
        } else if (type_name.starts_with("double")) {
            string_append(name, "f64", count);
            type_name.remove_prefix(6);
        }

        for (char c : type_name)
            if (c != ' ') name[count++] = c;

        if (add_pointer) name[count++] = '*';
    };

    bool add_const = false;
    bool add_pointer = false;
    bool add_reference = false;
    if (type_name.ends_with("* const")) {
        type_name.remove_suffix(7);
        add_const = true;
        add_pointer = true;
    }

    if (type_name.ends_with(" const ")) {
        string_append(name, "const ", count);
        type_name.remove_suffix(7);
    } else if (type_name.ends_with(" const *")) {
        string_append(name, "const ", count);
        type_name.remove_suffix(8);
        add_pointer = true;
    } else if (type_name.ends_with("const &")) {
        add_const = true;
        add_reference = true;
        type_name.remove_suffix(7);
    } else if (type_name.ends_with(" const&")) {
        add_const = true;
        add_reference = true;
        type_name.remove_suffix(7);
    } else if (type_name.ends_with('*')) {
        type_name.remove_suffix(1);
        add_pointer = true;
    } else if (type_name.ends_with('&')) {
        type_name.remove_suffix(1);
        add_reference = true;
    }

    if (type_name.ends_with(' ')) type_name.remove_suffix(1);

    if (type_name.ends_with('>')) {
        u64 open_angle_bracket_pos = type_name.find('<');
        append_type_name_prettified(name, type_name.substr(0, open_angle_bracket_pos), count);
        type_name.remove_prefix(open_angle_bracket_pos + 1);

        name[count++] = '<';
        u64 prev = 0;
        u64 match = 1;
        for (u64 c = 0; c < type_name.length(); ++c) {
            if (type_name.at(c) == '<') {
                ++match;
            }

            if (type_name.at(c) == '>') {
                --match;
                if (match <= 0) {
                    __neko_reflect_append_name(name, count, type_name.substr(prev, c - prev));
                    name[count++] = '>';
                    prev = c + 1;
                }
            }

            if (type_name.at(c) == ',') {
                __neko_reflect_append_name(name, count, type_name.substr(prev, c - prev));
                name[count++] = ',';
                prev = c + 1;
            }
        }
    } else {
        append_type_name_prettified(name, type_name, count);
    }

    if (add_pointer) name[count++] = '*';
    if (add_const) string_append(name, " const", count);
    if (add_reference) name[count++] = '&';
}

template <typename T>
inline static constexpr const char *name_of() {
    if constexpr (std::is_same_v<T, s8>)
        return "s8";
    else if constexpr (std::is_same_v<T, s16>)
        return "s16";
    else if constexpr (std::is_same_v<T, s32>)
        return "s32";
    else if constexpr (std::is_same_v<T, s64>)
        return "s64";
    else if constexpr (std::is_same_v<T, u8>)
        return "u8";
    else if constexpr (std::is_same_v<T, u16>)
        return "u16";
    else if constexpr (std::is_same_v<T, u32>)
        return "u32";
    else if constexpr (std::is_same_v<T, u64>)
        return "u64";
    else if constexpr (std::is_same_v<T, f32>)
        return "f32";
    else if constexpr (std::is_same_v<T, f64>)
        return "f64";
    else if constexpr (std::is_same_v<T, bool>)
        return "bool";
    else if constexpr (std::is_same_v<T, char>)
        return "char";
    else if constexpr (std::is_same_v<T, void>)
        return "void";
    else {
        constexpr auto get_type_name = [](std::string_view type_name) -> const char * {
            static char name[__neko_reflect_max_name_length] = {};
            u64 count = 0;
            __neko_reflect_append_name(name, count, type_name);
            return name;
        };

#if defined(_MSC_VER)
        constexpr auto type_function_name = std::string_view{__FUNCSIG__};
        constexpr auto type_name_prefix_length = type_function_name.find("name_of<") + 8;
        constexpr auto type_name_length = type_function_name.rfind(">") - type_name_prefix_length;
#elif defined(__GNUC__)
        constexpr auto type_function_name = std::string_view{__PRETTY_FUNCTION__};
        constexpr auto type_name_prefix_length = type_function_name.find("= ") + 2;
        constexpr auto type_name_length = type_function_name.rfind("]") - type_name_prefix_length;
#endif
        static const char *name = get_type_name({type_function_name.data() + type_name_prefix_length, type_name_length});
        return name;
    }
}

template <typename T>
inline static constexpr neko_type_kind kind_of() {
    using Type = std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<Type, s8>)
        return TYPE_KIND_S8;
    else if constexpr (std::is_same_v<Type, s16>)
        return TYPE_KIND_S16;
    else if constexpr (std::is_same_v<Type, s32>)
        return TYPE_KIND_S32;
    else if constexpr (std::is_same_v<Type, s64>)
        return TYPE_KIND_S64;
    else if constexpr (std::is_same_v<Type, u8>)
        return TYPE_KIND_U8;
    else if constexpr (std::is_same_v<Type, u16>)
        return TYPE_KIND_U16;
    else if constexpr (std::is_same_v<Type, u32>)
        return TYPE_KIND_U32;
    else if constexpr (std::is_same_v<Type, u64>)
        return TYPE_KIND_U64;
    else if constexpr (std::is_same_v<Type, f32>)
        return TYPE_KIND_F32;
    else if constexpr (std::is_same_v<Type, f64>)
        return TYPE_KIND_F64;
    else if constexpr (std::is_same_v<Type, bool>)
        return TYPE_KIND_BOOL;
    else if constexpr (std::is_same_v<Type, char>)
        return TYPE_KIND_CHAR;
    else if constexpr (std::is_same_v<Type, void>)
        return TYPE_KIND_VOID;
    else if constexpr (std::is_pointer_v<Type>)
        return TYPE_KIND_POINTER;
    else if constexpr (std::is_array_v<Type>)
        return TYPE_KIND_ARRAY;
    else if constexpr (std::is_enum_v<Type>)
        return TYPE_KIND_ENUM;
    else if constexpr (std::is_compound_v<Type>)
        return TYPE_KIND_STRUCT;
}

template <typename T>
inline static constexpr neko_type_kind kind_of(T &&) {
    return kind_of<T>();
}

template <typename T>
inline static constexpr const neko_type_meta_t *neko_typeof(const T) {
    static_assert(sizeof(T) == 0, "没有为此类型定义neko_typeof(const T)函数重载");
    return nullptr;
}

template <typename T>
    requires(std::is_fundamental_v<T> && !std::is_void_v<T>)
inline static constexpr const neko_type_meta_t *neko_typeof(const T) {
    static const neko_type_meta_t self = {.name = name_of<T>(), .kind = kind_of<T>(), .size = sizeof(T), .align = alignof(T), .as_struct = {}};
    return &self;
}

template <typename T>
    requires(std::is_void_v<T>)
inline static constexpr const neko_type_meta_t *neko_typeof() {
    static const neko_type_meta_t self = {.name = name_of<T>(), .kind = kind_of<T>(), .size = 0, .align = 0, .as_struct = {}};
    return &self;
}

template <typename T>
    requires(std::is_pointer_v<T>)
inline static constexpr const neko_type_meta_t *neko_typeof(const T) {
    using Pointee = std::remove_pointer_t<T>;
    static const neko_type_meta_t *pointee = nullptr;
    if constexpr (std::is_void_v<Pointee>)
        pointee = neko_typeof<Pointee>();
    else if constexpr (!std::is_abstract_v<Pointee>)
        pointee = neko_typeof(Pointee{});
    static const neko_type_meta_t self = {.name = name_of<T>(), .kind = kind_of<T>(), .size = sizeof(T), .align = alignof(T), .as_pointer = pointee};
    return &self;
}

template <typename T, u64 N>
inline static constexpr const neko_type_meta_t *neko_typeof(const T (&)[N]) {
    static const neko_type_meta_t self = {.name = name_of<T[N]>(), .kind = kind_of<T[N]>(), .size = sizeof(T[N]), .align = alignof(T[N]), .as_array = {neko_typeof(T{}), N}};
    return &self;
}

#define _TYPE_OF_ENUM(VALUE) \
    { (s32) VALUE, #VALUE }

#define TYPE_OF_ENUM(T, ...)                                                                                                                                                     \
    inline static const neko_type_meta_t *neko_typeof(const T) {                                                                                                                 \
        __VA_OPT__(static const neko_type_enum_v values[] = {FOR_EACH(_TYPE_OF_ENUM, __VA_ARGS__)};)                                                                             \
        static const neko_type_meta_t self = {                                                                                                                                   \
                .name = name_of<T>(), .kind = kind_of<T>(), .size = sizeof(T), .align = alignof(T), .as_enum = {__VA_OPT__(values, sizeof(values) / sizeof(neko_type_enum_v))}}; \
        return &self;                                                                                                                                                            \
    }

template <typename T>
    requires(std::is_enum_v<T>)
inline static constexpr const neko_type_meta_t *neko_typeof(const T) {

    struct __enum_v {
        s32 index;
        std::string_view name;
    };

    struct __enum {
        std::array<__enum_v, __neko_reflect_max_enum_value_count> values;
        u64 count;
    };

    constexpr auto get_enum_value = []<T V>() -> __enum_v {
#if defined(_MSC_VER)
        constexpr auto type_function_name = std::string_view{__FUNCSIG__};
        constexpr auto type_name_prefix_length = type_function_name.find("()<") + 3;
        constexpr auto type_name_length = type_function_name.find(">", type_name_prefix_length) - type_name_prefix_length;
#elif defined(__GNUC__)
        constexpr auto type_function_name = std::string_view{__PRETTY_FUNCTION__};
        constexpr auto type_name_prefix_length = type_function_name.find("= ") + 2;
#endif

        char c = type_function_name.at(type_name_prefix_length);
        if ((c >= '0' && c <= '9') || c == '(' || c == ')') return {};
        return {(s32)V, {type_function_name.data() + type_name_prefix_length, type_name_length}};
    };

    constexpr auto data = [get_enum_value]<s32... I>(std::integer_sequence<s32, I...>) -> __enum {
        return {{get_enum_value.template operator()<(T)(I + __neko_reflect_min_enum_value)>()...}, ((get_enum_value.template operator()<(T)(I + __neko_reflect_min_enum_value)>().name != "") + ...)};
    }(std::make_integer_sequence<s32, __neko_reflect_max_enum_value_count>());

    constexpr auto copy = [](char *dst, const char *src, u64 count) {
        for (u64 i = 0; i < count; ++i) dst[i] = src[i];
    };

    static neko_type_enum_v values[data.count] = {};
    static char names[data.count][__neko_reflect_max_name_length] = {};

    static bool initialized = false;
    if (initialized == false) {
        for (u64 i = 0, c = 0; i < __neko_reflect_max_enum_value_count; ++i) {
            if (const auto &value = data.values[i]; value.name != "") {
                values[c].index = value.index;
                copy(names[c], value.name.data(), value.name.length());
                values[c].name = names[c];
                ++c;
            }
        }
        initialized = true;
    }

    static const neko_type_meta_t self = {.name = name_of<T>(), .kind = kind_of<T>(), .size = sizeof(T), .align = alignof(T), .as_enum = {values, data.count}};
    return &self;
}

#define _TYPE_OF(NAME) IF(HAS_PARENTHESIS(NAME))(_TYPE_OF_HELPER NAME, _TYPE_OF_HELPER(NAME))
#define _TYPE_OF_HELPER(NAME, ...) \
    { #NAME, offsetof(TYPE, NAME), neko_typeof(t.NAME), "" __VA_ARGS__ }

#define _NAME_OF(T) IF(HAS_PARENTHESIS(T))(_NAME_OF_HELPER T, _NAME_OF_HELPER(T))
#define _NAME_OF_HELPER(...) __VA_ARGS__

#define neko_typeof_decl(T, ...)                                                                                                                                                                \
    inline const neko_type_meta_t *neko_typeof(const _NAME_OF(T)) {                                                                                                                             \
        static const neko_type_meta_t self = {.name = name_of<_NAME_OF(T)>(), .kind = kind_of<_NAME_OF(T)>(), .size = sizeof(_NAME_OF(T)), .align = alignof(_NAME_OF(T)), .as_struct = {}};     \
        __VA_OPT__(static bool initialized = false; if (initialized) return &self; initialized = true; using TYPE = _NAME_OF(T); TYPE t = {};                                                   \
                   static const neko_type_field_t fields[] = {FOR_EACH(_TYPE_OF, __VA_ARGS__)}; ((neko_type_meta_t *)&self)->as_struct = {fields, sizeof(fields) / sizeof(neko_type_field_t)};) \
        return &self;                                                                                                                                                                           \
    }

#define TYPE_OF_MEMBER(T) friend inline const neko_type_meta_t *neko_typeof(const _NAME_OF(T));

template <typename T>
inline static constexpr const neko_type_meta_t *neko_typeof() {
    return neko_typeof(T{});
}

template <typename T>
inline static constexpr const neko_value neko_valueof(T &&type) {
    T t = type;
    return {&type, neko_typeof(t)};
}

neko_typeof_decl(neko_type_field_t, name, offset, type);
neko_typeof_decl(neko_type_meta_t, name, kind, size, align, as_struct.fields, as_struct.field_count);
neko_typeof_decl(neko_value, data, type);

}  // namespace neko

#endif  // !NEKO_TYPES_REFLECT_H
