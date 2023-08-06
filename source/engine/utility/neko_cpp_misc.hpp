// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef NEKO_CPP_MISC_HPP
#define NEKO_CPP_MISC_HPP

#include "neko_cpp_utils.hpp"

#pragma region FuncTraits

namespace neko::cpp {
// type Object : if not member function, it is void
// type ArgList : TypeList<Args...>
// type Return
// type Signature : Ret(Args...)
// type Function : Ret(Args...) const? volatile? &/&&? noexcept
// bool is_const
// bool is_volatile
// ReferenceMode ref
// bool is_noexcept
template <typename T>
struct FuncTraits;

template <typename T>
using FuncTraits_Object = typename FuncTraits<T>::Object;
template <typename T>
using FuncTraits_ArgList = typename FuncTraits<T>::ArgList;
template <typename T>
using FuncTraits_Return = typename FuncTraits<T>::Return;
template <typename T>
using FuncTraits_Signature = typename FuncTraits<T>::Signature;
template <typename T>
using FuncTraits_Function = typename FuncTraits<T>::Function;
template <typename T>
constexpr bool FuncTraits_is_const = FuncTraits<T>::is_const;
template <typename T>
constexpr bool FuncTraits_is_volatile = FuncTraits<T>::is_volatile;
template <typename T>
constexpr ReferenceMode FuncTraits_ref = FuncTraits<T>::ref;
template <typename T>
constexpr bool FuncTraits_is_noexcept = FuncTraits<T>::is_noexcept;

// overload
template <typename Obj, typename Func>
struct MemFuncOf;
template <typename Func>
struct FuncOf;

// NewFunc == Ret(Args...)
// static Ret(Args...) run(Func);
// - { Func's arguments, ... } == { Args... }
// - Ret == void or Ret <- Func'return type
template <typename NewFunc>
struct FuncExpand;

template <typename Lambda>
constexpr auto DecayLambda(Lambda &&lambda) noexcept;
}  // namespace neko::cpp

namespace neko::cpp::details {
// ref: qobjectdefs_impl.h

template <typename T>
struct RmvLValueRef : std::type_identity<T> {};
template <typename T>
struct RmvLValueRef<T &> : std::type_identity<T> {};
template <typename T>
struct RmvConstRef : std::type_identity<T> {};
template <typename T>
struct RmvConstRef<const T &> : std::type_identity<T> {};

template <typename A1, typename A2>
struct AreArgumentsCompatible : std::is_same<const typename RmvLValueRef<A1>::type &, const typename RmvLValueRef<A2>::type &> {};
template <typename A1, typename A2>
struct AreArgumentsCompatible<A1, A2 &> : std::false_type {};
template <typename A>
struct AreArgumentsCompatible<A &, A &> : std::true_type {};
// void as a return value
template <typename A>
struct AreArgumentsCompatible<void, A> : std::true_type {};
template <typename A>
struct AreArgumentsCompatible<A, void> : std::true_type {};
template <>
struct AreArgumentsCompatible<void, void> : std::true_type {};

// ========================

template <typename ToArgList, typename FromArgList>
struct CheckCompatibleArguments : std::false_type {};

template <>
struct CheckCompatibleArguments<TypeList<>, TypeList<>> : std::true_type {};

template <typename ToArgList>
struct CheckCompatibleArguments<ToArgList, TypeList<>> : std::true_type {};

template <typename FromArgHead, typename... FromArgTail>
struct CheckCompatibleArguments<TypeList<>, TypeList<FromArgHead, FromArgTail...>> : std::false_type {};

template <typename ToArgHead, typename FromArgHead, typename... ToArgTail, typename... FromArgTail>
struct CheckCompatibleArguments<TypeList<ToArgHead, ToArgTail...>, TypeList<FromArgHead, FromArgTail...>> {
    static constexpr bool value = AreArgumentsCompatible<typename RmvConstRef<ToArgHead>::type, typename RmvConstRef<FromArgHead>::type>::value &&
                                  CheckCompatibleArguments<TypeList<ToArgTail...>, TypeList<FromArgTail...>>::value;
};
}  // namespace neko::cpp::details

