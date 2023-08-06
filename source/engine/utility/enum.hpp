// Metadot enum module is enhanced based on enum.hpp modification
// Metadot code Copyright(c) 2022-2023, KaoruXun All rights reserved.
// enum.hpp code by Matvey Cherevko licensed under the MIT License
// https://github.com/blackmatov/enum.hpp

#ifndef NEKO_ENUM_HPP
#define NEKO_ENUM_HPP

#include <array>
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#if !defined(__cpp_exceptions) && !defined(__EXCEPTIONS) && !defined(_CPPUNWIND)
#define ENUM_HPP_NO_EXCEPTIONS
#endif

namespace neko::cpp {
constexpr std::size_t invalid_index = std::size_t(-1);
constexpr std::string_view empty_string = std::string_view();

class exception final : public std::runtime_error {
public:
    explicit exception(const char* what) : std::runtime_error(what) {}
};
}  // namespace neko::cpp

namespace neko::cpp {
template <typename Enum>
using traits_t = decltype(enum_hpp_adl_find_registered_traits(std::declval<Enum>()));

template <typename Enum>
using underlying_type = typename traits_t<Enum>::underlying_type;

template <typename Enum>
constexpr std::size_t size() noexcept {
    return traits_t<Enum>::size;
}

template <typename Enum>
constexpr const std::array<Enum, size<Enum>()>& values() noexcept {
    return traits_t<Enum>::values;
}

template <typename Enum>
constexpr const std::array<std::string_view, size<Enum>()>& names() noexcept {
    return traits_t<Enum>::names;
}

template <typename Enum>
constexpr typename traits_t<Enum>::underlying_type to_underlying(Enum e) noexcept {
    return traits_t<Enum>::to_underlying(e);
}

template <typename Enum>
constexpr std::optional<std::string_view> to_string(Enum e) noexcept {
    return traits_t<Enum>::to_string(e);
}

template <typename Enum>
constexpr std::string_view to_string_or_empty(Enum e) noexcept {
    return traits_t<Enum>::to_string_or_empty(e);
}

template <typename Enum>
std::string_view to_string_or_throw(Enum e) {
    return traits_t<Enum>::to_string_or_throw(e);
}

template <typename Enum>
constexpr std::optional<Enum> from_string(std::string_view name) noexcept {
    return traits_t<Enum>::from_string(name);
}

template <typename Enum>
constexpr Enum from_string_or_default(std::string_view name, Enum def) noexcept {
    return traits_t<Enum>::from_string_or_default(name, def);
}

template <typename Enum>
Enum from_string_or_throw(std::string_view name) {
    return traits_t<Enum>::from_string_or_throw(name);
}

template <typename Enum>
constexpr std::optional<std::size_t> to_index(Enum e) noexcept {
    return traits_t<Enum>::to_index(e);
}

template <typename Enum>
constexpr std::size_t to_index_or_invalid(Enum e) noexcept {
    return traits_t<Enum>::to_index_or_invalid(e);
}

template <typename Enum>
std::size_t to_index_or_throw(Enum e) {
    return traits_t<Enum>::to_index_or_throw(e);
}

template <typename Enum>
constexpr std::optional<Enum> from_index(std::size_t index) noexcept {
    return traits_t<Enum>::from_index(index);
}

template <typename Enum>
constexpr Enum from_index_or_default(std::size_t index, Enum def) noexcept {
    return traits_t<Enum>::from_index_or_default(index, def);
}

template <typename Enum>
Enum from_index_or_throw(std::size_t index) {
    return traits_t<Enum>::from_index_or_throw(index);
}
}  // namespace neko::cpp

namespace neko::cpp::detail {
inline void throw_exception_with [[noreturn]] (const char* what) {
#ifndef ENUM_HPP_NO_EXCEPTIONS
    throw ::neko::cpp::exception(what);
#else
    (void)what;
    std::abort();
#endif
}

template <typename Enum>
struct ignore_assign final {
    Enum value;

    constexpr explicit ignore_assign(Enum value) noexcept : value(value) {}

