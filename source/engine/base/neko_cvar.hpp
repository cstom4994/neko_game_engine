// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef ME_CVAR_HPP
#define ME_CVAR_HPP

#include <cstddef>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <map>
#include <ostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include "engine/base/neko_meta.hpp"
#include "engine/common/neko_containers.h"
#include "engine/common/neko_util.h"
#include "engine/utility/module.hpp"
#include "engine/utility/neko_cpp_utils.hpp"

namespace neko {

enum cmd_type { CVAR_VAR = 0, CVAR_FUNC = 1 };

#define CVAR_TYPES() bool, int, float

struct neko_engine_cvar_t {
    bool ui_tweak;
    bool ui_debug;
    bool ui_imgui_debug;
    bool ui_profiler;
    bool ui_console;
    bool ui_pack_editor;
    bool ui_inspector;
    bool ui_memory_profiler;
    bool debug_entities_test;

    bool draw_shaders = true;
    bool tick_world = true;
    int cell_iter;
    f32 brush_size = 4.f;
    f32 world_gravity = 10.f;
};

extern neko_engine_cvar_t g_cvar;

void __neko_engine_cvar_init(neko_engine_cvar_t *s);

void __neko_engine_cvar_reg(neko_engine_cvar_t *s);

template <typename T>
void neko_engine_cvar_init(neko_engine_cvar_t *s, T func) {
    // static_assert(std::is_function_v<T>);
    __neko_engine_cvar_init(s);
    func();
    __neko_engine_cvar_reg(s);
}

#define __NEKO_CVAR_STR_LEN 64

typedef enum neko_cvar_type {
    __NEKO_CONFIG_TYPE_INT,
    __NEKO_CONFIG_TYPE_FLOAT,
    __NEKO_CONFIG_TYPE_STRING,
    __NEKO_CONFIG_TYPE_COUNT,
} neko_cvar_type;

typedef struct neko_cvar_t {
    char name[__NEKO_CVAR_STR_LEN];  // 直接开辟好空间
    neko_cvar_type type;             // cvar 类型
    union {                          // 联合体存值
        f32 f;
        s32 i;
        char *s;
    } value;
} neko_cvar_t;

typedef struct neko_config_t {
    neko_dyn_array(neko_cvar_t) cvars;
} neko_config_t;

void __neko_config_init();
void __neko_config_free();

neko_cvar_t *__neko_config_get(const_str name);
void neko_config_print();

#define neko_cvar_new(n, t, v)                                                  \
    {                                                                           \
        neko_cvar_t cvar = neko_cvar_t{.name = n, .type = t, .value = 0};       \
        if (t == __NEKO_CONFIG_TYPE_INT) {                                      \
            cvar.value.i = v;                                                   \
        } else if (t == __NEKO_CONFIG_TYPE_FLOAT) {                             \
            cvar.value.f = v;                                                   \
        } else {                                                                \
            neko_assert(false);                                                 \
        }                                                                       \
        neko_dyn_array_push((neko_engine_instance()->ctx.config)->cvars, cvar); \
    }

#define neko_cvar_lnew(n, t, v)                                                 \
    {                                                                           \
        neko_cvar_t cvar = neko_cvar_t{.type = t, .value = 0};                  \
        std::strncpy(cvar.name, n, sizeof(cvar.name) - 1);                      \
        cvar.name[sizeof(cvar.name) - 1] = '\0';                                \
        if (t == __NEKO_CONFIG_TYPE_INT) {                                      \
            cvar.value.i = v;                                                   \
        } else if (t == __NEKO_CONFIG_TYPE_FLOAT) {                             \
            cvar.value.f = v;                                                   \
        } else {                                                                \
            neko_assert(false);                                                 \
        }                                                                       \
        neko_dyn_array_push((neko_engine_instance()->ctx.config)->cvars, cvar); \
    }

// 由于 float -> char* 转换 很难将其写成单个宏
#define neko_cvar_new_str(n, t, v)                                                         \
    {                                                                                      \
        neko_cvar_t cvar = neko_cvar_t{.name = n, .type = t, .value = {0}};                \
        cvar.value.s = (char *)neko_malloc(__NEKO_CVAR_STR_LEN);                           \
        memset(cvar.value.s, 0, __NEKO_CVAR_STR_LEN);                                      \
        memcpy(cvar.value.s, v, neko_min(__NEKO_CVAR_STR_LEN - 1, neko_string_length(v))); \
        neko_dyn_array_push((neko_engine_instance()->ctx.config)->cvars, cvar);            \
    }

#define neko_cvar_lnew_str(n, t, v)                                                        \
    {                                                                                      \
        neko_cvar_t cvar = neko_cvar_t{.type = t, .value = {0}};                           \
        std::strncpy(cvar.name, n, sizeof(cvar.name) - 1);                                 \
        cvar.name[sizeof(cvar.name) - 1] = '\0';                                           \
        cvar.value.s = (char *)neko_malloc(__NEKO_CVAR_STR_LEN);                           \
        memset(cvar.value.s, 0, __NEKO_CVAR_STR_LEN);                                      \
        memcpy(cvar.value.s, v, neko_min(__NEKO_CVAR_STR_LEN - 1, neko_string_length(v))); \
        neko_dyn_array_push((neko_engine_instance()->ctx.config)->cvars, cvar);            \
    }

#define neko_cvar(n) __neko_config_get(n)

#define neko_cvar_print(cvar)                                           \
    {                                                                   \
        switch ((cvar)->type) {                                         \
            default:                                                    \
            case __NEKO_CONFIG_TYPE_STRING:                             \
                neko_println("%s = %s", (cvar)->name, (cvar)->value.s); \
                break;                                                  \
                                                                        \
            case __NEKO_CONFIG_TYPE_FLOAT:                              \
                neko_println("%s = %f", (cvar)->name, (cvar)->value.f); \
                break;                                                  \
                                                                        \
            case __NEKO_CONFIG_TYPE_INT:                                \
                neko_println("%s = %d", (cvar)->name, (cvar)->value.i); \
                break;                                                  \
        };                                                              \
    }

#define neko_cvar_set(cvar, str)                                           \
    {                                                                      \
        switch ((cvar)->type) {                                            \
            default:                                                       \
            case __NEKO_CONFIG_TYPE_STRING:                                \
                memcpy((cvar)->value.s, str, neko_string_length(str) + 1); \
                break;                                                     \
                                                                           \
            case __NEKO_CONFIG_TYPE_FLOAT:                                 \
                (cvar)->value.f = atof(str);                               \
                break;                                                     \
                                                                           \
            case __NEKO_CONFIG_TYPE_INT:                                   \
                (cvar)->value.i = atoi(str);                               \
                break;                                                     \
        };                                                                 \
    }

}  // namespace neko

#endif