namespace neko::cpp::details {
template <bool IsConst, bool IsVolatile, ReferenceMode Ref, bool IsNoexcept, typename Sig>
struct FuncTraitsBase;

template <bool IsConst, bool IsVolatile, ReferenceMode Ref, bool IsNoexcept, typename Ret, typename... Args>
struct FuncTraitsBase<IsConst, IsVolatile, Ref, IsNoexcept, Ret(Args...)> {
    using ArgList = TypeList<Args...>;
    using Return = Ret;
    using Signature = Ret(Args...);
    static constexpr bool is_const = IsConst;
    static constexpr bool is_volatile = IsVolatile;
    static constexpr ReferenceMode ref = Ref;
    static constexpr bool is_noexcept = IsNoexcept;
};

template <bool IsFunc, typename T>
struct FuncTraitsDispatch;
template <typename T>
struct FuncTraitsDispatch<false, T> : FuncTraits<decltype(&std::decay_t<T>::operator())> {};
template <typename T>
struct FuncTraitsDispatch<true, T> : FuncTraits<T> {
    using Function = T;
};
}  // namespace neko::cpp::details

// 2*2*3*2 = 24
template <typename Ret, typename... Args>  // 0000
struct neko::cpp::FuncTraits<Ret(Args...)> : details::FuncTraitsBase<false, false, ReferenceMode::None, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1000
struct neko::cpp::FuncTraits<Ret(Args...) const> : details::FuncTraitsBase<true, false, ReferenceMode::None, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0100
struct neko::cpp::FuncTraits<Ret(Args...) volatile> : details::FuncTraitsBase<false, true, ReferenceMode::None, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1100
struct neko::cpp::FuncTraits<Ret(Args...) const volatile> : details::FuncTraitsBase<true, true, ReferenceMode::None, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0010
struct neko::cpp::FuncTraits<Ret(Args...) &> : details::FuncTraitsBase<false, false, ReferenceMode::Left, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1010
struct neko::cpp::FuncTraits<Ret(Args...) const &> : details::FuncTraitsBase<true, false, ReferenceMode::Left, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0110
struct neko::cpp::FuncTraits<Ret(Args...) volatile &> : details::FuncTraitsBase<false, true, ReferenceMode::Left, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1110
struct neko::cpp::FuncTraits<Ret(Args...) const volatile &> : details::FuncTraitsBase<true, true, ReferenceMode::Left, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0020
struct neko::cpp::FuncTraits<Ret(Args...) &&> : details::FuncTraitsBase<false, false, ReferenceMode::Right, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1020
struct neko::cpp::FuncTraits<Ret(Args...) const &&> : details::FuncTraitsBase<true, false, ReferenceMode::Right, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0120
struct neko::cpp::FuncTraits<Ret(Args...) volatile &&> : details::FuncTraitsBase<false, true, ReferenceMode::Right, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1120
struct neko::cpp::FuncTraits<Ret(Args...) const volatile &&> : details::FuncTraitsBase<true, true, ReferenceMode::Right, false, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0001
struct neko::cpp::FuncTraits<Ret(Args...) noexcept> : details::FuncTraitsBase<false, false, ReferenceMode::None, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1001
struct neko::cpp::FuncTraits<Ret(Args...) const noexcept> : details::FuncTraitsBase<true, false, ReferenceMode::None, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0101
struct neko::cpp::FuncTraits<Ret(Args...) volatile noexcept> : details::FuncTraitsBase<false, true, ReferenceMode::None, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1101
struct neko::cpp::FuncTraits<Ret(Args...) const volatile noexcept> : details::FuncTraitsBase<true, true, ReferenceMode::None, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0011
struct neko::cpp::FuncTraits<Ret(Args...) & noexcept> : details::FuncTraitsBase<false, false, ReferenceMode::Left, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1011
struct neko::cpp::FuncTraits<Ret(Args...) const & noexcept> : details::FuncTraitsBase<true, false, ReferenceMode::Left, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0111
struct neko::cpp::FuncTraits<Ret(Args...) volatile & noexcept> : details::FuncTraitsBase<false, true, ReferenceMode::Left, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1111
struct neko::cpp::FuncTraits<Ret(Args...) const volatile & noexcept> : details::FuncTraitsBase<true, true, ReferenceMode::Left, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0021
struct neko::cpp::FuncTraits<Ret(Args...) && noexcept> : details::FuncTraitsBase<false, false, ReferenceMode::Right, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1021
struct neko::cpp::FuncTraits<Ret(Args...) const && noexcept> : details::FuncTraitsBase<true, false, ReferenceMode::Right, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 0121
struct neko::cpp::FuncTraits<Ret(Args...) volatile && noexcept> : details::FuncTraitsBase<false, true, ReferenceMode::Right, true, Ret(Args...)> {};

