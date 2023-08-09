// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef ME_SCRIPTING_HPP
#define ME_SCRIPTING_HPP

#include <functional>
#include <map>
#include <string>

#include "engine/common/neko_util.h"
#include "engine/scripting/neko_lua_base.h"
#include "engine/utility/logger.hpp"
#include "engine/utility/module.hpp"

struct lua_State;

namespace neko {

template <typename T>
neko_static_inline void struct_as(std::string &s, const char *table, const char *key, const T &value) {
    s += std::format("{0}.{1} = {2}\n", table, key, value);
}

template <>
neko_static_inline void struct_as(std::string &s, const char *table, const char *key, const std::string &value) {
    s += std::format("{0}.{1} = \"{2}\"\n", table, key, value);
}

using ppair = std::pair<const char *, const void *>;

struct test_visitor {
    std::vector<ppair> result;

    template <typename T>
    void operator()(const char *name, const T &t) {
        result.emplace_back(ppair{name, static_cast<const void *>(&t)});
    }
};

template <typename T>
void SaveLuaConfig(const T &_struct, const char *table_name, std::string &out) {
    ME::meta::dostruct::for_each(_struct, [&](const char *name, const auto &value) {
        // METADOT_INFO("{} == {} ({})", name, value, typeid(value).name());
        struct_as(out, table_name, name, value);
    });
}

#define LoadLuaConfig(_struct, _luat, _c) _struct->_c = _luat[#_c].get<decltype(_struct->_c)>()

// template<typename T>
// void LoadLuaConfig(const T &_struct, lua_wrapper::LuaTable *luat) {
//     int idx = 0;
//     test_visitor vis;
//     ME::meta::dostruct::apply_visitor(vis, _struct);
//     ME::meta::dostruct::for_each(_struct, [&](const char *name, const auto &value) {
//         // (*ME::meta::dostruct::get_pointer<idx>()) =
//         //         (*luat)[name].get<decltype(ME::meta::dostruct::get<idx>(_struct))>();
//         // (*vis1.result[idx].first) = (*luat)[name].get<>();
//     });
// }

void print_error(lua_State *state, int result = 0);
void script_runfile(const char *filePath);

template <class T>
struct scripting_auto_reg {
public:
    typedef typename T TEMPLATE_T;
    scripting_auto_reg() { auto_reg; }

private:
    struct type_registrator {
        type_registrator() { TEMPLATE_T::reg(); }
    };

    static const type_registrator auto_reg;
};

template <class T>
typename const scripting_auto_reg<T>::type_registrator scripting_auto_reg<T>::auto_reg;

class neko_scripting : public scripting_auto_reg<neko_scripting> {
public:
    static void reg() {}

public:
    neko_lua_t neko_lua;

    neko_scripting() noexcept {};
    ~neko_scripting() noexcept {};

    neko_move_only(neko_scripting);

    void __init();
    void __end();

public:
    void update();
    void update_render();
    void update_tick();
};

}  // namespace neko

#endif
