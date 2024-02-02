
#ifndef NEKO_ARGS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "engine/neko.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct neko_args_desc {
    s32 argc;
    char** argv;
    s32 max_args;
    s32 buf_size;
} neko_args_desc;

NEKO_API_DECL void neko_args_setup(const neko_args_desc* desc);
NEKO_API_DECL void neko_args_shutdown(void);
NEKO_API_DECL bool neko_args_isvalid(void);
NEKO_API_DECL bool neko_args_exists(const_str key);
NEKO_API_DECL const_str neko_args_value(const_str key);
NEKO_API_DECL const_str neko_args_value_def(const_str key, const_str def);
NEKO_API_DECL bool neko_args_equals(const_str key, const_str val);
NEKO_API_DECL bool neko_args_boolean(const_str key);
NEKO_API_DECL s32 neko_args_find(const_str key);
NEKO_API_DECL s32 neko_args_num_args(void);
NEKO_API_DECL const_str neko_args_key_at(s32 index);
NEKO_API_DECL const_str neko_args_value_at(s32 index);

#ifdef __cplusplus
}

inline void neko_args_setup(const neko_args_desc& desc) { return neko_args_setup(&desc); }

#endif

#endif

#ifdef NEKO_ARGS_IMPL

#define __neko_args_def(v, def) (((v) == 0) ? (def) : (v))

#define NEKO_ARGS_MAX_ARGS_DEF (16)
#define NEKO_ARGS_BUF_SIZE_DEF (16 * 1024)

#define NEKO_ARGS_EXPECT_KEY (1 << 0)
#define NEKO_ARGS_EXPECT_SEP (1 << 1)
#define NEKO_ARGS_EXPECT_VAL (1 << 2)
#define NEKO_ARGS_PARSING_KEY (1 << 3)
#define NEKO_ARGS_PARSING_VAL (1 << 4)
#define NEKO_ARGS_ERROR (1 << 5)

typedef struct {
    s32 key;
    s32 val;
} __neko_args_kvp_t;

typedef struct {
    s32 max_args;
    s32 num_args;
    __neko_args_kvp_t* args;
    s32 buf_size;
    s32 buf_pos;
    char* buf;
    bool valid;
    uint32_t parse_state;
    char quote;
    bool in_escape;
} neko_args_state_t;

static neko_args_state_t g_args;

neko_global void __neko_args_clear(void* ptr, size_t size) {
    neko_assert(ptr && (size > 0));
    memset(ptr, 0, size);
}

neko_global void* __neko_args_malloc(size_t size) {
    neko_assert(size > 0);
    void* ptr = malloc(size);
    neko_assert(ptr);
    return ptr;
}

neko_global void* __neko_args_malloc_clear(size_t size) {
    void* ptr = __neko_args_malloc(size);
    __neko_args_clear(ptr, size);
    return ptr;
}

neko_global void __neko_args_free(void* ptr) { free(ptr); }

neko_global void __neko_args_putc(char c) {
    if ((g_args.buf_pos + 2) < g_args.buf_size) {
        g_args.buf[g_args.buf_pos++] = c;
    }
}

neko_global const_str __neko_args_str(s32 index) {
    neko_assert((index >= 0) && (index < g_args.buf_size));
    return &g_args.buf[index];
}

neko_global void __neko_args_expect_key(void) { g_args.parse_state = NEKO_ARGS_EXPECT_KEY; }

neko_global bool __neko_args_key_expected(void) { return 0 != (g_args.parse_state & NEKO_ARGS_EXPECT_KEY); }

neko_global void __neko_args_expect_val(void) { g_args.parse_state = NEKO_ARGS_EXPECT_VAL; }

neko_global bool __neko_args_val_expected(void) { return 0 != (g_args.parse_state & NEKO_ARGS_EXPECT_VAL); }

neko_global void __neko_args_expect_sep_or_key(void) { g_args.parse_state = NEKO_ARGS_EXPECT_SEP | NEKO_ARGS_EXPECT_KEY; }

neko_global bool __neko_args_any_expected(void) { return 0 != (g_args.parse_state & (NEKO_ARGS_EXPECT_KEY | NEKO_ARGS_EXPECT_VAL | NEKO_ARGS_EXPECT_SEP)); }