template <typename Ret, typename... Args>  // 1121
struct neko::cpp::FuncTraits<Ret(Args...) const volatile && noexcept> : details::FuncTraitsBase<true, true, ReferenceMode::Right, true, Ret(Args...)> {};

// dispatch
template <typename Func>
struct neko::cpp::FuncTraits<Func *> : FuncTraits<Func> {
    using Object = void;
    using Function = Func;
};

template <typename T, typename Func>
struct neko::cpp::FuncTraits<Func T::*> : FuncTraits<Func> {
    using Object = T;
    using Function = Func;
};

template <typename Func>
struct neko::cpp::FuncTraits<Func &> : FuncTraits<Func> {};
template <typename Func>
struct neko::cpp::FuncTraits<Func &&> : FuncTraits<Func> {};
template <typename Func>
struct neko::cpp::FuncTraits<const Func &> : FuncTraits<Func> {};
template <typename Func>
struct neko::cpp::FuncTraits<const Func &&> : FuncTraits<Func> {};

template <typename T>
struct neko::cpp::FuncTraits : details::FuncTraitsDispatch<std::is_function_v<T>, T> {};

template <typename Ret, typename... Args>
struct neko::cpp::FuncExpand<Ret(Args...)> {
    template <typename Func>
    static auto get(Func &&func) noexcept {
        static_assert(std::is_void_v<Ret> || std::is_convertible_v<FuncTraits_Return<Func>, Ret>, "Func's return can't convert to Ret");
        constexpr std::size_t N = Length_v<typename FuncTraits<Func>::ArgList>;
        return get(std::forward<Func>(func), std::make_index_sequence<N>{});
    }

private:
    template <typename Func, std::size_t... Ns>
    static auto get(Func &&func, std::index_sequence<Ns...>) {
        using FromArgList = typename FuncTraits<Func>::ArgList;
        using ToArgList = TypeList<Args...>;
        return [func = std::forward<Func>(func)](Args... args) mutable -> decltype(auto) {
            std::tuple<Args...> argTuple{std::forward<Args>(args)...};
            static_assert(details::CheckCompatibleArguments<ToArgList, FromArgList>::value, "from and to arguments are not compatible.");
            if constexpr (std::is_void_v<Ret>)
                func(std::forward<At_t<ToArgList, Ns>>(std::get<Ns>(argTuple))...);
            else
                return static_cast<Ret>(func(std::forward<At_t<ToArgList, Ns>>(std::get<Ns>(argTuple))...));
        };
    }
};

template <typename Lambda>
constexpr auto neko::cpp::DecayLambda(Lambda &&lambda) noexcept {
    return static_cast<std::add_pointer_t<FuncTraits_Signature<std::remove_reference_t<Lambda>>>>(std::forward<Lambda>(lambda));
}

