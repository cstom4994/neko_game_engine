// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef NEKO_CPP_MISC_HPP
#define NEKO_CPP_MISC_HPP

#include "neko_cpp_utils.hpp"

#pragma region Tuple

namespace neko::cpp::detail {
template <typename Ints>
struct IntegerSequenceTraits;

template <typename T, T N0, T... Ns>
struct IntegerSequenceTraits<std::integer_sequence<T, N0, Ns...>> {
    static constexpr size_t head = N0;
    static constexpr auto tail = std::integer_sequence<T, Ns...>{};
};

template <typename Tuple, typename Acc, typename Func, bool... Masks, size_t... Ns>
constexpr auto tuple_accumulate(Tuple&& t, Acc&& acc, Func&& f, std::integer_sequence<bool, Masks...>, std::index_sequence<Ns...>) {
    if constexpr (sizeof...(Ns) > 0) {
        using IST_Ns = IntegerSequenceTraits<std::index_sequence<Ns...>>;
        if constexpr (sizeof...(Masks) > 0) {
            using IST_Masks = IntegerSequenceTraits<std::integer_sequence<bool, Masks...>>;
            if constexpr (IST_Masks::head) {
                return tuple_accumulate(std::forward<Tuple>(t), f(std::forward<Acc>(acc), std::get<IST_Ns::head>(std::forward<Tuple>(t))), std::forward<Func>(f), IST_Masks::tail, IST_Ns::tail);
            } else {
                return tuple_accumulate(std::forward<Tuple>(t), std::forward<Acc>(acc), std::forward<Func>(f), IST_Masks::tail, IST_Ns::tail);
            }
        } else {
            return tuple_accumulate(std::forward<Tuple>(t), f(std::forward<Acc>(acc), std::get<IST_Ns::head>(std::forward<Tuple>(t))), std::forward<Func>(f), std::integer_sequence<bool>{},
                                    IST_Ns::tail);
        }
    } else
        return std::forward<Acc>(acc);
}

template <typename Tuple, typename Func, size_t... Ns>
constexpr size_t tuple_find_if(const Tuple& t, Func&& f, std::index_sequence<Ns...>) {
    if constexpr (sizeof...(Ns) > 0) {
        using IST_Ns = IntegerSequenceTraits<std::index_sequence<Ns...>>;
        if (std::forward<Func>(f)(std::get<IST_Ns::head>(t)))
            return IST_Ns::head;
        else
            return tuple_find_if(t, std::forward<Func>(f), IST_Ns::tail);
    } else
        return static_cast<size_t>(-1);
}
}  // namespace neko::cpp::detail

namespace neko::cpp {
template <bool... Masks, typename Tuple, typename Init, typename Func>
constexpr auto tuple_accumulate(Tuple&& t, Init&& i, Func&& f) {
    constexpr size_t N = std::tuple_size_v<std::decay_t<Tuple>>;
    static_assert(sizeof...(Masks) <= N);
    return detail::tuple_accumulate(std::forward<Tuple>(t), std::forward<Init>(i), std::forward<Func>(f), std::integer_sequence<bool, Masks...>{}, std::make_index_sequence<N>{});
}

template <bool... Masks, typename Tuple, typename Func>
constexpr void tuple_for_each(Tuple&& t, Func&& f) {
    tuple_accumulate<Masks...>(std::forward<Tuple>(t), 0, [&](auto, auto&& val) {
        std::forward<Func>(f)(std::forward<decltype(val)>(val));
        return 0;
    });
}

template <typename Tuple, typename Func>
constexpr size_t tuple_find_if(const Tuple& t, Func&& f) {
    return detail::tuple_find_if(t, std::forward<Func>(f), std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

template <typename Tuple, typename Elem>
constexpr size_t tuple_find(const Tuple& t, const Elem& e) {
    return tuple_find_if(t, [&](const auto& t_e) {
        if constexpr (std::is_same_v<std::decay_t<decltype(t_e)>, Elem>)
            return t_e == e;
        else
            return false;
    });
}

template <typename Tuple, typename Elem>
constexpr bool tuple_constains(const Tuple& t, const Elem& e) {
    return tuple_find(t, e) != static_cast<size_t>(-1);
}

template <typename Tuple, typename Func>
constexpr size_t tuple_count_if(const Tuple& t, Func&& f) {
    return tuple_accumulate(t, 0, [&](auto cnt, const auto& e) {
        if (std::forward<Func>(f)(e))
            return cnt + 1;
        else
            return cnt;
    });
}

template <typename Tuple, typename Elem>
constexpr size_t tuple_count(const Tuple& t, const Elem& e) {
    return tuple_count_if(t, [&](const auto& t_e) {
        if constexpr (std::is_same_v<std::decay_t<decltype(t_e)>, Elem>)
            return t_e == e;
        else
            return false;
    });
}

template <typename Tuple, typename... Elems>
constexpr auto tuple_append(Tuple&& src, Elems&&... elems) {
    return std::apply([&](auto&&... src_elems) { return std::tuple{std::forward<decltype(src_elems)>(src_elems)..., std::forward<Elems>(elems)...}; }, std::forward<Tuple>(src));
}

template <typename Tuple, typename... Elems>
constexpr auto tuple_prepend(Tuple&& src, Elems&&... elems) {
    return std::apply([&](auto&&... src_elems) { return std::tuple{std::forward<Elems>(elems)..., std::forward<decltype(src_elems)>(src_elems)...}; }, std::forward<Tuple>(src));
}

template <bool... Masks, typename Tuple, typename Init, typename Func>
constexpr auto tuple_accumulate(Tuple&&, Init&&, Func&&);

template <bool... Masks, typename Tuple, typename Func>
constexpr void tuple_for_each(Tuple&&, Func&&);

template <typename Tuple, typename Func>
constexpr size_t tuple_find_if(const Tuple&, Func&&);

template <typename Tuple, typename Elem>
constexpr size_t tuple_find(const Tuple&, const Elem&);

template <typename Tuple, typename Elem>
constexpr bool tuple_constains(const Tuple&, const Elem&);

template <typename Tuple, typename Func>
constexpr size_t tuple_count_if(const Tuple&, Func&&);

template <typename Tuple, typename Elem>
constexpr size_t tuple_count(const Tuple&, const Elem&);

template <typename Tuple, typename... Elems>
constexpr auto tuple_append(Tuple&&, Elems&&...);

template <typename Tuple, typename... Elems>
constexpr auto tuple_prepend(Tuple&&, Elems&&...);

template <typename T>
struct tuple_size;

template <typename... Args>
struct tuple_size<std::tuple<Args...>> {
    static constexpr std::size_t value = sizeof...(Args);
};

template <typename T>
constexpr std::size_t tuple_size_v = tuple_size<T>::value;

}  // namespace neko::cpp

#pragma endregion Tuple

#endif