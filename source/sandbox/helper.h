
#ifndef GAME_HELPER_H
#define GAME_HELPER_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "engine/neko.hpp"


NEKO_INLINE neko_color_t neko_color_interpolate(neko_color_t x, neko_color_t y, float t) {
    u8 r = (u8)(x.r + (y.r - x.r) * t);
    u8 g = (u8)(x.g + (y.g - x.g) * t);
    u8 b = (u8)(x.b + (y.b - x.b) * t);
    u8 a = (u8)(x.a + (y.a - x.a) * t);
    return neko_color(r, g, b, a);
}

int g_random(void);
s32 random_val(s32 lower, s32 upper);
s32 random_tlb_val(s32 lower, s32 upper);

namespace neko {


template <typename T>
class creator {
public:
    virtual std::unique_ptr<T> create() = 0;
};

template <typename Type, typename ConcreteType, typename... Args>
class ConcreteCreator : public creator<Type> {
public:
    explicit ConcreteCreator(Args&&... args) : mArgs(std::forward<Args>(args)...) {}
    std::unique_ptr<Type> create() override { return createImpl(std::make_index_sequence<sizeof...(Args)>{}); }

private:
    template <size_t... Is>
    std::unique_ptr<Type> createImpl(std::index_sequence<Is...>) {
        return std::make_unique<ConcreteType>(std::get<Is>(mArgs)...);
    }
    std::tuple<Args...> mArgs;
};

template <typename T, typename I>
class Factory {
public:
    template <typename ConcreteType, typename... Args>
    void registerType(I identifier, Args&&... args) {
        static_assert(std::is_base_of<T, ConcreteType>::value, "Type must be a base of ConcreteType");
        creator_list[identifier] = std::make_unique<ConcreteCreator<T, ConcreteType, Args...>>(std::forward<Args>(args)...);
    }

    std::unique_ptr<T> create(I identifier) {
        if (creator_list.find(identifier) != creator_list.end()) {
            return creator_list[identifier]->create();
        }
        return nullptr;
    }

private:
    std::unordered_map<I, std::unique_ptr<creator<T>>> creator_list;
};

}  // namespace neko

#endif