template <typename Obj, typename Func>
struct neko::cpp::MemFuncOf {
    static_assert(std::is_function_v<Func>);
    static constexpr auto get(Func Obj::*func) noexcept { return func; }
};

template <typename Func>
struct neko::cpp::FuncOf {
    static_assert(std::is_function_v<Func>);
    static constexpr auto get(Func *func) noexcept { return func; }
};

#pragma endregion FuncTraits

#pragma region SI

#define SI_InterfaceTraits_Register(Interface, ...)   \
    template <>                                       \
    struct neko::cpp::SI_InterfaceTraits<Interface> { \
        using IList = TemplateList<__VA_ARGS__>;      \
    }

#define SI_InterfaceTraits_Register_Pro(Interface, ...) \
    template <>                                         \
    struct neko::cpp::SI_InterfaceTraits<Interface> : neko::cpp::details::IListBase<__VA_ARGS__>

#define SI_CombineInterface(Interface, ...) \
    template <typename Base, typename Impl> \
    struct Interface : Base {               \
        using Base::Base;                   \
    };                                      \
    SI_InterfaceTraits_Register(Interface, __VA_ARGS__)

#define SI_ImplTraits_Register(Impl, ...)        \
    template <>                                  \
    struct neko::cpp::SI_ImplTraits<Impl> {      \
        using IList = TemplateList<__VA_ARGS__>; \
    }

#define SI_ImplTraits_Register_Pro(Impl, ...) \
    template <>                               \
    struct neko::cpp::SI_ImplTraits<Impl> : neko::cpp::details::IListBase<__VA_ARGS__>

namespace neko::cpp::details {
template <typename Impl>
struct SI;
template <typename Impl>
using SI_t = typename SI<Impl>::type;
}  // namespace neko::cpp::details

namespace neko::cpp {
// IList : TemplateList<Interfaces...>
template <typename Impl>
struct SI_ImplTraits {};

// IList : TemplateList<Interfaces...>
template <template <typename Base, typename Impl> class Interface>
struct SI_InterfaceTraits {};

template <typename Impl>
using SI = details::SI_t<Impl>;

template <typename T, template <typename Base, typename Impl> class Interface>
struct SI_Contains;
template <typename T, template <typename Base, typename Impl> class Interface>
static constexpr bool SI_Contains_v = SI_Contains<T, Interface>::value;
}  // namespace neko::cpp

