// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef NEKO_CVAR_H
#define NEKO_CVAR_H

#include "engine/neko.h"
#include "engine/neko_containers.h"

enum cmd_type { CVAR_VAR = 0, CVAR_FUNC = 1 };

void __neko_engine_cvar_init();

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

NEKO_API_DECL void __neko_config_init();
NEKO_API_DECL void __neko_config_free();

NEKO_API_DECL neko_cvar_t *__neko_config_get(const_str name);
NEKO_API_DECL void neko_config_print();

#define neko_cvar_new(n, t, v)                                                \
    {                                                                         \
        neko_cvar_t cvar = {.name = n, .type = t, .value = 0};                \
        if (t == __NEKO_CONFIG_TYPE_INT) {                                    \
            cvar.value.i = v;                                                 \
        } else if (t == __NEKO_CONFIG_TYPE_FLOAT) {                           \
            cvar.value.f = v;                                                 \
        } else {                                                              \
            neko_assert(false);                                               \
        }                                                                     \
        neko_dyn_array_push((neko_instance()->ctx.game.config)->cvars, cvar); \
    }

#define neko_cvar_lnew(n, t, v)                                               \
    {                                                                         \
        neko_cvar_t cvar = {.type = t, .value = 0};                           \
        std::strncpy(cvar.name, n, sizeof(cvar.name) - 1);                    \
        cvar.name[sizeof(cvar.name) - 1] = '\0';                              \
        if (t == __NEKO_CONFIG_TYPE_INT) {                                    \
            cvar.value.i = v;                                                 \
        } else if (t == __NEKO_CONFIG_TYPE_FLOAT) {                           \
            cvar.value.f = v;                                                 \
        } else {                                                              \
            neko_assert(false);                                               \
        }                                                                     \
        neko_dyn_array_push((neko_instance()->ctx.game.config)->cvars, cvar); \
    }

// 由于 float -> char* 转换 很难将其写成单个宏
#define neko_cvar_new_str(n, t, v)                                                         \
    {                                                                                      \
        neko_cvar_t cvar = {.name = n, .type = t, .value = {0}};                           \
        cvar.value.s = (char *)neko_malloc(__NEKO_CVAR_STR_LEN);                           \
        memset(cvar.value.s, 0, __NEKO_CVAR_STR_LEN);                                      \
        memcpy(cvar.value.s, v, neko_min(__NEKO_CVAR_STR_LEN - 1, neko_string_length(v))); \
        neko_dyn_array_push((neko_instance()->ctx.game.config)->cvars, cvar);              \
    }

#define neko_cvar_lnew_str(n, t, v)                                                        \
    {                                                                                      \
        neko_cvar_t cvar = {.type = t, .value = {0}};                                      \
        std::strncpy(cvar.name, n, sizeof(cvar.name) - 1);                                 \
        cvar.name[sizeof(cvar.name) - 1] = '\0';                                           \
        cvar.value.s = (char *)neko_malloc(__NEKO_CVAR_STR_LEN);                           \
        memset(cvar.value.s, 0, __NEKO_CVAR_STR_LEN);                                      \
        memcpy(cvar.value.s, v, neko_min(__NEKO_CVAR_STR_LEN - 1, neko_string_length(v))); \
        neko_dyn_array_push((neko_instance()->ctx.game.config)->cvars, cvar);              \
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

#endif