    template <typename Other>
    // NOLINTNEXTLINE(readability-named-parameter)
    constexpr ignore_assign& operator=(const Other&) noexcept {
        return *this;
    }
};

constexpr bool is_end_of_name(char ch) noexcept {
    switch (ch) {
        case ' ':
        case '=':
        case '\r':
        case '\n':
        case '\t':
            return true;
        default:
            return false;
    }
}

constexpr std::string_view trim_raw_name(std::string_view raw_name) noexcept {
    for (std::size_t i = 0; i < raw_name.size(); ++i) {
        if (is_end_of_name(raw_name[i])) {
            return raw_name.substr(0, i);
        }
    }
    return raw_name;
}
}  // namespace neko::cpp::detail

//
// ENUM_HPP_GENERATE_FIELDS
//

#define ENUM_HPP_GENERATE_FIELDS_OP(d, i, x) x,

#define ENUM_HPP_GENERATE_FIELDS(Fields) ENUM_HPP_PP_SEQ_FOR_EACH(ENUM_HPP_GENERATE_FIELDS_OP, _, Fields)

//
// ENUM_HPP_GENERATE_VALUES
//

#define ENUM_HPP_GENERATE_VALUES_OP(Enum, i, x) ((::neko::cpp::detail::ignore_assign<Enum>)Enum::x).value,

#define ENUM_HPP_GENERATE_VALUES(Enum, Fields) ENUM_HPP_PP_SEQ_FOR_EACH(ENUM_HPP_GENERATE_VALUES_OP, Enum, Fields)

//
// ENUM_HPP_GENERATE_NAMES
//

#define ENUM_HPP_GENERATE_NAMES_OP(d, i, x) ::neko::cpp::detail::trim_raw_name(ENUM_HPP_PP_STRINGIZE(x)),

#define ENUM_HPP_GENERATE_NAMES(Fields) ENUM_HPP_PP_SEQ_FOR_EACH(ENUM_HPP_GENERATE_NAMES_OP, _, Fields)

//
// ENUM_HPP_GENERATE_VALUE_TO_NAME_CASES
//

#define ENUM_HPP_GENERATE_VALUE_TO_NAME_CASES_OP(Enum, i, x) \
    case values[i]:                                          \
        return names[i];

#define ENUM_HPP_GENERATE_VALUE_TO_NAME_CASES(Enum, Fields) ENUM_HPP_PP_SEQ_FOR_EACH(ENUM_HPP_GENERATE_VALUE_TO_NAME_CASES_OP, Enum, Fields)

//
// ENUM_HPP_GENERATE_VALUE_TO_INDEX_CASES
//

#define ENUM_HPP_GENERATE_VALUE_TO_INDEX_CASES_OP(Enum, i, x) \
    case values[i]:                                           \
        return i;

#define ENUM_HPP_GENERATE_VALUE_TO_INDEX_CASES(Enum, Fields) ENUM_HPP_PP_SEQ_FOR_EACH(ENUM_HPP_GENERATE_VALUE_TO_INDEX_CASES_OP, Enum, Fields)

//
// ENUM_HPP_DECL
//

#define ENUM_HPP_DECL(Enum, Type, Fields)                  \
    enum Enum : Type { ENUM_HPP_GENERATE_FIELDS(Fields) }; \
    ENUM_HPP_TRAITS_DECL(Enum, Fields)

//
// ENUM_HPP_CLASS_DECL
//

#define ENUM_HPP_CLASS_DECL(Enum, Type, Fields)                  \
    enum class Enum : Type { ENUM_HPP_GENERATE_FIELDS(Fields) }; \
    ENUM_HPP_TRAITS_DECL(Enum, Fields)

//
// ENUM_HPP_TRAITS_DECL
//