namespace neko::cpp::details {
//
// SI_ImplTraits_IList
////////////////////////

template <typename Enabler, typename Impl>
struct SI_ImplTraits_IList_Helper : std::type_identity<TemplateList<>> {};

template <typename Impl>
struct SI_ImplTraits_IList_Helper<std::void_t<typename SI_ImplTraits<Impl>::IList>, Impl> : std::type_identity<typename SI_ImplTraits<Impl>::IList> {};

template <typename Impl>
struct SI_ImplTraits_IList : SI_ImplTraits_IList_Helper<void, Impl> {};
template <typename Impl>
using SI_ImplTraits_IList_t = typename SI_ImplTraits_IList<Impl>::type;

//
// SI_InterfaceTraits_IList
/////////////////////////////

template <typename Void, template <typename Base, typename Impl> class Interface>
struct SI_InterfaceTraits_IList_Helper : std::type_identity<TemplateList<>> {};

template <template <typename Base, typename Impl> class Interface>
struct SI_InterfaceTraits_IList_Helper<std::void_t<typename SI_InterfaceTraits<Interface>::IList>, Interface> : std::type_identity<typename SI_InterfaceTraits<Interface>::IList> {};

template <template <typename Base, typename Impl> class Interface>
struct SI_InterfaceTraits_IList : SI_InterfaceTraits_IList_Helper<void, Interface> {};
template <template <typename Base, typename Impl> class Interface>
using SI_InterfaceTraits_IList_t = typename SI_InterfaceTraits_IList<Interface>::type;

//
// ITopoSort
//////////////

template <typename IList, typename SortedIList>
struct ITopoSort_Helper;
template <typename IList, typename SortedIList>
using ITopoSort_Helper_t = typename ITopoSort_Helper<IList, SortedIList>::type;

template <typename SortedIList>
struct ITopoSort_Helper<TemplateList<>, SortedIList> : std::type_identity<SortedIList> {};

template <bool NeedRecuresion, typename SortedIList, template <typename Base, typename Impl> class IHead>
struct ITopoSort_Recursion;
template <typename SortedIList, template <typename Base, typename Impl> class IHead>
struct ITopoSort_Recursion<true, SortedIList, IHead> : std::type_identity<SortedIList> {};
template <typename SortedIList, template <typename Base, typename Impl> class IHead>
struct ITopoSort_Recursion<false, SortedIList, IHead> : std::type_identity<TPushFront_t<ITopoSort_Helper_t<SI_InterfaceTraits_IList_t<IHead>, SortedIList>, IHead>> {};

template <template <typename Base, typename Impl> class IHead, template <typename Base, typename Impl> class... ITail, typename SortedIList>
struct ITopoSort_Helper<TemplateList<IHead, ITail...>, SortedIList> {
    using type = ITopoSort_Helper_t<TemplateList<ITail...>, typename ITopoSort_Recursion<TContain_v<SortedIList, IHead>, SortedIList, IHead>::type>;
};

template <typename IList>
struct ITopoSort : ITopoSort_Helper<IList, TemplateList<>> {};
template <typename IList>
using ITopoSort_t = typename ITopoSort<IList>::type;

//
// Nil
////////

template <typename Impl>
struct Nil {
    // for using Base::operatorXX in Interface

    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator+(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator-(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator*(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator/(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator^(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator&(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator|(SI_ERROR) = delete;
    /*template<typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator~() = delete;
    template<typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator!() = delete;*/
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator<(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator>(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator+=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator-=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator*=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator/=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator%=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator^=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator&=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator|=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator<<(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator>>(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator>>=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator<<=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator==(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator!=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator<=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator>=(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator&&(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator||(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator++(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator--(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator,(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator->*(SI_ERROR) = delete;
    /*template<typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator->() = delete;*/
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator()(SI_ERROR) = delete;
    template <typename SI_ERROR, typename = typename SI_ERROR::SI_ERROR>
    void operator[](SI_ERROR) = delete;
};

//
// SI
///////

template <typename IList, typename Impl>
struct SI_Helper;
template <typename IList, typename Impl>
using SI_Helper_t = typename SI_Helper<IList, Impl>::type;

template <typename Impl>
struct SI_Helper<TemplateList<>, Impl> : std::type_identity<Nil<Impl>> {};

template <template <typename Base, typename Impl> class IHead, template <typename Base, typename Impl> class... ITail, typename Impl>
struct SI_Helper<TemplateList<IHead, ITail...>, Impl> : std::type_identity<IHead<SI_Helper_t<TemplateList<ITail...>, Impl>, Impl>> {};

//
// SI_Contains
////////////////

template <typename Void, typename T, template <typename Base, typename Impl> class Interface>
struct SI_Contains_Helper : std::false_type {};
template <typename T, template <typename Base, typename Impl> class Interface>
struct SI_Contains_Helper<std::void_t<ITopoSort_t<SI_ImplTraits_IList_t<T>>>, T, Interface> : std::bool_constant<TContain_v<ITopoSort_t<SI_ImplTraits_IList_t<T>>, Interface>> {};
}  // namespace neko::cpp::details

template <typename Impl>
struct neko::cpp::details::SI : SI_Helper<ITopoSort_t<SI_ImplTraits_IList_t<Impl>>, Impl> {};

template <typename T, template <typename Base, typename Impl> class Interface>
struct neko::cpp::SI_Contains : details::SI_Contains_Helper<void, T, Interface> {};

#pragma endregion SI

#endif