neko_global bool __neko_args_is_separator(char c) { return c == '='; }

neko_global bool __neko_args_is_quote(char c) {
    if (0 == g_args.quote) {
        return (c == '\'') || (c == '"');
    } else {
        return c == g_args.quote;
    }
}

neko_global void __neko_args_begin_quote(char c) { g_args.quote = c; }

neko_global void __neko_args_end_quote(void) { g_args.quote = 0; }

neko_global bool __neko_args_in_quotes(void) { return 0 != g_args.quote; }

neko_global bool __neko_args_is_whitespace(char c) { return !__neko_args_in_quotes() && ((c == ' ') || (c == '\t')); }

neko_global void __neko_args_start_key(void) {
    neko_assert((g_args.num_args >= 0) && (g_args.num_args < g_args.max_args));
    g_args.parse_state = NEKO_ARGS_PARSING_KEY;
    g_args.args[g_args.num_args].key = g_args.buf_pos;
}

neko_global void __neko_args_end_key(void) {
    neko_assert((g_args.num_args >= 0) && (g_args.num_args < g_args.max_args));
    __neko_args_putc(0);

    g_args.args[g_args.num_args].val = g_args.buf_pos - 1;
    g_args.num_args++;
    g_args.parse_state = 0;
}

neko_global bool __neko_args_parsing_key(void) { return 0 != (g_args.parse_state & NEKO_ARGS_PARSING_KEY); }

neko_global void __neko_args_start_val(void) {
    neko_assert((g_args.num_args > 0) && (g_args.num_args <= g_args.max_args));
    g_args.parse_state = NEKO_ARGS_PARSING_VAL;
    g_args.args[g_args.num_args - 1].val = g_args.buf_pos;
}

neko_global void __neko_args_end_val(void) {
    __neko_args_putc(0);
    g_args.parse_state = 0;
}

neko_global bool __neko_args_is_escape(char c) { return '\\' == c; }

neko_global void __neko_args_start_escape(void) { g_args.in_escape = true; }

neko_global bool __neko_args_in_escape(void) { return g_args.in_escape; }

neko_global char __neko_args_escape(char c) {
    switch (c) {
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case '\\':
            return '\\';
        default:
            return c;
    }
}

neko_global void __neko_args_end_escape(void) { g_args.in_escape = false; }

neko_global bool __neko_args_parsing_val(void) { return 0 != (g_args.parse_state & NEKO_ARGS_PARSING_VAL); }

neko_global bool __neko_args_parse_carg(const_str src) {
    char c;
    while (0 != (c = *src++)) {
        if (__neko_args_in_escape()) {
            c = __neko_args_escape(c);
            __neko_args_end_escape();
        } else if (__neko_args_is_escape(c)) {
            __neko_args_start_escape();
            continue;
        }
        if (__neko_args_any_expected()) {
            if (!__neko_args_is_whitespace(c)) {

                if (__neko_args_is_separator(c)) {

                    __neko_args_expect_val();
                    continue;
                } else if (__neko_args_key_expected()) {

                    __neko_args_start_key();
                } else if (__neko_args_val_expected()) {

                    if (__neko_args_is_quote(c)) {
                        __neko_args_begin_quote(c);
                        continue;
                    }
                    __neko_args_start_val();
                }
            } else {

                continue;
            }
        } else if (__neko_args_parsing_key()) {
            if (__neko_args_is_whitespace(c) || __neko_args_is_separator(c)) {

                __neko_args_end_key();
                if (__neko_args_is_separator(c)) {
                    __neko_args_expect_val();
                } else {
                    __neko_args_expect_sep_or_key();
                }
                continue;
            }
        } else if (__neko_args_parsing_val()) {
            if (__neko_args_in_quotes()) {

                if (__neko_args_is_quote(c)) {
                    __neko_args_end_quote();
                    __neko_args_end_val();
                    __neko_args_expect_key();
                    continue;
                }
            } else if (__neko_args_is_whitespace(c)) {

                __neko_args_end_val();
                __neko_args_expect_key();
                continue;
            }
        }
        __neko_args_putc(c);
    }
    if (__neko_args_parsing_key()) {
        __neko_args_end_key();
        __neko_args_expect_sep_or_key();
    } else if (__neko_args_parsing_val() && !__neko_args_in_quotes()) {
        __neko_args_end_val();
        __neko_args_expect_key();
    }
    return true;
}