#define ENUM_HPP_TRAITS_DECL(Enum, Fields)                                                                                                \
    struct Enum##_traits final {                                                                                                          \
    private:                                                                                                                              \
        enum enum_names_for_this_score_ { ENUM_HPP_GENERATE_FIELDS(Fields) };                                                             \
                                                                                                                                          \
    public:                                                                                                                               \
        using enum_type = Enum;                                                                                                           \
        using underlying_type = std::underlying_type_t<enum_type>;                                                                        \
        static constexpr std::size_t size = ENUM_HPP_PP_SEQ_SIZE(Fields);                                                                 \
                                                                                                                                          \
        static constexpr const std::array<enum_type, size> values = {{ENUM_HPP_GENERATE_VALUES(Enum, Fields)}};                           \
                                                                                                                                          \
        static constexpr const std::array<std::string_view, size> names = {{ENUM_HPP_GENERATE_NAMES(Fields)}};                            \
                                                                                                                                          \
    public:                                                                                                                               \
        [[maybe_unused]] static constexpr underlying_type to_underlying(enum_type e) noexcept { return static_cast<underlying_type>(e); } \
        [[maybe_unused]] static constexpr std::optional<std::string_view> to_string(enum_type e) noexcept {                               \
            switch (e) {                                                                                                                  \
                ENUM_HPP_GENERATE_VALUE_TO_NAME_CASES(Enum, Fields)                                                                       \
                default:                                                                                                                  \
                    return std::nullopt;                                                                                                  \
            }                                                                                                                             \
        }                                                                                                                                 \
        [[maybe_unused]] static constexpr std::string_view to_string_or_empty(enum_type e) noexcept {                                     \
            if (auto s = to_string(e)) {                                                                                                  \
                return *s;                                                                                                                \
            }                                                                                                                             \
            return ::neko::cpp::empty_string;                                                                                               \
        }                                                                                                                                 \
        [[maybe_unused]] static std::string_view to_string_or_throw(enum_type e) {                                                        \
            if (auto s = to_string(e)) {                                                                                                  \
                return *s;                                                                                                                \
            }                                                                                                                             \
            ::neko::cpp::detail::throw_exception_with(#Enum "_traits::to_string_or_throw(): invalid argument");                             \
        }                                                                                                                                 \
        [[maybe_unused]] static constexpr std::optional<enum_type> from_string(std::string_view name) noexcept {                          \
            for (std::size_t i = 0; i < size; ++i) {                                                                                      \
                if (name == names[i]) {                                                                                                   \
                    return values[i];                                                                                                     \
                }                                                                                                                         \
            }                                                                                                                             \
            return std::nullopt;                                                                                                          \
        }                                                                                                                                 \
        [[maybe_unused]] static constexpr enum_type from_string_or_default(std::string_view name, enum_type def) noexcept {               \
            if (auto e = from_string(name)) {                                                                                             \
                return *e;                                                                                                                \
            }                                                                                                                             \
            return def;                                                                                                                   \
        }                                                                                                                                 \
        [[maybe_unused]] static enum_type from_string_or_throw(std::string_view name) {                                                   \
            if (auto e = from_string(name)) {                                                                                             \
                return *e;                                                                                                                \
            }                                                                                                                             \
            ::neko::cpp::detail::throw_exception_with(#Enum "_traits::from_string_or_throw(): invalid argument");                           \
        }                                                                                                                                 \
        [[maybe_unused]] static constexpr std::optional<std::size_t> to_index(enum_type e) noexcept {                                     \
            switch (e) {                                                                                                                  \
                ENUM_HPP_GENERATE_VALUE_TO_INDEX_CASES(Enum, Fields)                                                                      \
                default:                                                                                                                  \
                    return std::nullopt;                                                                                                  \
            }                                                                                                                             \
        }                                                                                                                                 \
        [[maybe_unused]] static constexpr std::size_t to_index_or_invalid(enum_type e) noexcept {                                         \
            if (auto i = to_index(e)) {                                                                                                   \
                return *i;                                                                                                                \
            }                                                                                                                             \
            return ::neko::cpp::invalid_index;                                                                                              \
        }                                                                                                                                 \
        [[maybe_unused]] static std::size_t to_index_or_throw(enum_type e) {                                                              \
            if (auto i = to_index(e)) {                                                                                                   \
                return *i;                                                                                                                \
            }                                                                                                                             \
            ::neko::cpp::detail::throw_exception_with(#Enum "_traits::to_index_or_throw(): invalid argument");                              \
        }                                                                                                                                 \
        [[maybe_unused]] static constexpr std::optional<enum_type> from_index(std::size_t index) noexcept {                               \
            if (index < size) {                                                                                                           \
                return values[index];                                                                                                     \
            }                                                                                                                             \
            return std::nullopt;                                                                                                          \
        }                                                                                                                                 \
        [[maybe_unused]] static constexpr enum_type from_index_or_default(std::size_t index, enum_type def) noexcept {                    \
            if (auto e = from_index(index)) {                                                                                             \
                return *e;                                                                                                                \
            }                                                                                                                             \
            return def;                                                                                                                   \
        }                                                                                                                                 \
        [[maybe_unused]] static enum_type from_index_or_throw(std::size_t index) {                                                        \
            if (auto e = from_index(index)) {                                                                                             \
                return *e;                                                                                                                \
            }                                                                                                                             \
            ::neko::cpp::detail::throw_exception_with(#Enum "_traits::from_index_or_throw(): invalid argument");                            \
        }                                                                                                                                 \
    }

//
// ENUM_HPP_REGISTER_TRAITS
//

#define ENUM_HPP_REGISTER_TRAITS(Enum) \
    constexpr Enum##_traits enum_hpp_adl_find_registered_traits [[maybe_unused]] (Enum) noexcept { return Enum##_traits{}; }

#include "enum_pp.inl"

#pragma region bitflag

namespace neko::cpp::bitflags {
template <typename Enum>
class bitflags final {
    static_assert(std::is_enum_v<Enum>);

public:
    using enum_type = Enum;
    using underlying_type = std::underlying_type_t<Enum>;

    bitflags() = default;
    bitflags(const bitflags&) = default;
    bitflags& operator=(const bitflags&) = default;
    bitflags(bitflags&&) noexcept = default;
    bitflags& operator=(bitflags&&) noexcept = default;
    ~bitflags() = default;

    constexpr bitflags(enum_type flags) : flags_(static_cast<underlying_type>(flags)) {}

    constexpr explicit bitflags(underlying_type flags) : flags_(flags) {}

    constexpr void swap(bitflags& other) noexcept {
        using std::swap;
        swap(flags_, other.flags_);
    }

    constexpr explicit operator bool() const noexcept { return !!flags_; }

    constexpr underlying_type as_raw() const noexcept { return flags_; }

    constexpr enum_type as_enum() const noexcept { return static_cast<enum_type>(flags_); }

    constexpr bool has(bitflags flags) const noexcept { return flags.flags_ == (flags_ & flags.flags_); }

    constexpr bitflags& set(bitflags flags) noexcept {
        flags_ |= flags.flags_;
        return *this;
    }

    constexpr bitflags& toggle(bitflags flags) noexcept {
        flags_ ^= flags.flags_;
        return *this;
    }

    constexpr bitflags& clear(bitflags flags) noexcept {
        flags_ &= ~flags.flags_;
        return *this;
    }

private:
    underlying_type flags_{};
};

template <typename Enum>
constexpr void swap(bitflags<Enum>& l, bitflags<Enum>& r) noexcept {
    l.swap(r);
}
}  // namespace neko::cpp::bitflags

namespace std {
template <typename Enum>
struct hash<neko::cpp::bitflags::bitflags<Enum>> {
    size_t operator()(neko::cpp::bitflags::bitflags<Enum> bf) const noexcept { return hash<Enum>{}(bf.as_enum()); }
};
}  // namespace std

namespace neko::cpp::bitflags {
#define ENUM_HPP_DEFINE_BINARY_OPERATOR(op)                                                 \
    template <typename Enum>                                                                \
    constexpr bool operator op(Enum l, bitflags<Enum> r) noexcept {                         \
        return l op r.as_enum();                                                            \
    }                                                                                       \
    template <typename Enum>                                                                \
    constexpr bool operator op(bitflags<Enum> l, Enum r) noexcept {                         \
        return l.as_enum() op r;                                                            \
    }                                                                                       \
    template <typename Enum>                                                                \
    constexpr bool operator op(std::underlying_type_t<Enum> l, bitflags<Enum> r) noexcept { \
        return l op r.as_raw();                                                             \
    }                                                                                       \
    template <typename Enum>                                                                \
    constexpr bool operator op(bitflags<Enum> l, std::underlying_type_t<Enum> r) noexcept { \
        return l.as_raw() op r;                                                             \
    }                                                                                       \
    template <typename Enum>                                                                \
    constexpr bool operator op(bitflags<Enum> l, bitflags<Enum> r) noexcept {               \
        return l.as_raw() op r.as_raw();                                                    \
    }
ENUM_HPP_DEFINE_BINARY_OPERATOR(<)
ENUM_HPP_DEFINE_BINARY_OPERATOR(>)
ENUM_HPP_DEFINE_BINARY_OPERATOR(<=)
ENUM_HPP_DEFINE_BINARY_OPERATOR(>=)
ENUM_HPP_DEFINE_BINARY_OPERATOR(==)
ENUM_HPP_DEFINE_BINARY_OPERATOR(!=)
#undef ENUM_HPP_DEFINE_BINARY_OPERATOR
}  // namespace neko::cpp::bitflags

namespace neko::cpp::bitflags {
template <typename Enum>
constexpr bitflags<Enum> operator~(bitflags<Enum> l) noexcept {
    return static_cast<Enum>(~l.as_raw());
}

#define ENUM_HPP_DEFINE_BINARY_OPERATOR(op)                                                  \
    template <typename Enum>                                                                 \
    constexpr bitflags<Enum> operator op(Enum l, bitflags<Enum> r) noexcept {                \
        return bitflags{l} op r;                                                             \
    }                                                                                        \
    template <typename Enum>                                                                 \
    constexpr bitflags<Enum> operator op(bitflags<Enum> l, Enum r) noexcept {                \
        return l op bitflags{r};                                                             \
    }                                                                                        \
    template <typename Enum>                                                                 \
    constexpr bitflags<Enum> operator op(bitflags<Enum> l, bitflags<Enum> r) noexcept {      \
        return static_cast<Enum>(l.as_raw() op r.as_raw());                                  \
    }                                                                                        \
    template <typename Enum>                                                                 \
    constexpr bitflags<Enum>& operator op##=(bitflags<Enum>& l, Enum r) noexcept {           \
        return l = l op bitflags{r};                                                         \
    }                                                                                        \
    template <typename Enum>                                                                 \
    constexpr bitflags<Enum>& operator op##=(bitflags<Enum>& l, bitflags<Enum> r) noexcept { \
        return l = l op r;                                                                   \
    }
ENUM_HPP_DEFINE_BINARY_OPERATOR(|)
ENUM_HPP_DEFINE_BINARY_OPERATOR(&)
ENUM_HPP_DEFINE_BINARY_OPERATOR(^)
#undef ENUM_HPP_DEFINE_BINARY_OPERATOR
}  // namespace neko::cpp::bitflags

namespace neko::cpp::bitflags {
//
// any
//

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int> = 0>
constexpr bool any(Enum flags) noexcept {
    return any(bitflags{flags});
}

template <typename Enum>
constexpr bool any(bitflags<Enum> flags) noexcept {
    return 0 != flags.as_raw();
}

//
// none
//

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int> = 0>
constexpr bool none(Enum flags) noexcept {
    return none(bitflags{flags});
}

template <typename Enum>
constexpr bool none(bitflags<Enum> flags) noexcept {
    return 0 == flags.as_raw();
}

//
// all_of
//

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int> = 0>
constexpr bool all_of(Enum flags, Enum mask) noexcept {
    return all_of(bitflags{flags}, bitflags{mask});
}

template <typename Enum>
constexpr bool all_of(Enum flags, bitflags<Enum> mask) noexcept {
    return all_of(bitflags{flags}, mask);
}

template <typename Enum>
constexpr bool all_of(bitflags<Enum> flags, Enum mask) noexcept {
    return all_of(flags, bitflags{mask});
}

template <typename Enum>
constexpr bool all_of(bitflags<Enum> flags, bitflags<Enum> mask) noexcept {
    return (flags.as_raw() & mask.as_raw()) == mask.as_raw();
}

//
// any_of
//

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int> = 0>
constexpr bool any_of(Enum flags, Enum mask) noexcept {
    return any_of(bitflags{flags}, bitflags{mask});
}

template <typename Enum>
constexpr bool any_of(Enum flags, bitflags<Enum> mask) noexcept {
    return any_of(bitflags{flags}, mask);
}

template <typename Enum>
constexpr bool any_of(bitflags<Enum> flags, Enum mask) noexcept {
    return any_of(flags, bitflags{mask});
}

template <typename Enum>
constexpr bool any_of(bitflags<Enum> flags, bitflags<Enum> mask) noexcept {
    return 0 != (flags.as_raw() & mask.as_raw());
}

//
// none_of
//

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int> = 0>
constexpr bool none_of(Enum flags, Enum mask) noexcept {
    return none_of(bitflags{flags}, bitflags{mask});
}

template <typename Enum>
constexpr bool none_of(Enum flags, bitflags<Enum> mask) noexcept {
    return none_of(bitflags{flags}, mask);
}

template <typename Enum>
constexpr bool none_of(bitflags<Enum> flags, Enum mask) noexcept {
    return none_of(flags, bitflags{mask});
}

template <typename Enum>
constexpr bool none_of(bitflags<Enum> flags, bitflags<Enum> mask) noexcept {
    return 0 == (flags.as_raw() & mask.as_raw());
}

//
// any_except
//

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int> = 0>
constexpr bool any_except(Enum flags, Enum mask) noexcept {
    return any_except(bitflags{flags}, bitflags{mask});
}

template <typename Enum>
constexpr bool any_except(Enum flags, bitflags<Enum> mask) noexcept {
    return any_except(bitflags{flags}, mask);
}

template <typename Enum>
constexpr bool any_except(bitflags<Enum> flags, Enum mask) noexcept {
    return any_except(flags, bitflags{mask});
}

template <typename Enum>
constexpr bool any_except(bitflags<Enum> flags, bitflags<Enum> mask) noexcept {
    return any_of(flags, ~mask);
}

//
// none_except
//

template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>, int> = 0>
constexpr bool none_except(Enum flags, Enum mask) noexcept {
    return none_except(bitflags{flags}, bitflags{mask});
}

template <typename Enum>
constexpr bool none_except(Enum flags, bitflags<Enum> mask) noexcept {
    return none_except(bitflags{flags}, mask);
}

template <typename Enum>
constexpr bool none_except(bitflags<Enum> flags, Enum mask) noexcept {
    return none_except(flags, bitflags{mask});
}

template <typename Enum>
constexpr bool none_except(bitflags<Enum> flags, bitflags<Enum> mask) noexcept {
    return none_of(flags, ~mask);
}
}  // namespace neko::cpp::bitflags

//
// ENUM_HPP_OPERATORS_DECL
//

#define ENUM_HPP_OPERATORS_DECL(Enum)                                                                                                                                                  \
    constexpr ::neko::cpp::bitflags::bitflags<Enum> operator~[[maybe_unused]] (Enum l) noexcept { return ~::neko::cpp::bitflags::bitflags(l); }                                            \
    constexpr ::neko::cpp::bitflags::bitflags<Enum> operator| [[maybe_unused]] (Enum l, Enum r) noexcept { return ::neko::cpp::bitflags::bitflags(l) | ::neko::cpp::bitflags::bitflags(r); } \
    constexpr ::neko::cpp::bitflags::bitflags<Enum> operator& [[maybe_unused]] (Enum l, Enum r) noexcept { return ::neko::cpp::bitflags::bitflags(l) & ::neko::cpp::bitflags::bitflags(r); } \
    constexpr ::neko::cpp::bitflags::bitflags<Enum> operator^ [[maybe_unused]] (Enum l, Enum r) noexcept { return ::neko::cpp::bitflags::bitflags(l) ^ ::neko::cpp::bitflags::bitflags(r); }

#pragma endregion

#endif  // ME_ENUM_HPP