neko_global bool __neko_args_parse_cargs(s32 argc, const_str* argv) {
    __neko_args_expect_key();
    bool retval = true;
    for (s32 i = 1; i < argc; i++) {
        retval &= __neko_args_parse_carg(argv[i]);
    }
    g_args.parse_state = 0;
    return retval;
}

neko_inline void neko_args_setup(const neko_args_desc* desc) {
    neko_assert(desc);
    __neko_args_clear(&g_args, sizeof(g_args));
    g_args.max_args = __neko_args_def(desc->max_args, NEKO_ARGS_MAX_ARGS_DEF);
    g_args.buf_size = __neko_args_def(desc->buf_size, NEKO_ARGS_BUF_SIZE_DEF);
    neko_assert(g_args.buf_size > 8);
    g_args.args = (__neko_args_kvp_t*)__neko_args_malloc_clear((size_t)g_args.max_args * sizeof(__neko_args_kvp_t));
    g_args.buf = (char*)__neko_args_malloc_clear((size_t)g_args.buf_size * sizeof(char));

    g_args.buf_pos = 1;
    g_args.valid = true;

    __neko_args_parse_cargs(desc->argc, (const_str*)desc->argv);
}

neko_inline void neko_args_shutdown(void) {
    neko_assert(g_args.valid);
    if (g_args.args) {
        __neko_args_free(g_args.args);
        g_args.args = 0;
    }
    if (g_args.buf) {
        __neko_args_free(g_args.buf);
        g_args.buf = 0;
    }
    g_args.valid = false;
}

neko_inline bool neko_args_isvalid(void) { return g_args.valid; }

neko_inline s32 neko_args_find(const_str key) {
    neko_assert(g_args.valid && key);
    for (s32 i = 0; i < g_args.num_args; i++) {
        if (0 == strcmp(__neko_args_str(g_args.args[i].key), key)) {
            return i;
        }
    }
    return -1;
}

neko_inline s32 neko_args_num_args(void) {
    neko_assert(g_args.valid);
    return g_args.num_args;
}

neko_inline const_str neko_args_key_at(s32 index) {
    neko_assert(g_args.valid);
    if ((index >= 0) && (index < g_args.num_args)) {
        return __neko_args_str(g_args.args[index].key);
    } else {

        return __neko_args_str(0);
    }
}

neko_inline const_str neko_args_value_at(s32 index) {
    neko_assert(g_args.valid);
    if ((index >= 0) && (index < g_args.num_args)) {
        return __neko_args_str(g_args.args[index].val);
    } else {

        return __neko_args_str(0);
    }
}

neko_inline bool neko_args_exists(const_str key) {
    neko_assert(g_args.valid && key);
    return -1 != neko_args_find(key);
}

neko_inline const_str neko_args_value(const_str key) {
    neko_assert(g_args.valid && key);
    return neko_args_value_at(neko_args_find(key));
}

neko_inline const_str neko_args_value_def(const_str key, const_str def) {
    neko_assert(g_args.valid && key && def);
    s32 arg_index = neko_args_find(key);
    if (-1 != arg_index) {
        const_str res = neko_args_value_at(arg_index);
        neko_assert(res);
        if (res[0] == 0) {
            return def;
        } else {
            return res;
        }
    } else {
        return def;
    }
}

neko_inline bool neko_args_equals(const_str key, const_str val) {
    neko_assert(g_args.valid && key && val);
    return 0 == strcmp(neko_args_value(key), val);
}

neko_inline bool neko_args_boolean(const_str key) {
    if (neko_args_exists(key)) {
        const_str val = neko_args_value(key);
        return (0 == strcmp("true", val)) || (0 == strcmp("yes", val)) || (0 == strcmp("on", val)) || (0 == strcmp("", val));
    } else {
        return false;
    }
}

#endif
