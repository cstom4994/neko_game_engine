//
// The MIT License
//
// Copyright (c) 2020 Daniel "q66" Kolesa
// Copyright (c) 2024 KaoruXun
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// https://github.com/q66/cffi-lua
//

#include "luaffi.hpp"

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>

void *operator new(std::size_t n) {
    void *p = malloc(n);
    if (!p) {
        abort(); /* FIXME: do not abort */
    }
    return p;
}

void *operator new[](std::size_t n) {
    void *p = malloc(n);
    if (!p) {
        abort();
    }
    return p;
}

void operator delete(void *p) { free(p); }

void operator delete[](void *p) { free(p); }

void operator delete(void *p, std::size_t) { free(p); }

void operator delete[](void *p, std::size_t) { free(p); }

namespace util {

std::size_t write_i(char *buf, std::size_t bufsize, long long v) {
    if (v < 0) {
        if (!bufsize) {
            return write_u(buf, bufsize, static_cast<unsigned long long>(-v)) + 1;
        }
        *buf = '-';
        return write_u(buf + 1, bufsize - 1, static_cast<unsigned long long>(-v)) + 1;
    }
    return write_u(buf, bufsize, static_cast<unsigned long long>(v));
}

std::size_t write_u(char *bufp, std::size_t bufsize, unsigned long long v) {
    char buf[sizeof(unsigned long long) * CHAR_BIT];
    std::size_t ndig = 0;
    if (!v) {
        buf[0] = '0';
        ndig = 1;
    } else {
        for (; v; v /= 10) {
            buf[ndig++] = char(v % 10) + '0';
        }
    }
    if (bufsize < (ndig + 1)) {
        return ndig;
    }
    for (std::size_t i = 0; i < ndig; ++i) {
        *bufp++ = buf[ndig - i - 1];
    }
    *bufp++ = '\0';
    return ndig;
}

} /* namespace util */

namespace parser {

/* define all keywords our subset of C understands */

/* stdint types might as well also be builtin... */
#define KEYWORDS                                                                                                                                                           \
    KW(alignof), KW(alignas), KW(auto), KW(const), KW(enum), KW(extern), KW(sizeof), KW(struct), KW(signed), KW(typedef), KW(union), KW(unsigned), KW(volatile), KW(void), \
                                                                                                                                                                           \
            KW(_Alignas),                                                                                                                                                  \
                                                                                                                                                                           \
            KW(__alignof__), KW(__const__), KW(__volatile__),                                                                                                              \
                                                                                                                                                                           \
            KW(__attribute__), KW(__extension__), KW(__asm__),                                                                                                             \
                                                                                                                                                                           \
            KW(__declspec), KW(__cdecl), KW(__fastcall), KW(__stdcall), KW(__thiscall), KW(__ptr32), KW(__ptr64),                                                          \
                                                                                                                                                                           \
            KW(true), KW(false),                                                                                                                                           \
                                                                                                                                                                           \
            KW(bool), KW(char), KW(char16_t), KW(char32_t), KW(short), KW(int), KW(long), KW(wchar_t), KW(float), KW(double),                                              \
                                                                                                                                                                           \
            KW(int8_t), KW(uint8_t), KW(int16_t), KW(uint16_t), KW(int32_t), KW(uint32_t), KW(int64_t), KW(uint64_t),                                                      \
                                                                                                                                                                           \
            KW(size_t), KW(ssize_t), KW(intptr_t), KW(uintptr_t), KW(ptrdiff_t), KW(time_t),                                                                               \
                                                                                                                                                                           \
            KW(va_list), KW(__builtin_va_list), KW(__gnuc_va_list),                                                                                                        \
                                                                                                                                                                           \
            KW(_Bool)

/* primary keyword enum */

#define KW(x) TOK_##x

/* a token is an int, single-char tokens are just their ascii */
/* TOK_NAME must be the first pre-keyword token! */
enum c_token {
    TOK_CUSTOM = 257,

    TOK_EQ = TOK_CUSTOM,
    TOK_NEQ,
    TOK_GE,
    TOK_LE,
    TOK_AND,
    TOK_OR,
    TOK_LSH,
    TOK_RSH,

    TOK_ELLIPSIS,
    TOK_ATTRIBB,
    TOK_ATTRIBE,
    TOK_ARROW,

    TOK_INTEGER,
    TOK_FLOAT,
    TOK_CHAR,
    TOK_STRING,
    TOK_NAME,
    KEYWORDS
};

#undef KW

/* end primary keyword enum */

/* token strings */

#define KW(x) #x

static char const *tokens[] = {"==",        "!=",      ">=",     "<=",       "&&",     "||",    "<<", ">>",

                               "...",       "((",      "))",     "->",

                               "<integer>", "<float>", "<char>", "<string>", "<name>", KEYWORDS};

#undef KW

/* end token strings */

/* lexer */

struct lex_token {
    int token = -1;
    ast::c_expr_type numtag = ast::c_expr_type::INVALID;
    ast::c_value value{};
};

/* represents a level of parens pair when parsing a type; so e.g. in
 *
 * void (*(*(*(*&))))
 *
 * we have 4 levels.
 */
struct parser_type_level {
    parser_type_level() : arrd{0}, cv{0}, flags{0}, is_term{false}, is_func{false}, is_ref{false} {}
    ~parser_type_level() {
        if (is_func) {
            using DT = util::vector<ast::c_param>;
            argl.~DT();
        }
    }
    parser_type_level(parser_type_level &&v) : cv{v.cv}, flags{v.flags}, cconv{v.cconv}, is_term{v.is_term}, is_func{v.is_func}, is_ref{v.is_ref} {
        if (is_func) {
            new (&argl) util::vector<ast::c_param>(util::move(v.argl));
        } else {
            arrd = v.arrd;
        }
    }

    union {
        util::vector<ast::c_param> argl;
        std::size_t arrd;
    };
    std::uint32_t cv : 2;
    std::uint32_t flags : 6;
    std::uint32_t cconv : 6;
    std::uint32_t is_term : 1;
    std::uint32_t is_func : 1;
    std::uint32_t is_ref : 1;
};

/* this stack stores whatever parse_array parses, the number of elements
 * per single parse_type_ptr call is stored above in the level struct; each
 * level with non-zero arrd will pop off arrd items
 */
struct parser_array_dim {
    std::size_t size;
    std::uint32_t quals;
};

/* global parser state, one per lua_State * */
struct parser_state {
    /* a mapping from keyword id to keyword name, populated on init */
    util::str_map<int> keyword_map;
    /* all-purpose string buffer used when parsing, also for error messages */
    util::strbuf ls_buf;
    /* used when parsing types */
    util::vector<parser_type_level> plevel_queue{};
    util::vector<parser_array_dim> arrdim_stack{};
    /* stores the token id when throwing errors */
    int err_token;
    /* stores the line number when throwing errors */
    int err_lnum;
};

static void init_kwmap(util::str_map<int> &km) {
    if (!km.empty()) {
        return;
    }
    auto nkw = int(sizeof(tokens) / sizeof(tokens[0]) + TOK_CUSTOM - TOK_NAME - 1);
    for (int i = 1; i <= nkw; ++i) {
        km[tokens[TOK_NAME - TOK_CUSTOM + i]] = i;
    }
}

enum parse_mode {
    PARSE_MODE_DEFAULT,
    PARSE_MODE_TYPEDEF,
    PARSE_MODE_NOTCDEF,
    PARSE_MODE_ATTRIB,
};

/* locale independent ctype replacements */

inline int is_digit(int c) { return (c >= '0') && (c <= '9'); }

inline int is_hex_digit(int c) {
    c |= 32; /* make lowercase */
    return is_digit(c) || ((c >= 'a') && (c <= 'f'));
}

inline int is_space(int c) { return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\v') || (c == '\f') || (c == '\r')); }

inline int is_alpha(int c) {
    c |= 32; /* lowercase */
    return ((c >= 'a') && (c <= 'z'));
}

inline int is_alphanum(int c) { return is_alpha(c) || is_digit(c); }

inline int is_print(int c) {
    /* between Space and ~ */
    return (c >= 0x20) && (c <= 0x7E);
}

struct lex_state {
    lex_state() = delete;

    lex_state(lua_State *L, char const *str, const char *estr, int pmode = PARSE_MODE_DEFAULT, int paridx = -1)
        : p_mode(pmode), p_pidx(paridx), p_L(L), stream(str), send(estr), p_dstore{ast::decl_store::get_main(L)} {
        lua_getfield(L, LUA_REGISTRYINDEX, lua::CFFI_PARSER_STATE);
        if (!lua_isuserdata(L, -1)) {
            luaL_error(L, "internal error: no parser state");
        }
        p_P = lua::touserdata<parser_state>(L, -1);
        if (!p_P) {
            luaL_error(L, "internal error: parser state is null");
        }
        lua_pop(L, 1);

        /* this should be enough that we should never have to resize it */
        p_P->ls_buf.clear();
        p_P->ls_buf.reserve(256);

        /* read first char */
        next_char();

        /* skip past potential UTF-8 BOM */
        if (current != 0xEF) {
            return;
        }
        next_char();
        if (current != 0xBB) {
            return;
        }
        next_char();
        if (current != 0xBF) {
            return;
        }
        next_char();
    }

    ~lex_state() {}

    bool get() WARN_UNUSED_RET {
        if (lahead.token >= 0) {
            t = util::move(lahead);
            lahead.token = -1;
            return true;
        }
        t.token = lex(t);
        return !!t.token;
    }

    bool lookahead(int &tok) WARN_UNUSED_RET {
        tok = lahead.token = lex(t);
        return !!tok;
    }

    bool lex_error(int tok, int linenum) WARN_UNUSED_RET {
        p_P->err_token = tok;
        p_P->err_lnum = linenum;
        return false;
    }

    bool lex_error(int tok) WARN_UNUSED_RET { return lex_error(tok, line_number); }

    bool syntax_error() WARN_UNUSED_RET { return lex_error(t.token); }

    bool store_decl(ast::c_object *obj, int lnum) WARN_UNUSED_RET {
        auto *old = p_dstore.add(obj);
        if (old) {
            p_P->ls_buf.clear();
            p_P->ls_buf.append('\'');
            p_P->ls_buf.append(old->name());
            p_P->ls_buf.append("' redefined");
            return lex_error(-1, lnum);
        }
        return true;
    }

    void commit() { p_dstore.commit(); }

    ast::c_object const *lookup(char const *name) const { return p_dstore.lookup(name); }

    ast::c_object *lookup(char const *name) { return p_dstore.lookup(name); }

    std::size_t request_name(char *buf, std::size_t bufsize) { return p_dstore.request_name(buf, bufsize); }

    int mode() const { return p_mode; }

    int mode(int nmode) {
        int ret = p_mode;
        p_mode = nmode;
        return ret;
    }

    bool param_maybe_name() WARN_UNUSED_RET {
        if (t.token != '$') {
            return true;
        }
        if (!ensure_pidx()) {
            return false;
        }
        std::size_t len;
        char const *str = lua_tolstring(p_L, p_pidx, &len);
        if (!str) {
            p_P->ls_buf.set("name expected");
            return syntax_error();
        }
        /* replace $ with name */
        t.token = TOK_NAME;
        p_P->ls_buf.set(str, len);
        ++p_pidx;
        return true;
    }

    /* FIXME: very preliminary, should support more stuff, more types */
    bool param_maybe_expr() WARN_UNUSED_RET {
        if (t.token != '$') {
            return true;
        }
        if (!ensure_pidx()) {
            return false;
        }
        lua_Integer d = lua_tointeger(p_L, p_pidx);
        if (!d && !lua_isnumber(p_L, p_pidx)) {
            p_P->ls_buf.set("integer expected");
            return syntax_error();
        }
        /* replace $ with integer */
        t.token = TOK_INTEGER;
        if (d < 0) {
            t.numtag = ast::c_expr_type::LLONG;
            t.value.ll = d;
        } else {
            t.numtag = ast::c_expr_type::ULLONG;
            t.value.ull = d;
        }
        ++p_pidx;
        return true;
    }

    bool param_get_type(ast::c_type &res) WARN_UNUSED_RET {
        if (!ensure_pidx()) {
            return false;
        }
        if (!luaL_testudata(p_L, p_pidx, lua::CFFI_CDATA_MT)) {
            p_P->ls_buf.set("type expected");
            return syntax_error();
        }
        res = lua::touserdata<ast::c_type>(p_L, p_pidx)->copy();
        /* consume $ */
        if (!get()) {
            return false;
        }
        ++p_pidx;
        return true;
    }

    lua_State *lua_state() const { return p_L; }

    util::strbuf &get_buf() { return p_P->ls_buf; }

    util::strbuf const &get_buf() const { return p_P->ls_buf; }

    int err_token() const { return p_P->err_token; }

    int err_line() const { return p_P->err_lnum; }

    util::vector<parser_type_level> &type_level_queue() { return p_P->plevel_queue; }

    util::vector<parser_array_dim> &array_dim_stack() { return p_P->arrdim_stack; }

private:
    bool ensure_pidx() WARN_UNUSED_RET {
        if ((p_pidx <= 0) || lua_isnone(p_L, p_pidx)) {
            p_P->ls_buf.set("wrong number of type parameters");
            return syntax_error();
        }
        return true;
    }

    bool is_newline(int c) { return (c == '\n') || (c == '\r'); }

    char next_char() {
        char ret = char(current);
        if (stream == send) {
            current = '\0';
            return ret;
        }
        current = *(stream++);
        return ret;
    }

    char upcoming() const {
        if (stream == send) {
            return '\0';
        }
        return *stream;
    }

    void next_line() {
        int old = current;
        next_char();
        if (is_newline(current) && current != old) {
            next_char();
        }
        ++line_number; /* FIXME: handle overflow */
    }

    template <typename T>
    bool check_int_fits(unsigned long long val) {
        using U = unsigned long long;
        return (val <= U(~T(0)));
    }

    /* this doesn't deal with stuff like negative values at all, that's
     * done in the expression parser as a regular unary expression and
     * is subject to the standard rules
     */
    ast::c_expr_type get_int_type(lex_token &tok, unsigned long long val, bool decimal) {
        bool unsig = false;
        int use_long = 0;
        if ((current | 32) == 'u') {
            unsig = true;
            next_char();
            if ((current | 32) == 'l') {
                ++use_long;
                next_char();
                if ((current | 32) == 'l') {
                    ++use_long;
                    next_char();
                }
            }
        } else if ((current | 32) == 'l') {
            ++use_long;
            next_char();
            if ((current | 32) == 'l') {
                ++use_long;
                next_char();
            }
            if ((current | 32) == 'u') {
                unsig = true;
            }
        }
        /* decimals still allow explicit unsigned, for others it's implicit */
        bool aus = (unsig || !decimal);
        switch (use_long) {
            case 0:
                /* no long suffix, can be any size */
                if (!unsig && check_int_fits<int>(val)) {
                    tok.value.i = static_cast<int>(val);
                    return ast::c_expr_type::INT;
                } else if (aus && check_int_fits<unsigned int>(val)) {
                    tok.value.u = static_cast<unsigned int>(val);
                    return ast::c_expr_type::UINT;
                } else if (!unsig && check_int_fits<long>(val)) {
                    tok.value.l = static_cast<long>(val);
                    return ast::c_expr_type::LONG;
                } else if (aus && check_int_fits<unsigned long>(val)) {
                    tok.value.ul = static_cast<unsigned long>(val);
                    return ast::c_expr_type::ULONG;
                } else if (!unsig && check_int_fits<long long>(val)) {
                    tok.value.ll = static_cast<long long>(val);
                    return ast::c_expr_type::LLONG;
                } else if (aus) {
                    tok.value.ull = static_cast<unsigned long long>(val);
                    return ast::c_expr_type::ULLONG;
                }
                break;
            case 1:
                /* l suffix */
                if (!unsig && check_int_fits<long>(val)) {
                    tok.value.l = static_cast<long>(val);
                    return ast::c_expr_type::LONG;
                } else if (aus && check_int_fits<unsigned long>(val)) {
                    tok.value.ul = static_cast<unsigned long>(val);
                    return ast::c_expr_type::ULONG;
                }
                break;
            case 2:
                /* ll suffix */
                if (!unsig && check_int_fits<long long>(val)) {
                    tok.value.ll = static_cast<long long>(val);
                    return ast::c_expr_type::LLONG;
                } else if (aus) {
                    tok.value.ull = static_cast<unsigned long long>(val);
                    return ast::c_expr_type::ULLONG;
                }
                break;
            default:
                break;
        }
        /* unsuffixed decimal and doesn't fit into signed long long,
         * or explicitly marked long and out of bounds
         */
        p_P->ls_buf.set("value out of bounds");
        if (!lex_error(TOK_INTEGER)) {
            return ast::c_expr_type::INVALID;
        }
        /* unreachable */
        return ast::c_expr_type::INVALID;
    }

    template <std::size_t base, typename F, typename G>
    bool read_int_core(F &&digf, G &&convf, lex_token &tok) {
        auto &lb = p_P->ls_buf.raw();
        lb.clear();
        do {
            lb.push_back(next_char());
        } while (digf(current));
        char const *numbeg = &p_P->ls_buf[0], *numend = &p_P->ls_buf[lb.size()];
        /* go from the end */
        unsigned long long val = 0, mul = 1;
        do {
            /* standardize case */
            int dig = convf(*--numend);
            val += dig * mul;
            mul *= base;
        } while (numend != numbeg);
        /* write type and value */
        tok.numtag = get_int_type(tok, val, base == 10);
        return (tok.numtag != ast::c_expr_type::INVALID);
    }

    bool read_integer(lex_token &tok) WARN_UNUSED_RET {
        if (current == '0') {
            next_char();
            if (!current || (((current | 32) != 'x') && ((current | 32) != 'b') && !(current >= '0' && current <= '7'))) {
                /* special case: value 0 */
                tok.value.i = 0;
                tok.numtag = ast::c_expr_type::INT;
                return true;
            }
            if ((current | 32) == 'x') {
                /* hex */
                next_char();
                if (!is_hex_digit(current)) {
                    p_P->ls_buf.set("malformed integer");
                    return lex_error(TOK_INTEGER);
                }
                return read_int_core<16>(
                        is_hex_digit,
                        [](int dig) {
                            dig |= 32;
                            dig = (dig >= 'a') ? (dig - 'a' + 10) : (dig - '0');
                            return dig;
                        },
                        tok);
            } else if ((current | 32) == 'b') {
                /* binary */
                next_char();
                if ((current != '0') && (current != '1')) {
                    p_P->ls_buf.set("malformed integer");
                    return lex_error(TOK_INTEGER);
                }
                return read_int_core<2>([](int cur) { return (cur == '0') || (cur == '1'); }, [](int dig) { return (dig - '0'); }, tok);
            } else {
                /* octal */
                return read_int_core<8>([](int cur) { return (cur >= '0') && (cur <= '7'); }, [](int dig) { return (dig - '0'); }, tok);
            }
        }
        /* decimal */
        return read_int_core<10>(
                is_digit, [](int dig) { return (dig - '0'); }, tok);
    }

    bool read_escape(char &c) WARN_UNUSED_RET {
        next_char();
        switch (current) {
            case '\0':
                p_P->ls_buf.set("unterminated escape sequence");
                return lex_error(TOK_CHAR);
            case '\'':
            case '\"':
            case '\\':
            case '?':
                c = char(current);
                next_char();
                return true;
            case 'e': /* extension */
                c = 0x1B;
                next_char();
                return true;
            case 'a':
                c = '\a';
                next_char();
                return true;
            case 'b':
                c = '\b';
                next_char();
                return true;
            case 'f':
                c = '\f';
                next_char();
                return true;
            case 'n':
                c = '\n';
                next_char();
                return true;
            case 'r':
                c = '\r';
                next_char();
                return true;
            case 't':
                c = '\t';
                next_char();
                return true;
            case 'v':
                c = '\v';
                next_char();
                return true;
            case 'x': {
                next_char();
                int c1 = current, c2 = upcoming();
                if (!is_hex_digit(c1) || !is_hex_digit(c2)) {
                    p_P->ls_buf.set("malformed hex escape");
                    return lex_error(TOK_CHAR);
                }
                c1 |= 32;
                c2 |= 32;
                c1 = (c1 >= 'a') ? (c1 - 'a' + 10) : (c1 - '0');
                c2 = (c2 >= 'a') ? (c2 - 'a' + 10) : (c2 - '0');
                c = char(c2 + (c1 * 16));
                next_char();
                next_char();
                return true;
            }
            default:
                break;
        }
        if ((current >= '0') && (current <= '7')) {
            int c1 = current - '0';
            next_char();
            if ((current >= '0') && (current <= '7')) {
                /* 2 or more octal digits */
                int c2 = current - '0';
                next_char();
                if ((current >= '0') && (current <= '7')) {
                    /* 3 octal digits, may be more than 255 */
                    int c3 = current - '0';
                    next_char();
                    int r = (c3 + (c2 * 8) + (c1 * 64));
                    if (r > 0xFF) {
                        p_P->ls_buf.set("octal escape out of bounds");
                        return lex_error(TOK_CHAR);
                    }
                    c = char(r);
                    return true;
                } else {
                    /* 2 octal digits */
                    c = char(c2 + (c1 * 8));
                    return true;
                }
            } else {
                /* 1 octal digit */
                c = char(c1);
                return true;
            }
        }
        p_P->ls_buf.set("malformed escape sequence");
        return lex_error(TOK_CHAR);
    }

    int lex(lex_token &tok) WARN_UNUSED_RET {
        for (;;) switch (current) {
                case '\0':
                    return -1;
                case '\n':
                case '\r':
                    next_line();
                    continue;
                /* either comment or / */
                case '/': {
                    next_char();
                    if (current == '*') {
                        next_char();
                        while (current) {
                            if (current == '*') {
                                next_char();
                                if (current == '/') {
                                    next_char();
                                    goto cont;
                                }
                            }
                            next_char();
                        }
                        p_P->ls_buf.set("unterminated comment");
                        return int(syntax_error());
                    } else if (current != '/') {
                        /* just / */
                        return '/';
                    }
                    /* C++ style comment */
                    next_char();
                    while (current && !is_newline(current)) {
                        next_char();
                    }
                cont:
                    continue;
                }
                /* =, == */
                case '=':
                    next_char();
                    if (current == '=') {
                        next_char();
                        return TOK_EQ;
                    }
                    return '=';
                /* !, != */
                case '!':
                    next_char();
                    if (current == '=') {
                        next_char();
                        return TOK_NEQ;
                    }
                    return '!';
                /* >, >>, >= */
                case '>':
                    next_char();
                    if (current == '>') {
                        next_char();
                        return TOK_RSH;
                    } else if (current == '=') {
                        next_char();
                        return TOK_GE;
                    }
                    return '>';
                /* <, <<, <= */
                case '<':
                    next_char();
                    if (current == '<') {
                        next_char();
                        return TOK_LSH;
                    } else if (current == '=') {
                        next_char();
                        return TOK_LE;
                    }
                    return '<';
                /* &, &&, |, || */
                case '&':
                case '|': {
                    int c = current;
                    next_char();
                    if (current != c) {
                        return c;
                    }
                    return (c == '&') ? TOK_AND : TOK_OR;
                }
                /* ., ... */
                case '.': {
                    next_char();
                    if ((current != '.') || (upcoming() != '.')) {
                        return '.';
                    }
                    next_char();
                    next_char();
                    return TOK_ELLIPSIS;
                }
                /* (, (( */
                case '(': {
                    next_char();
                    if ((p_mode == PARSE_MODE_ATTRIB) && (current == '(')) {
                        next_char();
                        return TOK_ATTRIBB;
                    }
                    return '(';
                }
                /* ), )) */
                case ')': {
                    next_char();
                    if ((p_mode == PARSE_MODE_ATTRIB) && (current == ')')) {
                        next_char();
                        return TOK_ATTRIBE;
                    }
                    return ')';
                }
                /* -, -> */
                case '-': {
                    next_char();
                    if (current == '>') {
                        next_char();
                        return TOK_ARROW;
                    }
                    return '-';
                }
                /* character literal */
                case '\'': {
                    next_char();
                    if (current == '\0') {
                        p_P->ls_buf.set("unterminated literal");
                        return int(lex_error(TOK_CHAR));
                    } else if (current == '\\') {
                        if (!read_escape(tok.value.c)) {
                            return 0;
                        }
                    } else {
                        tok.value.c = char(current);
                        next_char();
                    }
                    if (current != '\'') {
                        p_P->ls_buf.set("unterminated literal");
                        return int(lex_error(TOK_CHAR));
                    }
                    next_char();
                    tok.numtag = ast::c_expr_type::CHAR;
                    return TOK_CHAR;
                }
                /* string literal */
                case '\"': {
                    auto &lb = p_P->ls_buf.raw();
                    lb.clear();
                    next_char();
                    for (;;) {
                        if (current == '\"') {
                            /* multiple string literals are one string */
                            if (upcoming() == '\"') {
                                next_char();
                                next_char();
                            } else {
                                break;
                            }
                        }
                        if (current == '\0') {
                            p_P->ls_buf.set("unterminated string");
                            return int(lex_error(TOK_STRING));
                        }
                        if (current == '\\') {
                            char c = '\0';
                            if (!read_escape(c)) {
                                return 0;
                            }
                            lb.push_back(c);
                        } else {
                            lb.push_back(char(current));
                            next_char();
                        }
                    }
                    next_char();
                    lb.push_back('\0');
                    return TOK_STRING;
                }
                /* single-char tokens, number literals, keywords, names */
                default: {
                    if (is_space(current)) {
                        next_char();
                        continue;
                    } else if (is_digit(current)) {
                        if (!read_integer(tok)) {
                            return 0;
                        }
                        return TOK_INTEGER;
                    }
                    if (is_alpha(current) || (current == '_')) {
                        /* names, keywords */
                        /* what current pointed to */
                        /* keep reading until we readh non-matching char */
                        auto &lb = p_P->ls_buf.raw();
                        lb.clear();
                        do {
                            lb.push_back(next_char());
                        } while (is_alphanum(current) || (current == '_'));
                        lb.push_back('\0');
                        /* could be a keyword? */
                        auto kwit = p_P->keyword_map.find(p_P->ls_buf.data());
                        if (kwit) {
                            return TOK_NAME + *kwit;
                        }
                        return TOK_NAME;
                    }
                    /* single-char token */
                    int c = current;
                    next_char();
                    return c;
                }
            }
    }

    int current = -1;
    int p_mode = PARSE_MODE_DEFAULT;
    int p_pidx;

    lua_State *p_L;
    parser_state *p_P;
    char const *stream;
    char const *send;

    ast::decl_store p_dstore;

public:
    int line_number = 1;
    lex_token t, lahead;
};

static char const *token_to_str(int tok, char *buf) {
    if (tok < 0) {
        return "<eof>";
    }
    if (tok < TOK_CUSTOM) {
        if (is_print(tok)) {
            buf[0] = char(tok);
            buf[1] = '\0';
        } else {
            char *bufp = buf;
            std::memcpy(bufp, "char(", 5);
            bufp += 5;
            bufp += util::write_i(bufp, 11, tok);
            *bufp++ = ')';
            *bufp++ = '\0';
        }
        return buf;
    }
    return tokens[tok - TOK_CUSTOM];
}

/* parser */

static bool error_expected(lex_state &ls, int tok) WARN_UNUSED_RET;
static bool test_next(lex_state &ls, int tok) WARN_UNUSED_RET;
static bool check(lex_state &ls, int tok) WARN_UNUSED_RET;
static bool check_next(lex_state &ls, int tok) WARN_UNUSED_RET;
static bool check_match(lex_state &ls, int what, int who, int where) WARN_UNUSED_RET;

static bool error_expected(lex_state &ls, int tok) {
    char buf[16 + sizeof("'' expected")];
    char *bufp = buf;
    *bufp++ = '\'';
    char const *tk = token_to_str(tok, bufp);
    auto tlen = std::strlen(tk);
    if (tk != bufp) {
        std::memcpy(bufp, tk, tlen);
    }
    bufp += tlen;
    std::memcpy(bufp, "' expected", sizeof("' expected"));
    ls.get_buf().set(buf);
    return ls.syntax_error();
}

static bool test_next(lex_state &ls, int tok) {
    if (ls.t.token == tok) {
        return ls.get();
    }
    return false;
}

static bool check(lex_state &ls, int tok) {
    if (ls.t.token != tok) {
        return error_expected(ls, tok);
    }
    return true;
}

static bool check_next(lex_state &ls, int tok) {
    if (!check(ls, tok)) {
        return false;
    }
    return ls.get();
}

static bool check_match(lex_state &ls, int what, int who, int where) {
    if (test_next(ls, what)) {
        return true;
    }
    if (where == ls.line_number) {
        return error_expected(ls, what);
    }
    char buf[16];
    auto &b = ls.get_buf();
    b.clear();
    b.append('\'');
    b.append(token_to_str(what, buf));
    b.append("' expected (to close '");
    b.append(token_to_str(who, buf));
    b.append("' at line ");
    util::write_i(buf, sizeof(buf), where);
    b.append(buf);
    b.append(')');
    return ls.syntax_error();
}

static ast::c_expr_binop get_binop(int tok) {
    switch (tok) {
        case '+':
            return ast::c_expr_binop::ADD;
        case '-':
            return ast::c_expr_binop::SUB;
        case '*':
            return ast::c_expr_binop::MUL;
        case '/':
            return ast::c_expr_binop::DIV;
        case '%':
            return ast::c_expr_binop::MOD;

        case TOK_EQ:
            return ast::c_expr_binop::EQ;
        case TOK_NEQ:
            return ast::c_expr_binop::NEQ;
        case '>':
            return ast::c_expr_binop::GT;
        case '<':
            return ast::c_expr_binop::LT;
        case TOK_GE:
            return ast::c_expr_binop::GE;
        case TOK_LE:
            return ast::c_expr_binop::LE;

        case TOK_AND:
            return ast::c_expr_binop::AND;
        case TOK_OR:
            return ast::c_expr_binop::OR;

        case '&':
            return ast::c_expr_binop::BAND;
        case '|':
            return ast::c_expr_binop::BOR;
        case '^':
            return ast::c_expr_binop::BXOR;
        case TOK_LSH:
            return ast::c_expr_binop::LSH;
        case TOK_RSH:
            return ast::c_expr_binop::RSH;

        default:
            return ast::c_expr_binop::INVALID;
    }
}

static ast::c_expr_unop get_unop(int tok) {
    switch (tok) {
        case '+':
            return ast::c_expr_unop::UNP;
        case '-':
            return ast::c_expr_unop::UNM;
        case '!':
            return ast::c_expr_unop::NOT;
        case '~':
            return ast::c_expr_unop::BNOT;

        default:
            return ast::c_expr_unop::INVALID;
    }
}

/* operator precedences as defined by the C standard, as ugly as that is... */

/* matches layout of c_expr_binop */
static constexpr int binprec[] = {
        -1,  // invalid

        10,  // +
        10,  // -
        11,  // *
        11,  // /
        11,  // %

        7,  // ==
        7,  // !=
        8,  // >
        8,  // <
        8,  // >=
        8,  // <=

        3,  // &&
        2,  // ||

        6,  // &
        4,  // |
        5,  // ^
        9,  // <<
        9,  // >>
};

static constexpr int unprec = 11;
static constexpr int ifprec = 1;

static bool parse_cexpr(lex_state &ls, ast::c_expr &ret);
static bool parse_cexpr_bin(lex_state &ls, int min_prec, ast::c_expr &ret);

static bool parse_type(lex_state &ls, ast::c_type &ret, util::strbuf *fpname = nullptr);

static ast::c_record const *parse_record(lex_state &ls, bool *newst = nullptr);
static ast::c_enum const *parse_enum(lex_state &ls);

static bool parse_cexpr_simple(lex_state &ls, ast::c_expr &ret) {
    auto unop = get_unop(ls.t.token);
    if (unop != ast::c_expr_unop::INVALID) {
        ast::c_expr exp;
        if (!ls.get() || !parse_cexpr_bin(ls, unprec, exp)) {
            return false;
        }
        ret.type(ast::c_expr_type::UNARY);
        ret.un.op = unop;
        ret.un.expr = new ast::c_expr{util::move(exp)};
        return true;
    }
    /* FIXME: implement non-integer constants */
    if (ls.t.token == '$') {
        if (!ls.param_maybe_expr()) {
            return false;
        }
    }
    switch (ls.t.token) {
        case TOK_INTEGER:
        case TOK_FLOAT:
        case TOK_CHAR: {
            ret.type(ls.t.numtag);
            std::memcpy(&ret.val, &ls.t.value, sizeof(ls.t.value));
            return ls.get();
        }
        case TOK_NAME: {
            auto *o = ls.lookup(ls.get_buf().data());
            if (!o || (o->obj_type() != ast::c_object_type::CONSTANT)) {
                ls.get_buf().prepend("unknown constant '");
                ls.get_buf().append('\'');
                return ls.syntax_error();
            }
            auto &ct = o->as<ast::c_constant>();
            switch (ct.type().type()) {
                case ast::C_BUILTIN_INT:
                    ret.type(ast::c_expr_type::INT);
                    break;
                case ast::C_BUILTIN_UINT:
                    ret.type(ast::c_expr_type::UINT);
                    break;
                case ast::C_BUILTIN_LONG:
                    ret.type(ast::c_expr_type::LONG);
                    break;
                case ast::C_BUILTIN_ULONG:
                    ret.type(ast::c_expr_type::ULONG);
                    break;
                case ast::C_BUILTIN_LLONG:
                    ret.type(ast::c_expr_type::LLONG);
                    break;
                case ast::C_BUILTIN_ULLONG:
                    ret.type(ast::c_expr_type::ULLONG);
                    break;
                case ast::C_BUILTIN_FLOAT:
                    ret.type(ast::c_expr_type::FLOAT);
                    break;
                case ast::C_BUILTIN_DOUBLE:
                    ret.type(ast::c_expr_type::DOUBLE);
                    break;
                case ast::C_BUILTIN_LDOUBLE:
                    ret.type(ast::c_expr_type::LDOUBLE);
                    break;
                case ast::C_BUILTIN_CHAR:
                    ret.type(ast::c_expr_type::CHAR);
                    break;
                case ast::C_BUILTIN_BOOL:
                    ret.type(ast::c_expr_type::BOOL);
                    break;
                default:
                    /* should be generally unreachable */
                    ls.get_buf().set("unknown type");
                    return ls.syntax_error();
            }
            ret.val = ct.value();
            return ls.get();
        }
        case TOK_true:
        case TOK_false: {
            ret.type(ast::c_expr_type::BOOL);
            ret.val.b = (ls.t.token == TOK_true);
            return ls.get();
        }
        case TOK_sizeof: {
            /* TODO: this should also take expressions
             * we just don't support expressions this would support yet
             */
            if (!ls.get()) {
                return false;
            }
            int line = ls.line_number;
            if (!check_next(ls, '(')) {
                return false;
            }
            ast::c_type tp{};
            if (!parse_type(ls, tp) || !check_match(ls, ')', '(', line)) {
                return false;
            }
            auto align = tp.libffi_type()->size;
            if (sizeof(unsigned long long) > sizeof(void *)) {
                ret.type(ast::c_expr_type::ULONG);
                ret.val.ul = static_cast<unsigned long>(align);
            } else {
                ret.type(ast::c_expr_type::ULLONG);
                ret.val.ull = static_cast<unsigned long long>(align);
            }
            return true;
        }
        case TOK_alignof:
        case TOK___alignof__: {
            if (!ls.get()) {
                return false;
            }
            int line = ls.line_number;
            if (!check_next(ls, '(')) {
                return false;
            }
            ast::c_type tp{};
            if (!parse_type(ls, tp) || !check_match(ls, ')', '(', line)) {
                return false;
            }
            auto align = tp.libffi_type()->alignment;
            if (sizeof(unsigned long long) > sizeof(void *)) {
                ret.type(ast::c_expr_type::ULONG);
                ret.val.ul = static_cast<unsigned long>(align);
            } else {
                ret.type(ast::c_expr_type::ULLONG);
                ret.val.ull = static_cast<unsigned long long>(align);
            }
            return true;
        }
        case '(': {
            int line = ls.line_number;
            return ls.get() && parse_cexpr(ls, ret) && check_match(ls, ')', '(', line);
        }
        default:
            break;
    }
    ls.get_buf().set("unexpected symbol");
    return ls.syntax_error();
}

static bool parse_cexpr_bin(lex_state &ls, int min_prec, ast::c_expr &lhs) {
    if (!parse_cexpr_simple(ls, lhs)) {
        return false;
    }
    for (;;) {
        bool istern = (ls.t.token == '?');
        ast::c_expr_binop op{};
        int prec;
        if (istern) {
            prec = ifprec;
        } else {
            op = get_binop(ls.t.token);
            prec = binprec[int(op)];
        }
        /* also matches when prec == -1 (for ast::c_expr_binop::INVALID) */
        if (prec < min_prec) {
            break;
        }
        if (!ls.get()) {
            return false;
        }
        if (istern) {
            ast::c_expr texp;
            if (!parse_cexpr(ls, texp)) {
                return false;
            }
            ast::c_expr fexp;
            if (!check_next(ls, ':') || !parse_cexpr_bin(ls, ifprec, fexp)) {
                return false;
            }
            ast::c_expr tern;
            tern.type(ast::c_expr_type::TERNARY);
            tern.tern.cond = new ast::c_expr{util::move(lhs)};
            tern.tern.texpr = new ast::c_expr{util::move(texp)};
            tern.tern.fexpr = new ast::c_expr{util::move(fexp)};
            lhs = util::move(tern);
            continue;
        }
        /* for right associative this would be prec, we don't
         * have those except ternary which is handled specially
         */
        int nprec = prec + 1;
        ast::c_expr rhs;
        if (!parse_cexpr_bin(ls, nprec, rhs)) {
            return false;
        }
        ast::c_expr bin;
        bin.type(ast::c_expr_type::BINARY);
        bin.bin.op = op;
        bin.bin.lhs = new ast::c_expr{util::move(lhs)};
        bin.bin.rhs = new ast::c_expr{util::move(rhs)};
        lhs = util::move(bin);
    }
    return true;
}

static bool parse_cexpr(lex_state &ls, ast::c_expr &ret) { return parse_cexpr_bin(ls, 1, ret); }

static bool get_arrsize(lex_state &ls, ast::c_expr const &exp, std::size_t &ret) {
    ast::c_expr_type et;
    ast::c_value val;
    if (!exp.eval(ls.lua_state(), val, et, true)) {
        std::size_t strl;
        char const *errm = lua_tolstring(ls.lua_state(), -1, &strl);
        ls.get_buf().set(errm, strl);
        lua_pop(ls.lua_state(), 1);
        return ls.syntax_error();
    }

    long long sval = 0;
    unsigned long long uval = 0;
    switch (et) {
        case ast::c_expr_type::INT:
            sval = val.i;
            break;
        case ast::c_expr_type::LONG:
            sval = val.l;
            break;
        case ast::c_expr_type::LLONG:
            sval = val.ll;
            break;
        case ast::c_expr_type::UINT:
            uval = val.u;
            goto done;
        case ast::c_expr_type::ULONG:
            uval = val.ul;
            goto done;
        case ast::c_expr_type::ULLONG:
            uval = val.ull;
            goto done;
        default:
            ls.get_buf().set("invalid array size");
            return ls.syntax_error();
    }
    if (sval < 0) {
        ls.get_buf().set("array size is negative");
        return ls.syntax_error();
    }
    uval = sval;

done:
    using ULL = unsigned long long;
    if (uval > ULL(~std::size_t(0))) {
        ls.get_buf().set("array sie too big");
        return ls.syntax_error();
    }
    ret = std::size_t(uval);
    return true;
}

static bool parse_cv(lex_state &ls, std::uint32_t &ret, bool *tdef = nullptr, bool *extr = nullptr) {
    ret = 0;
    for (;;) switch (ls.t.token) {
            case TOK_const:
            case TOK___const__:
                if (ret & ast::C_CV_CONST) {
                    ls.get_buf().set("duplicate const qualifier");
                    return ls.syntax_error();
                }
                if (!ls.get()) {
                    return false;
                }
                ret |= ast::C_CV_CONST;
                break;
            case TOK_volatile:
            case TOK___volatile__:
                if (ret & ast::C_CV_VOLATILE) {
                    ls.get_buf().set("duplicate volatile qualifier");
                    return ls.syntax_error();
                }
                if (!ls.get()) {
                    return false;
                }
                ret |= ast::C_CV_VOLATILE;
                break;
            case TOK_typedef:
                if (!tdef) {
                    return true;
                }
                if (*tdef) {
                    ls.get_buf().set("duplicate typedef qualifier");
                    return ls.syntax_error();
                }
                if (!ls.get()) {
                    return false;
                }
                *tdef = true;
                break;
            case TOK_extern:
                if (!extr) {
                    return true;
                }
                if (*extr) {
                    ls.get_buf().set("duplicate extern qualifier");
                    return ls.syntax_error();
                }
                if (!ls.get()) {
                    return false;
                }
                *extr = true;
                break;
            default:
                goto end;
        }
end:
    return true;
}

static bool parse_callconv_attrib(lex_state &ls, std::uint32_t &ret) {
    if (ls.t.token != TOK___attribute__) {
        ret = ast::C_FUNC_DEFAULT;
        return true;
    }
    int omod = ls.mode(PARSE_MODE_ATTRIB);
    if (!ls.get()) {
        return false;
    }
    int ln = ls.line_number;
    if (!check_next(ls, TOK_ATTRIBB)) {
        return false;
    }
    std::uint32_t conv;
    if (!check(ls, TOK_NAME)) {
        return false;
    }
    auto &b = ls.get_buf();
    if (!std::strcmp(b.data(), "cdecl")) {
        conv = ast::C_FUNC_CDECL;
    } else if (!std::strcmp(b.data(), "fastcall")) {
        conv = ast::C_FUNC_FASTCALL;
    } else if (!std::strcmp(b.data(), "stdcall")) {
        conv = ast::C_FUNC_STDCALL;
    } else if (!std::strcmp(b.data(), "thiscall")) {
        conv = ast::C_FUNC_THISCALL;
    } else {
        b.set("invalid calling convention");
        return ls.syntax_error();
    }
    if (!ls.get() || !check_match(ls, TOK_ATTRIBE, TOK_ATTRIBB, ln)) {
        return false;
    }
    ls.mode(omod);
    ret = conv;
    return true;
}

static bool parse_callconv_ms(lex_state &ls, std::uint32_t &ret) {
    switch (ls.t.token) {
        case TOK___cdecl:
            ret = ast::C_FUNC_CDECL;
            return ls.get();
        case TOK___fastcall:
            ret = ast::C_FUNC_FASTCALL;
            return ls.get();
        case TOK___stdcall:
            ret = ast::C_FUNC_STDCALL;
            return ls.get();
        case TOK___thiscall:
            ret = ast::C_FUNC_THISCALL;
            return ls.get();
        default:
            break;
    }
    ret = ast::C_FUNC_DEFAULT;
    return true;
}

static bool parse_paramlist(lex_state &ls, util::vector<ast::c_param> &params) {
    int linenum = ls.line_number;
    if (!ls.get()) {
        return false;
    }

    if (ls.t.token == TOK_void) {
        int lah = 0;
        if (!ls.lookahead(lah)) {
            return false;
        }
        if (lah == ')') {
            if (!ls.get()) {
                return false;
            }
            goto done_params;
        }
    }

    if (ls.t.token == ')') {
        goto done_params;
    }

    for (;;) {
        if (ls.t.token == TOK_ELLIPSIS) {
            /* varargs, insert a sentinel type (will be dropped) */
            params.emplace_back(util::strbuf{}, ast::c_type{ast::C_BUILTIN_VOID, 0});
            if (!ls.get()) {
                return false;
            }
            /* varargs ends the arglist */
            break;
        }
        util::strbuf pname{};
        ast::c_type pt{};
        if (!parse_type(ls, pt, &pname)) {
            return false;
        }
        /* check if argument type can be passed by value */
        if (!pt.passable()) {
            auto &b = ls.get_buf();
            b.clear();
            b.append('\'');
            pt.serialize(b);
            b.append("' cannot be passed by value");
            return ls.syntax_error();
        }
        if (pname[0] == '?') {
            pname.clear();
        }
        params.emplace_back(util::move(pname), util::move(pt));
        if (!test_next(ls, ',')) {
            break;
        }
    }

done_params:
    return check_match(ls, ')', '(', linenum);
}

/* FIXME: when in var declarations, all components must be complete */
static bool parse_array(lex_state &ls, std::size_t &ret, int &flags) {
    auto &dimstack = ls.array_dim_stack();
    flags = 0;
    std::size_t ndims = 0;
    if (ls.t.token != '[') {
        ret = ndims;
        return true;
    }
    std::uint32_t cv = 0;
    if (!ls.get() || !parse_cv(ls, cv)) {
        return false;
    }
    if (ls.t.token == ']') {
        flags |= ast::C_TYPE_NOSIZE;
        dimstack.push_back({0, cv});
        ++ndims;
        if (!ls.get()) {
            return false;
        }
    } else if (ls.t.token == '?') {
        /* FIXME: this should only be available in cdata creation contexts */
        flags |= ast::C_TYPE_VLA;
        dimstack.push_back({0, cv});
        ++ndims;
        if (!ls.get() || !check_next(ls, ']')) {
            return false;
        }
    } else {
        ast::c_expr exp;
        if (!parse_cexpr(ls, exp)) {
            return false;
        }
        std::size_t arrs;
        if (!get_arrsize(ls, util::move(exp), arrs)) {
            return false;
        }
        dimstack.push_back({arrs, cv});
        ++ndims;
        if (!check_next(ls, ']')) {
            return false;
        }
    }
    while (ls.t.token == '[') {
        if (!ls.get() || !parse_cv(ls, cv)) {
            return false;
        }
        ast::c_expr exp;
        if (!parse_cexpr(ls, exp)) {
            return false;
        }
        std::size_t arrs;
        if (!get_arrsize(ls, util::move(exp), arrs)) {
            return false;
        }
        dimstack.push_back({arrs, cv});
        ++ndims;
        if (!check_next(ls, ']')) {
            return false;
        }
    }
    ret = ndims;
    return true;
}

/* this attempts to implement the complete syntax of how types are parsed
 * in C; that means it covers pointers, function pointers, references
 * and arrays, including hopefully correct parenthesization rules and
 * binding of pointers/references and cv qualifiers...
 *
 * it also handles proper parsing and placement of name, e.g. when declaring
 * function prototypes or dealing with struct members or function arguments,
 * so you could say it handles not only parsing of types by itself, but really
 * parsing of declarations in general; if you look down in parse_decl, you can
 * see that a declaration parse is pretty much just calling parse_type()
 *
 * below is described how it works:
 */
static bool parse_type_ptr(lex_state &ls, ast::c_type &tp, util::strbuf *fpname, bool needn, bool tdef, bool &tdef_bltin) {
    /* our input is the left-side qualified type; that means constructs such
     * as 'const int' or 'unsigned long int const'
     */

    /*
     * first we define a list containing 'levels'... each level denotes one
     * matched pair of parens, except the implicit default level which is
     * always added; new level is delimited by a sentinel value, and the
     * elements past the sentinel can specify pointers and references
     *
     * this list is stored in the per-lua-state parser state struct, to avoid
     * the hassle with static thread-local constructors/destructors, while
     * still conserving resources (reused across parser runs)
     *
     * now the real fun begins, because we're going to be statekeeping
     *
     * the C function pointer syntax is quite awful, as is the array syntax
     * that follows similar conventions - let's start with the simplest
     * example of a function declaration:
     *
     * void const *func_name(int arg);
     *
     * that as a whole is a type - there is also a fairly liberal usage of
     * parenthesis allowed, so we can write this identical declaration:
     *
     * void const *(func_name)(int arg);
     *
     * now let's turn it into a function pointer declaration - in that case,
     * the parenthesis becomes mandatory, since we need to bind a pointer to
     * the function and not to the left-side type:
     *
     * void const *(*func_name)(int arg);
     *
     * now, there is this fun rule of how the '*' within the parenthesis binds
     * to something - consider this:
     *
     * void const (*ptr_name);
     *
     * this is functionally identical to:
     *
     * void const *ptr_name;
     *
     * in short, if an argument list follows a parenthesized part of the type,
     * we've created a function type, and the asterisks inside the parens will
     * now bind to the new function type rather than the 'void const *'
     *
     * the same applies to arrays:
     *
     * void const *(*thing);      // pointer to pointer to const void
     * void const *(*thing)[100]; // pointer to array of pointers
     * void const **thing[100];   // array of pointers to pointers
     * void const *(*thing[100]); // same as above
     *
     * now, what if we wanted to nest this, and create a function that returns
     * a function pointer? we put the argument list after the name:
     *
     * void const *(*func_name(int arg))(float other_arg);
     *
     * this is a plain function taking one 'int' argument that returns a
     * pointer to function taking one 'float' argument and returning a ptr
     *
     * what if we wanted to make THIS into a function pointer?
     *
     * void const *(*(*func_name)(int arg))(float other_arg);
     *
     * now we have a pointer to the same thing as above...
     *
     * this can be nested indefinitely, and arrays behave exactly the same;
     * that also means pointers/references bind left to right (leftmost '*'
     * is the 'deepest' pointer, e.g. 'void (*&foo)()' is a reference to a
     * function pointer) while argument lists bind right to left
     *
     * array dimensions for multidimensional arrays also bind right to left,
     * with leftmost dimension being the outermost (which can therefore be of
     * unknown/variable size, while the others must specify exact sizes)
     *
     * cv-qualifiers bind to the thing on their left as usual and can be
     * specified after any '*' (but not '&'); references behave the same
     * way as pointers with the difference that you can't have a reference
     * to a reference, it must terminate the chain
     *
     * this is enough background, let's parse:
     *
     * first we save the size of the level list, that will denote the boundary
     * of this specific call - it will be the first index we can access from
     * here - this is because this function can be recursive, and we can't have
     * inner calls overwriting stuff that belongs to the outer calls
     */
    auto &pcvq = ls.type_level_queue();
    auto pidx = std::intptr_t(pcvq.size());
    bool nolev = true;
    /* normally we'd consume the '(', but remember, first level is implicit */
    goto newlevel;
    do {
        if (!ls.get()) {
            return false;
        }
        nolev = false;
    newlevel:
        /* create the sentinel */
        pcvq.emplace_back();
        pcvq.back().is_term = true;
        if (!nolev) {
            std::uint32_t conv = 0;
            if (!parse_callconv_ms(ls, conv)) {
                return false;
            }
            pcvq.back().cconv = conv;
        } else {
            pcvq.back().cconv = ast::C_FUNC_DEFAULT;
        }
        /* count all '*' and create element for each */
        while (ls.t.token == '*') {
            pcvq.emplace_back();
            std::uint32_t cv = 0;
            if (!ls.get() || !parse_cv(ls, cv)) {
                return false;
            }
            pcvq.back().cv = cv;
        }
        /* references are handled the same, but we know there can only be
         * one of them; this actually does not cover all cases, since putting
         * parenthesis after this will allow you to specify another reference,
         * but filter this trivial case early on since we can */
        if (ls.t.token == '&') {
            if (!ls.get()) {
                return false;
            }
            pcvq.emplace_back();
            pcvq.back().is_ref = true;
        }
        /* we've found what might be an array dimension or an argument list,
         * so break out, we've finished parsing the early bits...
         */
        if (ls.t.token == '[') {
            break;
        } else if (ls.t.token == '(') {
            /* these indicate not an arglist, so keep going */
            int lah = 0;
            if (!ls.lookahead(lah)) {
                return false;
            }
            switch (lah) {
                case TOK___cdecl:
                case TOK___fastcall:
                case TOK___stdcall:
                case TOK___thiscall:
                case '*':
                case '&':
                case '(':
                    continue;
                default:
                    break;
            }
            break;
        }
    } while (ls.t.token == '(');
    /* this function doesn't change the list past this, so save it */
    auto tidx = std::intptr_t(pcvq.size());
    /* the most basic special case when there are no (),
     * calling convention can go before the name
     */
    if (nolev) {
        std::uint32_t conv = 0;
        if (!parse_callconv_ms(ls, conv)) {
            return false;
        }
        pcvq[pidx].cconv = conv;
        if (pcvq[pidx].cconv == ast::C_FUNC_DEFAULT) {
            if (!parse_callconv_attrib(ls, conv)) {
                return false;
            }
            pcvq[pidx].cconv = conv;
        }
    }
    /* if 'fpname' was passed, it means we might want to handle a named type
     * or declaration, with the name being optional or mandatory depending
     * on 'needn' - if name was optionally requested but not found, we write
     * a dummy value to tell the caller what happened
     */
    if (fpname) {
        if (!ls.param_maybe_name()) {
            return false;
        }
        bool check_kw = (ls.t.token == TOK_NAME);
        if (tdef) {
            /* builtins can be "redefined", but the definitions
             * are ignored, matching luajit fuzzy behavior
             */
            switch (ls.t.token) {
                case TOK_int8_t:
                case TOK_int16_t:
                case TOK_int32_t:
                case TOK_int64_t:
                case TOK_uint8_t:
                case TOK_uint16_t:
                case TOK_uint32_t:
                case TOK_uint64_t:
                case TOK_uintptr_t:
                case TOK_intptr_t:
                case TOK_ptrdiff_t:
                case TOK_ssize_t:
                case TOK_size_t:
                case TOK_va_list:
                case TOK___builtin_va_list:
                case TOK___gnuc_va_list:
                case TOK_time_t:
                case TOK_wchar_t:
                    check_kw = true;
                    tdef_bltin = true;
                    break;
                default:
                    break;
            }
        }
        if (needn || check_kw) {
            /* we're in a context where name can be provided, e.g. if
             * parsing a typedef or a prototype, this will be the name
             */
            if (!check_kw && !check(ls, TOK_NAME)) {
                return false;
            }
            *fpname = ls.get_buf();
            if (!ls.get()) {
                return false;
            }
        } else {
            fpname->set("?");
        }
    }
    /* remember when we declared that paramlists and array dimensions bind
     * right to left, rather than left to right? we now have a queue of levels
     * available, and we might be at the first, innermost argument list, or
     * maybe an array; what we do is iterate all sentinels in the queue, but
     * backwards, starting with the last defined one...
     *
     * once we've found one, we attempt to parse an argument list or an array
     * dimension depending on what was found, and if nothing was found, that
     * is fine too - this will alter to what pointers are bound to though
     *
     * in short, in 'void (*foo(argl1))(argl2)', 'argl1' will be attached to
     * level 2, while 'argl2' will be stored in level 1 (the implicit one)
     */
    std::uint32_t prevconv = ast::C_FUNC_DEFAULT;
    for (std::intptr_t ridx = tidx - 1;;) {
        if (!pcvq[ridx].is_term) { /* skip non-sentinels */
            --ridx;
            continue;
        }
        if (ls.t.token == '(') {
            /* we know it's a paramlist, since all starting '(' of levels
             * are already consumed since before
             */
            util::vector<ast::c_param> argl{};
            if (!parse_paramlist(ls, argl)) {
                return false;
            }
            auto &clev = pcvq[ridx];
            new (&clev.argl) util::vector<ast::c_param>(util::move(argl));
            clev.is_func = true;
            /* attribute style calling convention after paramlist */
            std::uint32_t conv = 0;
            if (!parse_callconv_attrib(ls, conv)) {
                return false;
            }
            clev.cconv = conv;
            if (clev.cconv == ast::C_FUNC_DEFAULT) {
                clev.cconv = prevconv;
            }
        } else if (ls.t.token == '[') {
            /* array dimensions may be multiple */
            int flags = 0;
            std::size_t arrd = 0;
            if (!parse_array(ls, arrd, flags)) {
                return false;
            }
            pcvq[ridx].arrd = arrd;
            pcvq[ridx].flags = flags;
        }
        if (!pcvq[ridx].is_func && (prevconv != ast::C_FUNC_DEFAULT)) {
            ls.get_buf().set("calling convention on non-function declaration");
            return ls.syntax_error();
        }
        prevconv = pcvq[ridx].cconv;
        --ridx;
        /* special case of the implicit level, it's not present in syntax */
        if (ridx < pidx) {
            break;
        }
        if (!check_next(ls, ')')) {
            return false;
        }
    }
    /* now that arglists and arrays are attached to their respective levels,
     * we can iterate the queue forward, and execute the appropriate binding
     * logic, which is as follows:
     *
     * we take a pointer to the 'outer' level, which is by default the implicit
     * one, and start iterating from the second level afterwards; in something
     * like a function pointer or something parenthesized, this may be another
     * level already, or it might be a bunch of pointer declarations...
     *
     * let's consider our previous, moderately complex example:
     *
     * void const *(*(*func_name)(int arg))(float other_arg);
     *
     * right now, 'tp' is 'void const *', and the first outer level is the
     * implicit one, and it has the float arglist attached to it... so, we
     * start at level 2, and do this:
     *
     * void const * -> function<void const *, (float other_arg)>
     *
     * now, our level 2 has one '*'; this binds to whatever is currently 'tp',
     * so as a result, we get:
     *
     * function<void const *, (float other_arg)> *
     *
     * and that's all, so we set the 'outer' level to level 2 and proceed to
     * level 3...
     *
     * our new 'outer' level has the int arglist, so we do this:
     *
     * function<...> * -> function<function<...>, (int arg)>
     *
     * and finally bind level 3's '*' to it, which gets us the final type,
     * which is a pointer to a function returning a pointer to a function.
     */
    parser_type_level *olev = &pcvq[pidx];
    auto &dimstack = ls.array_dim_stack();
    for (std::intptr_t cidx = pidx + 1;; ++cidx) {
        /* for the implicit level, its pointers/ref are bound to current 'tp',
         * as there is definitely no outer arglist or anything, and we need
         * to make sure return types for functions are properly built, e.g.
         *
         * void *(*foo)()
         *
         * here 'tp' is 'void' at first, we need to make it into a 'void *'
         * before proceeding to 2nd level, which will then attach the arglist
         *
         * for any further level, bind pointers and reférences to whatever is
         * 'tp' at the time, which will be a new thing if an arglist is there,
         * or the previous type if not
         */
        while ((cidx < tidx) && !pcvq[cidx].is_term) {
            /* references are trailing, we can't make pointers
             * to them nor we can make references to references
             */
            if (tp.is_ref()) {
                ls.get_buf().set("references must be trailing");
                return ls.syntax_error();
            }
            if (pcvq[cidx].is_ref) {
                tp.add_ref();
            } else {
                ast::c_type ntp{util::make_rc<ast::c_type>(util::move(tp)), pcvq[cidx].cv, ast::C_BUILTIN_PTR};
                tp = util::move(ntp);
            }
            ++cidx;
        }
        /* now attach the function or array or whatever */
        if (olev->is_func) {
            /* outer level has an arglist */
            std::uint32_t fflags = olev->cconv;
            if (!olev->argl.empty() && (olev->argl.back().type().type() == ast::C_BUILTIN_VOID)) {
                fflags |= ast::C_FUNC_VARIADIC;
                olev->argl.pop_back();
            }
            /* check if return type can be passed */
            if ((tp.type() == ast::C_BUILTIN_ARRAY) || ((tp.type() != ast::C_BUILTIN_VOID) && !tp.passable())) {
                auto &b = ls.get_buf();
                b.clear();
                b.append('\'');
                tp.serialize(b);
                b.append("' cannot be passed by value");
                return ls.syntax_error();
                break;
            }
            tp = ast::c_type{util::make_rc<ast::c_function>(util::move(tp), util::move(olev->argl), fflags), 0, false};
        } else if (olev->arrd) {
            if (tp.flex()) {
                ls.get_buf().set("only first bound of an array may have unknown size");
                return ls.syntax_error();
            }
            while (olev->arrd) {
                std::size_t dim = dimstack.back().size;
                auto quals = dimstack.back().quals;
                dimstack.pop_back();
                --olev->arrd;
                ast::c_type atp{util::make_rc<ast::c_type>(util::move(tp)), quals, dim, (!olev->arrd ? olev->flags : std::uint32_t(0))};
                tp = util::move(atp);
            }
        }
        if (cidx >= tidx) {
            break;
        }
        olev = &pcvq[cidx];
    }
    /* one last thing: if plain void type is not allowed in this context
     * and we nevertheless got it, we need to error
     */
    if ((ls.mode() == PARSE_MODE_DEFAULT) && (tp.type() == ast::C_BUILTIN_VOID)) {
        ls.get_buf().set("void type in forbidden context");
        return ls.syntax_error();
    }
    /* shrink it back to what it was, these resources can be reused later */
    pcvq.shrink(pidx);
    return true;
}

enum type_signedness { TYPE_SIGNED = 1 << 0, TYPE_UNSIGNED = 1 << 1 };

using signed_size_t =
        util::conditional_t<sizeof(std::size_t) == sizeof(char), signed char,
                            util::conditional_t<sizeof(std::size_t) == sizeof(short), short,
                                                util::conditional_t<sizeof(std::size_t) == sizeof(int), int, util::conditional_t<sizeof(std::size_t) == sizeof(long), long, long long> > > >;

static bool parse_typebase_core(lex_state &ls, ast::c_type &ret, bool *tdef, bool *extr) {
    /* left-side cv */
    std::uint32_t quals = 0;
    if (!parse_cv(ls, quals, tdef, extr)) {
        return false;
    }
    std::uint32_t squals = 0;

    /* parameterized types */
    if (ls.t.token == '$') {
        if (!ls.param_get_type(ret)) {
            return false;
        }
        ret.cv(quals);
        return true;
    }

    ast::c_builtin cbt = ast::C_BUILTIN_INVALID;

    if (ls.t.token == TOK_signed || ls.t.token == TOK_unsigned) {
        if (ls.t.token == TOK_signed) {
            squals |= TYPE_SIGNED;
        } else {
            squals |= TYPE_UNSIGNED;
        }
        if (!ls.get()) {
            return false;
        }
        /* when followed by char/short/int/long, it means signed/unsigned
         * was used as a mere qualifier, so proceed with parsing the type
         */
        switch (ls.t.token) {
            case TOK_char:
            case TOK_short:
            case TOK_int:
            case TOK_long:
                goto qualified;
            default:
                break;
        }
        /* when not followed by that, treat them as a whole type */
        if (squals & TYPE_SIGNED) {
            cbt = ast::C_BUILTIN_INT;
        } else {
            cbt = ast::C_BUILTIN_UINT;
        }
        goto newtype;
    } else if ((ls.t.token == TOK_struct) || (ls.t.token == TOK_union)) {
        auto *st = parse_record(ls);
        if (!st) {
            return false;
        }
        ret = ast::c_type{st, quals};
        return true;
    } else if (ls.t.token == TOK_enum) {
        auto *en = parse_enum(ls);
        if (!en) {
            return false;
        }
        ret = ast::c_type{en, quals};
        return true;
    }

qualified:
    if (ls.t.token == TOK_NAME) {
        /* typedef, struct, enum, var, etc. */
        auto *decl = ls.lookup(ls.get_buf().data());
        if (!decl) {
            ls.get_buf().prepend("undeclared symbol '");
            ls.get_buf().append('\'');
            return ls.syntax_error();
        }
        switch (decl->obj_type()) {
            case ast::c_object_type::TYPEDEF: {
                if (!ls.get()) {
                    return false;
                }
                ret = decl->as<ast::c_typedef>().type().copy();
                /* merge qualifiers */
                ret.cv(quals);
                return true;
            }
            case ast::c_object_type::RECORD: {
                if (!ls.get()) {
                    return false;
                }
                auto &tp = decl->as<ast::c_record>();
                ret = ast::c_type{&tp, quals};
                return true;
            }
            case ast::c_object_type::ENUM: {
                if (!ls.get()) {
                    return false;
                }
                auto &tp = decl->as<ast::c_enum>();
                ret = ast::c_type{&tp, quals};
                return true;
            }
            default: {
                ls.get_buf().prepend("symbol '");
                ls.get_buf().append("' is not a type");
                return ls.syntax_error();
            }
        }
    } else
        switch (ls.t.token) {
            /* may be a builtin type */
            case TOK_void:
                cbt = ast::C_BUILTIN_VOID;
                goto btype;
            case TOK_int8_t:
                cbt = ast::builtin_v<std::int8_t>;
                goto btype;
            case TOK_int16_t:
                cbt = ast::builtin_v<std::int16_t>;
                goto btype;
            case TOK_int32_t:
                cbt = ast::builtin_v<std::int32_t>;
                goto btype;
            case TOK_int64_t:
                cbt = ast::builtin_v<std::int64_t>;
                goto btype;
            case TOK_uint8_t:
                cbt = ast::builtin_v<std::uint8_t>;
                goto btype;
            case TOK_uint16_t:
                cbt = ast::builtin_v<std::uint16_t>;
                goto btype;
            case TOK_uint32_t:
                cbt = ast::builtin_v<std::uint32_t>;
                goto btype;
            case TOK_uint64_t:
                cbt = ast::builtin_v<std::uint64_t>;
                goto btype;
            case TOK_uintptr_t:
                cbt = ast::builtin_v<std::uintptr_t>;
                goto btype;
            case TOK_intptr_t:
                cbt = ast::builtin_v<std::intptr_t>;
                goto btype;
            case TOK_ptrdiff_t:
                cbt = ast::builtin_v<std::ptrdiff_t>;
                goto btype;
            case TOK_ssize_t:
                cbt = ast::builtin_v<signed_size_t>;
                goto btype;
            case TOK_size_t:
                cbt = ast::builtin_v<std::size_t>;
                goto btype;
            case TOK_va_list:
            case TOK___builtin_va_list:
            case TOK___gnuc_va_list:
                cbt = ast::C_BUILTIN_VA_LIST;
                goto btype;
            case TOK_time_t:
                cbt = ast::builtin_v<std::time_t>;
                goto btype;
            case TOK_wchar_t:
                cbt = ast::builtin_v<wchar_t>;
                goto btype;
            case TOK_char16_t:
                cbt = ast::builtin_v<char16_t>;
                goto btype;
            case TOK_char32_t:
                cbt = ast::builtin_v<char32_t>;
                goto btype;
            case TOK_float:
                cbt = ast::C_BUILTIN_FLOAT;
                goto btype;
            case TOK_double:
                cbt = ast::C_BUILTIN_DOUBLE;
                goto btype;
            case TOK_bool:
            case TOK__Bool:
                cbt = ast::C_BUILTIN_BOOL;
            btype:
                if (!ls.get()) {
                    return false;
                }
                break;
            case TOK_char:
                if (squals & TYPE_SIGNED) {
                    cbt = ast::C_BUILTIN_SCHAR;
                } else if (squals & TYPE_UNSIGNED) {
                    cbt = ast::C_BUILTIN_UCHAR;
                } else {
                    cbt = ast::C_BUILTIN_CHAR;
                }
                if (!ls.get()) {
                    return false;
                }
                break;
            case TOK_short:
                if (squals & TYPE_UNSIGNED) {
                    cbt = ast::C_BUILTIN_USHORT;
                } else {
                    cbt = ast::C_BUILTIN_SHORT;
                }
                if (!ls.get()) {
                    return false;
                }
                if ((ls.t.token == TOK_int) && !ls.get()) {
                    return false;
                }
                break;
            case TOK_int:
                if (squals & TYPE_UNSIGNED) {
                    cbt = ast::C_BUILTIN_UINT;
                } else {
                    cbt = ast::C_BUILTIN_INT;
                }
                if (!ls.get()) {
                    return false;
                }
                break;
            case TOK_long:
                if (!ls.get()) {
                    return false;
                }
                if (ls.t.token == TOK_long) {
                    if (squals & TYPE_UNSIGNED) {
                        cbt = ast::C_BUILTIN_ULLONG;
                    } else {
                        cbt = ast::C_BUILTIN_LLONG;
                    }
                    if (!ls.get()) {
                        return false;
                    }
                } else if (ls.t.token == TOK_int) {
                    if (squals & TYPE_UNSIGNED) {
                        cbt = ast::C_BUILTIN_ULONG;
                    } else {
                        cbt = ast::C_BUILTIN_LONG;
                    }
                    if (!ls.get()) {
                        return false;
                    }
                } else if (ls.t.token == TOK_double) {
                    cbt = ast::C_BUILTIN_LDOUBLE;
                    if (!ls.get()) {
                        return false;
                    }
                } else if (squals & TYPE_UNSIGNED) {
                    cbt = ast::C_BUILTIN_ULONG;
                } else {
                    cbt = ast::C_BUILTIN_LONG;
                }
                break;
            default:
                ls.get_buf().set("type name expected");
                return ls.syntax_error();
        }

newtype:
    assert(cbt != ast::C_BUILTIN_INVALID);
    ret = ast::c_type{cbt, quals};
    return true;
}

static bool parse_typebase(lex_state &ls, ast::c_type &ret, bool *tdef = nullptr, bool *extr = nullptr) {
    if (!parse_typebase_core(ls, ret, tdef, extr)) {
        return false;
    }
    /* right-side cv that can always apply */
    std::uint32_t cv = 0;
    if (!parse_cv(ls, cv, tdef, extr)) {
        return false;
    }
    ret.cv(cv);
    return true;
}

static bool parse_type(lex_state &ls, ast::c_type &ret, util::strbuf *fpn) {
    bool tdef_bltin = false;
    return (parse_typebase(ls, ret) && parse_type_ptr(ls, ret, fpn, false, false, tdef_bltin));
}

static ast::c_record const *parse_record(lex_state &ls, bool *newst) {
    int sline = ls.line_number;
    bool is_uni = (ls.t.token == TOK_union);
    /* struct/union keyword */
    if (!ls.get()) {
        return nullptr;
    }
    /* name is optional */
    bool named = false;
    util::strbuf sname{is_uni ? "union " : "struct "};
    if (!ls.param_maybe_name()) {
        return nullptr;
    }
    if (ls.t.token == TOK_NAME) {
        sname.append(ls.get_buf());
        if (!ls.get()) {
            return nullptr;
        }
        named = true;
    } else {
        char buf[32];
        auto wn = ls.request_name(buf, sizeof(buf));
        static_cast<void>(wn); /* silence NDEBUG warnings */
        assert(wn < sizeof(buf));
        sname.append(buf);
    }

    int linenum = ls.line_number;

    auto mode_error = [&ls, named]() -> bool {
        if (named && (ls.mode() == PARSE_MODE_NOTCDEF)) {
            ls.get_buf().set("struct declaration not allowed in this context");
            return ls.syntax_error();
        }
        return true;
    };

    /* opaque */
    if (!test_next(ls, '{')) {
        auto *oldecl = ls.lookup(sname.data());
        if (!oldecl || (oldecl->obj_type() != ast::c_object_type::RECORD)) {
            if (!mode_error()) {
                return nullptr;
            }
            /* different type or not stored yet, raise error or store */
            auto *p = new ast::c_record{util::move(sname), is_uni};
            if (!ls.store_decl(p, sline)) {
                return nullptr;
            }
            return p;
        }
        return &oldecl->as<ast::c_record>();
    }

    if (!mode_error()) {
        return nullptr;
    }

    util::vector<ast::c_record::field> fields;

    while (ls.t.token != '}') {
        ast::c_type tpb{};
        if ((ls.t.token == TOK_struct) || (ls.t.token == TOK_union)) {
            bool transp = false;
            auto *st = parse_record(ls, &transp);
            if (!st) {
                return nullptr;
            }
            if (transp && test_next(ls, ';')) {
                fields.emplace_back(util::strbuf{}, ast::c_type{st, 0});
                continue;
            }
            std::uint32_t cv = 0;
            if (!parse_cv(ls, cv)) {
                return nullptr;
            }
            tpb = ast::c_type{st, cv};
        } else {
            if (!parse_typebase(ls, tpb)) {
                return nullptr;
            }
        }
        bool flexible = false;
        do {
            util::strbuf fpn;
            auto tp = tpb.copy();
            bool tdef_bltin = false;
            if (!parse_type_ptr(ls, tp, &fpn, false, false, tdef_bltin)) {
                return nullptr;
            }
            if (fpn[0] == '?') {
                /* nameless field declarations do nothing */
                goto field_end;
            }
            flexible = tp.flex();
            fields.emplace_back(util::move(fpn), util::move(tp));
            /* flexible array must be the last in the list */
            if (flexible) {
                break;
            }
        } while (test_next(ls, ','));
    field_end:
        if (!check_next(ls, ';')) {
            return nullptr;
        }
        /* flexible array must be the last in the struct */
        if (flexible) {
            break;
        }
    }

    if (!check_match(ls, '}', '{', linenum)) {
        return nullptr;
    }

    auto *oldecl = ls.lookup(sname.data());
    if (oldecl && (oldecl->obj_type() == ast::c_object_type::RECORD)) {
        auto &st = oldecl->as<ast::c_record>();
        if (st.opaque()) {
            /* previous declaration was opaque; prevent redef errors */
            st.set_fields(util::move(fields));
            if (newst) {
                *newst = true;
            }
            return &st;
        }
    }

    if (newst) {
        *newst = true;
    }
    auto *p = new ast::c_record{util::move(sname), util::move(fields), is_uni};
    if (!ls.store_decl(p, sline)) {
        return nullptr;
    }
    return p;
}

static ast::c_enum const *parse_enum(lex_state &ls) {
    int eline = ls.line_number;
    if (!ls.get()) {
        return nullptr;
    }
    /* name is optional */
    bool named = false;
    util::strbuf ename{"enum "};
    if (!ls.param_maybe_name()) {
        return nullptr;
    }
    if (ls.t.token == TOK_NAME) {
        ename.append(ls.get_buf());
        if (!ls.get()) {
            return nullptr;
        }
        named = true;
    } else {
        char buf[32];
        auto wn = ls.request_name(buf, sizeof(buf));
        static_cast<void>(wn); /* silence NDEBUG warnings */
        assert(wn < sizeof(buf));
        ename.append(buf);
    }

    int linenum = ls.line_number;

    auto mode_error = [&ls, named]() -> bool {
        if (named && (ls.mode() == PARSE_MODE_NOTCDEF)) {
            ls.get_buf().set("enum declaration not allowed in this context");
            return ls.syntax_error();
        }
        return true;
    };

    if (!test_next(ls, '{')) {
        auto *oldecl = ls.lookup(ename.data());
        if (!oldecl || (oldecl->obj_type() != ast::c_object_type::ENUM)) {
            if (!mode_error()) {
                return nullptr;
            }
            auto *p = new ast::c_enum{util::move(ename)};
            if (!ls.store_decl(p, eline)) {
                return nullptr;
            }
            return p;
        }
        return &oldecl->as<ast::c_enum>();
    }

    if (!mode_error()) {
        return nullptr;
    }

    util::vector<ast::c_enum::field> fields;

    while (ls.t.token != '}') {
        int eln = ls.line_number;
        if (!ls.param_maybe_name() || !check(ls, TOK_NAME)) {
            return nullptr;
        }
        util::strbuf fname{ls.get_buf()};
        if (!ls.get()) {
            return nullptr;
        }
        if (ls.t.token == '=') {
            eln = ls.line_number;
            ast::c_expr exp;
            if (!ls.get() || !parse_cexpr(ls, exp)) {
                return nullptr;
            }
            ast::c_expr_type et;
            ast::c_value val;
            if (!exp.eval(ls.lua_state(), val, et, true)) {
                std::size_t strl;
                char const *errm = lua_tolstring(ls.lua_state(), -1, &strl);
                ls.get_buf().set(errm, strl);
                lua_pop(ls.lua_state(), 1);
                if (!ls.syntax_error()) {
                    return nullptr;
                }
            }
            /* for now large types just get truncated */
            switch (et) {
                case ast::c_expr_type::INT:
                    break;
                case ast::c_expr_type::UINT:
                    val.i = val.u;
                    break;
                case ast::c_expr_type::LONG:
                    val.i = int(val.l);
                    break;
                case ast::c_expr_type::ULONG:
                    val.i = int(val.ul);
                    break;
                case ast::c_expr_type::LLONG:
                    val.i = int(val.ll);
                    break;
                case ast::c_expr_type::ULLONG:
                    val.i = int(val.ull);
                    break;
                default:
                    ls.get_buf().set("unsupported type");
                    if (!ls.syntax_error()) {
                        return nullptr;
                    }
            }
            fields.emplace_back(util::move(fname), val.i);
        } else {
            fields.emplace_back(util::move(fname), fields.empty() ? 0 : (fields.back().value + 1));
        }
        /* enums: register fields as constant values
         * FIXME: don't hardcode like this
         */
        auto &fld = fields.back();
        ast::c_value fval;
        fval.i = fld.value;
        auto *p = new ast::c_constant{fld.name, ast::c_type{ast::C_BUILTIN_INT, 0}, fval};
        if (!ls.store_decl(p, eln)) {
            return nullptr;
        }
        if (ls.t.token != ',') {
            break;
        } else if (!ls.get()) {
            return nullptr;
        }
    }

    if (!check_match(ls, '}', '{', linenum)) {
        return nullptr;
    }

    auto *oldecl = ls.lookup(ename.data());
    if (oldecl && (oldecl->obj_type() == ast::c_object_type::ENUM)) {
        auto &st = oldecl->as<ast::c_enum>();
        if (st.opaque()) {
            /* previous declaration was opaque; prevent redef errors */
            st.set_fields(util::move(fields));
            return &st;
        }
    }

    auto *p = new ast::c_enum{util::move(ename), util::move(fields)};
    if (!ls.store_decl(p, eline)) {
        return nullptr;
    }
    return p;
}

static bool parse_decl(lex_state &ls) {
    int dline = ls.line_number;
    std::uint32_t cconv = 0;
    if (!parse_callconv_attrib(ls, cconv)) {
        return false;
    }
    bool tdef = false, extr = false;
    ast::c_type tpb{};
    if (!parse_typebase(ls, tpb, &tdef, &extr)) {
        return false;
    }
    bool first = true;
    do {
        util::strbuf dname;
        int oldmode = 0;
        if (tdef) {
            oldmode = ls.mode(PARSE_MODE_TYPEDEF);
        }
        auto tp = tpb.copy();
        bool tdef_bltin = false;
        if (!parse_type_ptr(ls, tp, &dname, !first, tdef, tdef_bltin)) {
            return false;
        }
        first = false;
        if (cconv != ast::C_FUNC_DEFAULT) {
            if (tp.type() != ast::C_BUILTIN_FUNC) {
                ls.get_buf().set("calling convention on non-function declaration");
                return ls.syntax_error();
            }
            auto *func = const_cast<ast::c_function *>(&*tp.function());
            func->callconv(cconv);
        }
        if (tdef) {
            ls.mode(oldmode);
            if (dname[0] != '?') {
                /* redefinition of builtin, skip */
                if (tdef_bltin) {
                    continue;
                }
                /* store if the name is non-empty, if it's empty there is no
                 * way to access the type and it'd be unique either way
                 */
                if (!ls.store_decl(new ast::c_typedef{util::move(dname), util::move(tp)}, dline)) {
                    return false;
                }
                continue;
            } else {
                /* unnamed typedef must not be a list */
                break;
            }
        } else if (dname[0] == '?') {
            /* if no name is permitted, it must be the only one */
            break;
        }
        util::strbuf sym;
        /* symbol redirection */
        if (test_next(ls, TOK___asm__)) {
            int lnum = ls.line_number;
            if (!check_next(ls, '(') || !check(ls, TOK_STRING)) {
                return false;
            }
            if (ls.get_buf().empty()) {
                ls.get_buf().set("empty symbol name");
                return ls.syntax_error();
            }
            sym = ls.get_buf();
            if (!ls.get() || !check_match(ls, ')', '(', lnum)) {
                return false;
            }
        }
        if (!ls.store_decl(new ast::c_variable{util::move(dname), util::move(sym), util::move(tp)}, dline)) {
            return false;
        }
    } while (test_next(ls, ','));
    return true;
}

static bool parse_decls(lex_state &ls) {
    while (ls.t.token >= 0) {
        if (ls.t.token == ';') {
            /* empty statement */
            if (!ls.get()) {
                return false;
            }
            continue;
        }
        if (!parse_decl(ls)) {
            return false;
        }
        if (!ls.t.token) {
            break;
        }
        if (!check_next(ls, ';')) {
            return false;
        }
    }
    return true;
}

static void parse_err(lua_State *L) {
    luaL_where(L, 1);
    lua_insert(L, -2);
    lua_concat(L, 2);
    lua_error(L);
}

void parse(lua_State *L, char const *input, char const *iend, int paridx) {
    if (!iend) {
        iend = input + std::strlen(input);
    }
    {
        lex_state ls{L, input, iend, PARSE_MODE_DEFAULT, paridx};
        if (!ls.get() || !parse_decls(ls)) {
            if (ls.err_token() > 0) {
                char buf[16];
                lua_pushfstring(L, "input:%d: %s near '%s'", ls.err_line(), ls.get_buf().data(), token_to_str(ls.err_token(), buf));
            } else {
                lua_pushfstring(L, "input:%d: %s", ls.err_line(), ls.get_buf().data());
            }
            goto lerr;
        }
        ls.commit();
        return;
    }
lerr:
    parse_err(L);
}

ast::c_type parse_type(lua_State *L, char const *input, char const *iend, int paridx) {
    if (!iend) {
        iend = input + std::strlen(input);
    }
    {
        lex_state ls{L, input, iend, PARSE_MODE_NOTCDEF, paridx};
        ast::c_type tp{};
        if (!ls.get() || !parse_type(ls, tp) || !check(ls, -1)) {
            if (ls.err_token() > 0) {
                char buf[16];
                lua_pushfstring(L, "%s near '%s'", ls.get_buf().data(), token_to_str(ls.err_token(), buf));
            } else {
                lua_pushfstring(L, "%s", ls.get_buf().data());
            }
            goto lerr;
        }
        ls.commit();
        return tp;
    }
lerr:
    parse_err(L);
    /* unreachable */
    return ast::c_type{};
}

ast::c_expr_type parse_number(lua_State *L, ast::c_value &v, char const *input, char const *iend) {
    if (!iend) {
        iend = input + std::strlen(input);
    }
    {
        lex_state ls{L, input, iend, PARSE_MODE_NOTCDEF};
        if (!ls.get() || !check(ls, TOK_INTEGER)) {
            if (ls.err_token() > 0) {
                char buf[16];
                lua_pushfstring(L, "%s near '%s'", ls.get_buf().data(), token_to_str(ls.err_token(), buf));
            } else {
                lua_pushfstring(L, "%s", ls.get_buf().data());
            }
            goto lerr;
        }
        v = ls.t.value;
        ls.commit();
        return ls.t.numtag;
    }
lerr:
    parse_err(L);
    /* unreachable */
    return ast::c_expr_type{};
}

void init(lua_State *L) {
    /* init parser state for each lua state */
    auto *p = static_cast<parser_state *>(lua_newuserdata(L, sizeof(parser_state)));
    new (p) parser_state{};
    /* make sure its destructor is invoked later */
    lua_newtable(L); /* parser_state metatable */
    lua_pushcfunction(L, [](lua_State *LL) -> int {
        auto *pp = lua::touserdata<parser_state>(LL, 1);
        pp->~parser_state();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    /* store */
    lua_setfield(L, LUA_REGISTRYINDEX, lua::CFFI_PARSER_STATE);
    /* initialize keywords */
    init_kwmap(p->keyword_map);
}

} /* namespace parser */

#ifdef FFI_USE_DLFCN
#include <dlfcn.h>

#include <cstdio>
#include <cstring>

#endif

#if FFI_OS == FFI_OS_WINDOWS
#include <cstdlib>
#endif

namespace lib {

static int make_cache(lua_State *L) {
    lua_newtable(L);
    return luaL_ref(L, LUA_REGISTRYINDEX);
}

#ifdef FFI_USE_DLFCN

#ifdef FFI_OS_CYGWIN
#define FFI_DL_SOPREFIX "cyg"
#else
#define FFI_DL_SOPREFIX "lib"
#endif

#if FFI_OS == FFI_OS_OSX
#define FFI_DL_SONAME "%s.dylib"
#elif defined(FFI_OS_CYGWIN)
#define FFI_DL_SONAME "%s.dll"
#else
#define FFI_DL_SONAME "%s.so"
#endif

#if defined(RTLD_DEFAULT)
#define FFI_DL_DEFAULT RTLD_DEFAULT
#elif FFI_OS == FFI_OS_BSD || FFI_OS == FFI_OS_OSX
#define FFI_DL_DEFAULT reinterpret_cast<void *>(std::intptr_t(-2))
#else
#define FFI_DL_DEFAULT nullptr
#endif

/* low level dlfcn handling */

static handle open(char const *path, bool global) { return dlopen(path, RTLD_LAZY | (global ? RTLD_GLOBAL : RTLD_LOCAL)); }

void close(c_lib *cl, lua_State *L) {
    luaL_unref(L, LUA_REGISTRYINDEX, cl->cache);
    cl->cache = LUA_REFNIL;
    if (cl->h != FFI_DL_DEFAULT) {
        dlclose(cl->h);
    }
    cl->h = nullptr;
}

static void *get_sym(c_lib const *cl, char const *name) { return dlsym(cl->h, name); }

/* library resolution */

static char const *resolve_name(lua_State *L, char const *name) {
    if (std::strchr(name, '/')
#ifdef FFI_OS_CYGWIN
        || std::strchr(name, '\\')
#endif
    ) {
        /* input is a path */
        lua_pushstring(L, name);
        return lua_tostring(L, -1);
    }
    if (!std::strchr(name, '.')) {
        /* name without ext */
        lua_pushfstring(L, FFI_DL_SONAME, name);
    } else {
        lua_pushstring(L, name);
#ifdef FFI_OS_CYGWIN
        /* name with ext on cygwin can be used directly (no prefix) */
        return lua_tostring(L, -1);
#endif
    }
    if (!std::strncmp(name, FFI_DL_SOPREFIX, sizeof(FFI_DL_SOPREFIX) - 1)) {
        /* lib/cyg prefix found */
        return lua_tostring(L, -1);
    }
    /* no prefix, so prepend it */
    lua_pushliteral(L, FFI_DL_SOPREFIX);
    lua_insert(L, -2);
    lua_concat(L, 2);
    return lua_tostring(L, -1);
}

/* ldscript handling logic generally adapted from luajit... */

static bool check_ldscript(char const *buf, char const *&beg, char const *&end) {
    char const *p;
    if ((!std::strncmp(buf, "GROUP", 5) || !std::strncmp(buf, "INPUT", 5)) && (p = std::strchr(buf, '('))) {
        while (*++p == ' ') {
        }
        char const *e = p;
        while (*e && (*e != ' ') && (*e != ')')) {
            ++e;
        }
        beg = p;
        end = e;
        return true;
    }
    return false;
}

static bool resolve_ldscript(lua_State *L, char const *nbeg, char const *nend) {
    lua_pushlstring(L, nbeg, nend - nbeg);
    FILE *f = std::fopen(lua_tostring(L, -1), "r");
    lua_pop(L, 1);
    if (!f) {
        return false;
    }
    char buf[256];
    if (!std::fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return false;
    }
    char const *pb, *pe;
    bool got = false;
    if (!std::strncmp(buf, "/* GNU ld script", 16)) {
        while (fgets(buf, sizeof(buf), f)) {
            got = check_ldscript(buf, pb, pe);
            if (got) {
                break;
            }
        }
    } else {
        got = check_ldscript(buf, pb, pe);
    }
    std::fclose(f);
    if (got) {
        lua_pushlstring(L, pb, pe - pb);
    }
    return got;
}

void load(c_lib *cl, char const *path, lua_State *L, bool global) {
    if (!path) {
        /* primary namespace */
        cl->h = FFI_DL_DEFAULT;
        cl->cache = make_cache(L);
        lua::mark_lib(L);
        return;
    }
    handle h = open(resolve_name(L, path), global);
    lua_pop(L, 1);
    if (h) {
        lua::mark_lib(L);
        cl->h = h;
        cl->cache = make_cache(L);
        return;
    }
    char const *err = dlerror(), *e;
    if (err && (*err == '/') && (e = std::strchr(err, ':')) && resolve_ldscript(L, err, e)) {
        h = open(lua_tostring(L, -1), global);
        lua_pop(L, 1);
        if (h) {
            lua::mark_lib(L);
            cl->h = h;
            cl->cache = make_cache(L);
            return;
        }
        err = dlerror();
    }
    luaL_error(L, err ? err : "dlopen() failed");
}

bool is_c(c_lib const *cl) { return (cl->h == FFI_DL_DEFAULT); }

#elif FFI_OS == FFI_OS_WINDOWS /* FFI_USE_DLFCN */

/* This is generally adapted from LuaJIT source code.
 *
 * Didn't bother with the UWP bits yet; that may be added later if anybody
 * actually needs that (Lua does not support dynamic modules with UWP though,
 * so only the library version would work)
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 4
#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT 2
BOOL WINAPI GetModuleHandleExA(DWORD, LPCSTR, HMODULE *);
#endif

#define FFI_DL_DEFAULT reinterpret_cast<void *>(std::intptr_t(-1))

enum { FFI_DL_HANDLE_EXE = 0, FFI_DL_HANDLE_DLL, FFI_DL_HANDLE_CRT, FFI_DL_HANDLE_KERNEL32, FFI_DL_HANDLE_USER32, FFI_DL_HANDLE_GDI32, FFI_DL_HANDLE_MAX };

static void *ffi_dl_handle[FFI_DL_HANDLE_MAX] = {0};

static void dl_error(lua_State *L, char const *fmt, char const *name) {
    auto err = GetLastError();
    wchar_t wbuf[128];
    char buf[256];
    if (!FormatMessageW(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, wbuf, sizeof(wbuf) / sizeof(wchar_t), nullptr) ||
        !WideCharToMultiByte(CP_ACP, 0, wbuf, 128, buf, 256, nullptr, nullptr)) {
        buf[0] = '\0';
    }
    luaL_error(L, fmt, name, buf);
}

static bool dl_need_ext(char const *s) {
    while (*s) {
        if ((*s == '/') || (*s == '\\') || (*s == '.')) {
            return false;
        }
        ++s;
    }
    return true;
}

static char const *dl_ext_name(lua_State *L, char const *name) {
    lua_pushstring(L, name);
    if (dl_need_ext(name)) {
        lua_pushliteral(L, ".dll");
        lua_concat(L, 2);
    }
    return lua_tostring(L, -1);
}

void load(c_lib *cl, char const *path, lua_State *L, bool) {
    if (!path) {
        /* primary namespace */
        cl->h = FFI_DL_DEFAULT;
        cl->cache = make_cache(L);
        lua::mark_lib(L);
        return;
    }
    auto olderr = GetLastError();
    handle h = static_cast<handle>(LoadLibraryExA(dl_ext_name(L, path), nullptr, 0));
    lua_pop(L, 1);
    if (!h) {
        dl_error(L, "cannot load module '%s': %s", path);
    }
    SetLastError(olderr);
    cl->h = h;
    cl->cache = make_cache(L);
    lua::mark_lib(L);
}

void close(c_lib *cl, lua_State *L) {
    luaL_unref(L, LUA_REGISTRYINDEX, cl->cache);
    cl->cache = LUA_REFNIL;
    if (cl->h == FFI_DL_DEFAULT) {
        for (int i = FFI_DL_HANDLE_KERNEL32; i < FFI_DL_HANDLE_MAX; ++i) {
            void *p = ffi_dl_handle[i];
            if (p) {
                ffi_dl_handle[i] = nullptr;
                FreeLibrary(static_cast<HMODULE>(cl->h));
            }
        }
    } else if (cl->h) {
        FreeLibrary(static_cast<HMODULE>(cl->h));
    }
}

static void *get_sym(c_lib const *cl, char const *name) {
    if (cl->h != FFI_DL_DEFAULT) {
        return util::pun<void *>(GetProcAddress(static_cast<HMODULE>(cl->h), name));
    }
    for (std::size_t i = 0; i < FFI_DL_HANDLE_MAX; ++i) {
        if (!ffi_dl_handle[i]) {
            HMODULE h = nullptr;
            char const *dlh = nullptr;
            switch (i) {
                case FFI_DL_HANDLE_EXE:
                    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr, &h);
                    break;
                case FFI_DL_HANDLE_CRT: {
                    dlh = util::pun<char const *>(&_fmode);
                    goto handle_dll;
                }
                case FFI_DL_HANDLE_DLL:
                    dlh = util::pun<char const *>(ffi_dl_handle);
                handle_dll:
                    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, dlh, &h);
                    break;
                case FFI_DL_HANDLE_KERNEL32:
                    h = LoadLibraryExA("kernel32.dll", nullptr, 0);
                    break;
                case FFI_DL_HANDLE_USER32:
                    h = LoadLibraryExA("user32.dll", nullptr, 0);
                    break;
                case FFI_DL_HANDLE_GDI32:
                    h = LoadLibraryExA("gdi32.dll", nullptr, 0);
                    break;
                default:
                    break;
            }
            if (!h) {
                continue;
            }
            ffi_dl_handle[i] = static_cast<void *>(h);
        }
        HMODULE h = static_cast<HMODULE>(ffi_dl_handle[i]);
        auto paddr = GetProcAddress(h, name);
        if (paddr) {
            return util::pun<void *>(paddr);
        }
    }
    return nullptr;
}

bool is_c(c_lib const *cl) { return (cl->h == FFI_DL_DEFAULT); }

#else

void load(c_lib *, char const *, lua_State *L, bool) {
    luaL_error(L, "no support for dynamic library loading on this target");
    return nullptr;
}

void close(c_lib *, lua_State *) {}

static void *get_sym(c_lib const *, char const *) { return nullptr; }

bool is_c(c_lib const *) { return true; }

#endif /* FFI_USE_DLFCN, FFI_OS == FFI_OS_WINDOWS */

void *get_sym(c_lib const *cl, lua_State *L, char const *name) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, cl->cache);
    lua_getfield(L, -1, name);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        void *p = get_sym(cl, name);
        if (!p) {
            lua_pop(L, 1);
            luaL_error(L, "undefined symbol: %s", name);
            return nullptr;
        }
        lua_pushlightuserdata(L, p);
        lua_setfield(L, -2, name);
        lua_pop(L, 1);
        return p;
    }
    void *p = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return p;
}

} /* namespace lib */

/* sets up the metatable for library, i.e. the individual namespaces
 * of loaded shared libraries as well as the primary C namespace.
 */
struct lib_meta {
    static int gc(lua_State *L) {
        lib::close(lua::touserdata<lib::c_lib>(L, 1), L);
        return 0;
    }

    static int tostring(lua_State *L) {
        auto *cl = lua::touserdata<lib::c_lib>(L, 1);
        if (lib::is_c(cl)) {
            lua_pushfstring(L, "library: default");
        } else {
            lua_pushfstring(L, "library: %p", cl->h);
        }
        return 1;
    }

    static int index(lua_State *L) {
        auto dl = lua::touserdata<lib::c_lib>(L, 1);
        ffi::get_global(L, dl, luaL_checkstring(L, 2));
        return 1;
    }

    static int newindex(lua_State *L) {
        auto dl = lua::touserdata<lib::c_lib>(L, 1);
        ffi::set_global(L, dl, luaL_checkstring(L, 2), 3);
        return 0;
    }

    static void setup(lua_State *L) {
        if (!luaL_newmetatable(L, lua::CFFI_LIB_MT)) {
            luaL_error(L, "unexpected error: registry reinitialized");
        }

        lua_pushliteral(L, "ffi");
        lua_setfield(L, -2, "__metatable");

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");

        lua_pushcfunction(L, index);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, newindex);
        lua_setfield(L, -2, "__newindex");

        lua_pushcfunction(L, tostring);
        lua_setfield(L, -2, "__tostring");

        lua_setmetatable(L, -2);
        lua_setfield(L, -2, "C");
    }
};

/* used by all kinds of cdata
 *
 * there are several kinds of cdata:
 * - callable cdata (functions)
 * - indexable cdata (pointers, arrays)
 * - value cdata (primitives)
 */
struct cdata_meta {
    static int gc(lua_State *L) {
        ffi::destroy_cdata(L, ffi::tocdata(L, 1));
        return 0;
    }

    static int metatype_getmt(lua_State *L, int idx, int &mflags) {
        auto &cd = ffi::tocdata(L, idx);
        auto *decl = &cd.decl;
        auto tp = decl->type();
        if (tp == ast::C_BUILTIN_RECORD) {
            return cd.decl.record().metatype(mflags);
        } else if (tp == ast::C_BUILTIN_PTR) {
            if (cd.decl.ptr_base().type() != ast::C_BUILTIN_RECORD) {
                return LUA_REFNIL;
            }
            return cd.decl.ptr_base().record().metatype(mflags);
        }
        return LUA_REFNIL;
    }

    template <ffi::metatype_flag flag>
    static inline bool metatype_check(lua_State *L, int idx) {
        int mflags = 0;
        int mtp = metatype_getmt(L, idx, mflags);
        if (!(mflags & flag)) {
            return false;
        }
        return ffi::metatype_getfield(L, mtp, ffi::metafield_name(flag));
    }

    static int tostring(lua_State *L) {
        if (metatype_check<ffi::METATYPE_FLAG_TOSTRING>(L, 1)) {
            lua_pushvalue(L, 1);
            lua_call(L, 1, 1);
            return 1;
        }
        auto &cd = ffi::tocdata(L, 1);
        if (ffi::isctype(cd)) {
#if LUA_VERSION_NUM > 502
            if (metatype_check<ffi::METATYPE_FLAG_NAME>(L, 1)) {
                /* __name is respected only when it's specifically
                 * a string, otherwise ignore like if it didn't exist
                 */
                if (lua_type(L, -1) == LUA_TSTRING) {
                    return 1;
                }
                lua_pop(L, 1);
            }
#endif
            lua_pushliteral(L, "ctype<");
            cd.decl.serialize(L);
            lua_pushliteral(L, ">");
            lua_concat(L, 3);
            return 1;
        }
#if LUA_VERSION_NUM > 502
        if (metatype_check<ffi::METATYPE_FLAG_NAME>(L, 1)) {
            if (lua_type(L, -1) == LUA_TSTRING) {
                lua_pushfstring(L, ": %p", cd.address_of());
                lua_concat(L, 2);
                return 1;
            }
            lua_pop(L, 1);
        }
#endif
        auto const *tp = &cd.decl;
        void *val = cd.as_deref_ptr();
        /* 64-bit integers */
        /* XXX: special printing for lua builds with non-double numbers? */
        if (tp->integer() && (tp->alloc_size() == 8)) {
            char buf[32];
            std::size_t written;
            if (tp->is_unsigned()) {
                written = util::write_u(buf, sizeof(buf), *static_cast<unsigned long long *>(val));
                std::memcpy(&buf[written], "ULL", 4);
                written += 4;
            } else {
                written = util::write_i(buf, sizeof(buf), *static_cast<long long *>(val));
                std::memcpy(&buf[written], "LL", 3);
                written += 3;
            }
            lua_pushlstring(L, buf, written);
            return 1;
        }
        lua_pushliteral(L, "cdata<");
        cd.decl.serialize(L);
        lua_pushfstring(L, ">: %p", cd.address_of());
        lua_concat(L, 3);
        return 1;
    }

    static int call(lua_State *L) {
        auto &fd = ffi::tocdata(L, 1);
        if (ffi::isctype(fd)) {
            if (metatype_check<ffi::METATYPE_FLAG_NEW>(L, 1)) {
                int nargs = lua_gettop(L) - 1;
                lua_insert(L, 1);
                lua_call(L, nargs, 1);
            } else {
                ffi::make_cdata(L, fd.decl, ffi::RULE_CONV, 2);
            }
            return 1;
        }
        if (!fd.decl.callable()) {
            int nargs = lua_gettop(L);
            if (metatype_check<ffi::METATYPE_FLAG_CALL>(L, 1)) {
                lua_insert(L, 1);
                lua_call(L, nargs, LUA_MULTRET);
                return lua_gettop(L);
            }
            fd.decl.serialize(L);
            luaL_error(L, "'%s' is not callable", lua_tostring(L, -1));
        }
        if (fd.decl.closure() && !fd.as<ffi::fdata>().cd) {
            luaL_error(L, "bad callback");
        }
        return ffi::call_cif(fd, L, lua_gettop(L) - 1);
    }

    template <bool New, typename F>
    static bool index_common(lua_State *L, F &&func) {
        auto &cd = ffi::tocdata(L, 1);
        if (ffi::isctype(cd)) {
            if (New) {
                luaL_error(L, "'ctype' is not indexable");
            } else {
                /* indexing ctypes is okay if they have __index */
                return false;
            }
        }
        void **valp = static_cast<void **>(cd.as_deref_ptr());
        auto const *decl = &cd.decl;
        if ((decl->type() == ast::C_BUILTIN_PTR) && (lua_type(L, 2) == LUA_TSTRING)) {
            /* pointers are indexable like arrays using numbers, but
             * not by names; however, there's a special case for record
             * types, where pointers to them can be indexed like the
             * underlying record, so assume that for the time being
             */
            decl = &decl->ptr_base();
            valp = static_cast<void **>(*valp);
        }
        std::size_t elsize = 0;
        unsigned char *ptr = nullptr;
        switch (decl->type()) {
            case ast::C_BUILTIN_PTR:
            case ast::C_BUILTIN_ARRAY:
                ptr = static_cast<unsigned char *>(*valp);
                elsize = decl->ptr_base().alloc_size();
                if (!elsize) {
                    decl->serialize(L);
                    luaL_error(L, "attempt to index an incomplete type '%s'", lua_tostring(L, -1));
                }
                break;
            case ast::C_BUILTIN_RECORD: {
                char const *fname = luaL_checkstring(L, 2);
                ast::c_type const *outf;
                auto foff = decl->record().field_offset(fname, outf);
                if (foff < 0) {
                    return false;
                }
                func(*outf, util::pun<unsigned char *>(valp) + foff);
                return true;
            }
            default: {
                decl->serialize(L);
                luaL_error(L, "'%s' is not indexable", lua_tostring(L, -1));
                break;
            }
        }
        auto sidx = ffi::check_arith<std::size_t>(L, 2);
        func(decl->ptr_base(), static_cast<void *>(&ptr[sidx * elsize]));
        return true;
    }

    static int cb_free(lua_State *L) {
        auto &cd = ffi::checkcdata(L, 1);
        luaL_argcheck(L, cd.decl.closure(), 1, "not a callback");
        if (!cd.as<ffi::fdata>().cd) {
            luaL_error(L, "bad callback");
        }
        ffi::destroy_closure(L, cd.as<ffi::fdata>().cd);
        return 0;
    }

    static int cb_set(lua_State *L) {
        auto &cd = ffi::checkcdata(L, 1);
        luaL_argcheck(L, cd.decl.closure(), 1, "not a callback");
        if (!cd.as<ffi::fdata>().cd) {
            luaL_error(L, "bad callback");
        }
        if (!lua_isfunction(L, 2)) {
            lua::type_error(L, 2, "function");
        }
        luaL_unref(L, LUA_REGISTRYINDEX, cd.as<ffi::fdata>().cd->fref);
        lua_pushvalue(L, 2);
        cd.as<ffi::fdata>().cd->fref = luaL_ref(L, LUA_REGISTRYINDEX);
        return 0;
    }

    static int index(lua_State *L) {
        auto &cd = ffi::tocdata(L, 1);
        if (cd.decl.closure()) {
            /* callbacks have some methods */
            char const *mname = lua_tostring(L, 2);
            /* if we had more methods, we'd do a table */
            if (!std::strcmp(mname, "free")) {
                lua_pushcfunction(L, cb_free);
                return 1;
            } else if (!std::strcmp(mname, "set")) {
                lua_pushcfunction(L, cb_set);
                return 1;
            } else if (!mname) {
                cd.decl.serialize(L);
                luaL_error(L, "'%s' cannot be indexed with '%s'", lua_tostring(L, -1), lua_typename(L, lua_type(L, 2)));
            } else {
                cd.decl.serialize(L);
                luaL_error(L, "'%s' has no member named '%s'", lua_tostring(L, -1), mname);
            }
            return 0;
        }
        if (index_common<false>(L, [L](auto &decl, void *val) {
                if (!ffi::to_lua(L, decl, val, ffi::RULE_CONV, false)) {
                    luaL_error(L, "invalid C type");
                }
            })) {
            return 1;
        };
        if (metatype_check<ffi::METATYPE_FLAG_INDEX>(L, 1)) {
            /* if __index is a function, call it */
            if (lua_isfunction(L, -1)) {
                /* __index takes 2 args, put it to the beginning and call */
                lua_insert(L, 1);
                lua_call(L, 2, 1);
                return 1;
            }
            /* otherwise, index it with key that's on top of the stack */
            lua_pushvalue(L, 2);
            lua_gettable(L, -2);
            if (!lua_isnil(L, -1)) {
                return 1;
            }
        }
        if (ffi::isctype(cd)) {
            luaL_error(L, "'ctype' is not indexable");
        }
        if (lua_type(L, 2) != LUA_TSTRING) {
            cd.decl.serialize(L);
            luaL_error(L, "'%s' is not indexable with '%s'", lua_tostring(L, -1), lua_typename(L, 2));
        } else {
            cd.decl.serialize(L);
            luaL_error(L, "'%s' has no member named '%s'", lua_tostring(L, -1), lua_tostring(L, 2));
        }
        return 1;
    }

    static int newindex(lua_State *L) {
        if (index_common<true>(L, [L](auto &decl, void *val) { ffi::from_lua(L, decl, val, 3); })) {
            return 0;
        };
        if (metatype_check<ffi::METATYPE_FLAG_NEWINDEX>(L, 1)) {
            lua_insert(L, 1);
            lua_call(L, 3, 0);
            return 0;
        }
        ffi::tocdata(L, 1).decl.serialize(L);
        luaL_error(L, "'%s' has no member named '%s'", lua_tostring(L, -1), lua_tostring(L, 2));
        return 0;
    }

    template <ffi::metatype_flag mtype>
    static inline bool op_try_mt(lua_State *L, ffi::cdata const *cd1, ffi::cdata const *cd2, int rvals = 1) {
        /* custom metatypes, either operand */
        if ((cd1 && metatype_check<mtype>(L, 1)) || (cd2 && metatype_check<mtype>(L, 2))) {
            lua_insert(L, 1);
            lua_call(L, lua_gettop(L) - 1, rvals);
            return true;
        }
        return false;
    }

    static int concat(lua_State *L) {
        auto *cd1 = ffi::testcdata(L, 1);
        auto *cd2 = ffi::testcdata(L, 2);
        if (op_try_mt<ffi::METATYPE_FLAG_CONCAT>(L, cd1, cd2)) {
            return 1;
        }
        luaL_error(L, "attempt to concatenate '%s' and '%s'", ffi::lua_serialize(L, 1), ffi::lua_serialize(L, 2));
        return 0;
    }

    static int len(lua_State *L) {
        auto *cd = ffi::testcdata(L, 1);
        if (op_try_mt<ffi::METATYPE_FLAG_LEN>(L, cd, nullptr)) {
            return 1;
        }
        luaL_error(L, "attempt to get length of '%s'", ffi::lua_serialize(L, 1));
        return 0;
    }

    /* this follows LuaJIT rules for cdata arithmetic: each operand is
     * converted to signed 64-bit integer unless one of them is an
     * unsigned 64-bit integer, in which case both become unsigned
     */
    template <typename T, ast::c_expr_type et>
    static void promote_to_64bit(ast::c_expr_type &t, void *v) {
        switch (t) {
            case ast::c_expr_type::INT:
                *static_cast<T *>(v) = T(*static_cast<int *>(v));
                break;
            case ast::c_expr_type::UINT:
                *static_cast<T *>(v) = T(*static_cast<unsigned int *>(v));
                break;
            case ast::c_expr_type::LONG:
                *static_cast<T *>(v) = T(*static_cast<long *>(v));
                break;
            case ast::c_expr_type::ULONG:
                *static_cast<T *>(v) = T(*static_cast<unsigned long *>(v));
                break;
            case ast::c_expr_type::LLONG:
                *static_cast<T *>(v) = T(*static_cast<long long *>(v));
                break;
            case ast::c_expr_type::FLOAT:
                *static_cast<T *>(v) = T(*static_cast<float *>(v));
                break;
            case ast::c_expr_type::DOUBLE:
                *static_cast<T *>(v) = T(*static_cast<double *>(v));
                break;
            case ast::c_expr_type::LDOUBLE:
                *static_cast<T *>(v) = T(*static_cast<long double *>(v));
                break;
            default:
                break;
        }
        t = et;
    }

    static void promote_long(ast::c_expr_type &t) {
        if (sizeof(long) == sizeof(long long)) {
            switch (t) {
                case ast::c_expr_type::LONG:
                    t = ast::c_expr_type::LLONG;
                    break;
                case ast::c_expr_type::ULONG:
                    t = ast::c_expr_type::ULLONG;
                    break;
                default:
                    break;
            }
        }
    }

    static void promote_sides(ast::c_expr_type &lt, ast::c_value &lv, ast::c_expr_type &rt, ast::c_value &rv) {
        promote_long(lt);
        promote_long(rt);
        if ((lt == ast::c_expr_type::ULLONG) || (rt == ast::c_expr_type::ULLONG)) {
            promote_to_64bit<unsigned long long, ast::c_expr_type::ULLONG>(lt, &lv);
            promote_to_64bit<unsigned long long, ast::c_expr_type::ULLONG>(rt, &rv);
        } else {
            promote_to_64bit<long long, ast::c_expr_type::LLONG>(lt, &lv);
            promote_to_64bit<long long, ast::c_expr_type::LLONG>(rt, &rv);
        }
    }

    static ast::c_value arith_64bit_base(lua_State *L, ast::c_expr_binop op, ast::c_expr_type &retp) {
        ast::c_expr bexp{ast::C_TYPE_WEAK}, lhs, rhs;
        ast::c_expr_type lt = ffi::check_arith_expr(L, 1, lhs.val);
        ast::c_expr_type rt = ffi::check_arith_expr(L, 2, rhs.val);
        promote_sides(lt, lhs.val, rt, rhs.val);
        lhs.type(lt);
        rhs.type(rt);
        bexp.type(ast::c_expr_type::BINARY);
        bexp.bin.op = op;
        bexp.bin.lhs = &lhs;
        bexp.bin.rhs = &rhs;
        ast::c_value ret;
        if (!bexp.eval(L, ret, retp, true)) {
            lua_error(L);
        }
        return ret;
    }

    static void arith_64bit_bin(lua_State *L, ast::c_expr_binop op) {
        /* regular arithmetic */
        ast::c_expr_type retp;
        auto rv = arith_64bit_base(L, op, retp);
        ffi::make_cdata_arith(L, retp, rv);
    }

    static void arith_64bit_cmp(lua_State *L, ast::c_expr_binop op) {
        /* comparison */
        ast::c_expr_type retp;
        auto rv = arith_64bit_base(L, op, retp);
        assert(retp == ast::c_expr_type::BOOL);
        lua_pushboolean(L, rv.b);
    }

    static int add(lua_State *L) {
        auto *cd1 = ffi::testcdata(L, 1);
        auto *cd2 = ffi::testcdata(L, 2);
        /* pointer arithmetic */
        if (cd1 && cd1->decl.ptr_like()) {
            auto asize = cd1->decl.ptr_base().alloc_size();
            if (!asize) {
                if (op_try_mt<ffi::METATYPE_FLAG_ADD>(L, cd1, cd2)) {
                    return 1;
                }
                luaL_error(L, "unknown C type size");
            }
            std::ptrdiff_t d;
            if (!ffi::test_arith<std::ptrdiff_t>(L, 2, d)) {
                if (op_try_mt<ffi::METATYPE_FLAG_ADD>(L, cd1, cd2)) {
                    return 1;
                }
                ffi::check_arith<std::ptrdiff_t>(L, 2);
            }
            /* do arithmetic on uintptr, doing it with a pointer would be UB
             * in case of a null pointer (and we want predicable behavior)
             */
            auto p = cd1->as_deref<std::uintptr_t>();
            auto tp = cd1->decl.as_type(ast::C_BUILTIN_PTR);
            auto &ret = ffi::newcdata(L, tp.unref(), sizeof(void *));
            ret.as<std::uintptr_t>() = p + d * asize;
            return 1;
        } else if (cd2 && cd2->decl.ptr_like()) {
            auto asize = cd2->decl.ptr_base().alloc_size();
            if (!asize) {
                if (op_try_mt<ffi::METATYPE_FLAG_ADD>(L, cd1, cd2)) {
                    return 1;
                }
                luaL_error(L, "unknown C type size");
            }
            std::ptrdiff_t d;
            if (!ffi::test_arith<std::ptrdiff_t>(L, 1, d)) {
                if (op_try_mt<ffi::METATYPE_FLAG_ADD>(L, cd1, cd2)) {
                    return 1;
                }
                ffi::check_arith<std::ptrdiff_t>(L, 1);
            }
            auto p = cd2->as_deref<std::uintptr_t>();
            auto tp = cd2->decl.as_type(ast::C_BUILTIN_PTR);
            auto &ret = ffi::newcdata(L, tp.unref(), sizeof(void *));
            ret.as<std::uintptr_t>() = d * asize + p;
            return 1;
        }
        if (op_try_mt<ffi::METATYPE_FLAG_ADD>(L, cd1, cd2)) {
            return 1;
        }
        arith_64bit_bin(L, ast::c_expr_binop::ADD);
        return 1;
    }

    static int sub(lua_State *L) {
        auto *cd1 = ffi::testcdata(L, 1);
        auto *cd2 = ffi::testcdata(L, 2);
        /* pointer difference */
        if (cd1 && cd1->decl.ptr_like()) {
            auto asize = cd1->decl.ptr_base().alloc_size();
            if (!asize) {
                if (op_try_mt<ffi::METATYPE_FLAG_SUB>(L, cd1, cd2)) {
                    return 1;
                }
                luaL_error(L, "unknown C type size");
            }
            if (cd2 && cd2->decl.ptr_like()) {
                if (!cd1->decl.ptr_base().is_same(cd2->decl.ptr_base(), true)) {
                    if (op_try_mt<ffi::METATYPE_FLAG_SUB>(L, cd1, cd2)) {
                        return 1;
                    }
                    cd2->decl.serialize(L);
                    cd1->decl.serialize(L);
                    luaL_error(L, "cannot convert '%s' to '%s'", lua_tostring(L, -2), lua_tostring(L, -1));
                }
                /* use intptrs to prevent UB with potential nulls; signed so
                 * we can get a potential negative result in a safe way
                 */
                auto ret = cd1->as_deref<std::intptr_t>() - cd2->as_deref<std::intptr_t>();
                lua_pushinteger(L, lua_Integer(ret / asize));
                return 1;
            }
            std::ptrdiff_t d;
            if (!ffi::test_arith<std::ptrdiff_t>(L, 2, d)) {
                if (op_try_mt<ffi::METATYPE_FLAG_ADD>(L, cd1, cd2)) {
                    return 1;
                }
                ffi::check_arith<std::ptrdiff_t>(L, 2);
            }
            auto p = cd1->as_deref<std::uintptr_t>();
            auto &ret = ffi::newcdata(L, cd1->decl, sizeof(void *));
            ret.as<std::uintptr_t>() = p + d;
            return 1;
        }
        if (op_try_mt<ffi::METATYPE_FLAG_SUB>(L, cd1, cd2)) {
            return 1;
        }
        arith_64bit_bin(L, ast::c_expr_binop::SUB);
        return 1;
    }

    template <ffi::metatype_flag mflag, ast::c_expr_binop bop>
    static int arith_bin(lua_State *L) {
        auto *cd1 = ffi::testcdata(L, 1);
        auto *cd2 = ffi::testcdata(L, 2);
        if (!op_try_mt<mflag>(L, cd1, cd2)) {
            arith_64bit_bin(L, bop);
        }
        return 1;
    }

    template <typename T>
    static T powimp(T base, T exp) {
        if (util::is_signed<T>::value && (exp < 0)) {
            return 0;
        }
        T ret = 1;
        for (;;) {
            if (exp & 1) {
                ret *= base;
            }
            exp = exp >> 1;
            if (!exp) {
                break;
            }
            base *= base;
        }
        return ret;
    }

    static int pow(lua_State *L) {
        auto *cd1 = ffi::testcdata(L, 1);
        auto *cd2 = ffi::testcdata(L, 2);
        if (op_try_mt<ffi::METATYPE_FLAG_POW>(L, cd1, cd2)) {
            return 1;
        }
        ast::c_value lhs, rhs;
        ast::c_expr_type lt = ffi::check_arith_expr(L, 1, lhs);
        ast::c_expr_type rt = ffi::check_arith_expr(L, 2, rhs);
        promote_sides(lt, lhs, rt, rhs);
        assert(lt == rt);
        switch (lt) {
            case ast::c_expr_type::LLONG:
                lhs.ll = powimp<long long>(lhs.ll, rhs.ll);
                break;
            case ast::c_expr_type::ULLONG:
                lhs.ull = powimp<unsigned long long>(lhs.ull, rhs.ull);
                break;
            default:
                assert(false);
                break;
        }
        ffi::make_cdata_arith(L, lt, lhs);
        return 1;
    }

    template <ffi::metatype_flag mflag, ast::c_expr_unop uop>
    static int arith_un(lua_State *L) {
        auto *cd = ffi::testcdata(L, 1);
        if (op_try_mt<mflag>(L, cd, nullptr)) {
            return 1;
        }
        ast::c_expr uexp{ast::C_TYPE_WEAK}, exp;
        ast::c_expr_type et = ffi::check_arith_expr(L, 1, exp.val);
        promote_long(et);
        if (et != ast::c_expr_type::ULLONG) {
            promote_to_64bit<long long, ast::c_expr_type::LLONG>(et, &exp.val);
        }
        exp.type(et);
        uexp.type(ast::c_expr_type::UNARY);
        uexp.un.op = uop;
        uexp.un.expr = &exp;
        ast::c_value rv;
        if (!uexp.eval(L, rv, et, true)) {
            lua_error(L);
        }
        ffi::make_cdata_arith(L, et, rv);
        return 1;
    }

    static void *cmp_addr(ffi::cdata *cd) {
        if (cd->decl.ptr_like()) {
            return cd->as_deref<void *>();
        }
        return cd->as_deref_ptr();
    }

    static int eq(lua_State *L) {
        auto *cd1 = ffi::testcval(L, 1);
        auto *cd2 = ffi::testcval(L, 2);
        if (!cd1 || !cd2) {
            /* equality against non-cdata object is always false */
            lua_pushboolean(L, false);
            return 1;
        }
        if ((cd1->gc_ref == lua::CFFI_CTYPE_TAG) || (cd2->gc_ref == lua::CFFI_CTYPE_TAG)) {
            if (cd1->gc_ref != cd2->gc_ref) {
                /* ctype against cdata */
                lua_pushboolean(L, false);
            } else {
                lua_pushboolean(L, cd1->decl.is_same(cd2->decl));
            }
            return 1;
        }
        if (!cd1->decl.arith() || !cd2->decl.arith()) {
            if (cd1->decl.ptr_like() && cd2->decl.ptr_like()) {
                lua_pushboolean(L, cd1->as_deref<void *>() == cd2->as_deref<void *>());
                return 1;
            }
            if (op_try_mt<ffi::METATYPE_FLAG_EQ>(L, cd1, cd2)) {
                return 1;
            }
            /* if any operand is non-arithmetic, compare by address */
            lua_pushboolean(L, cmp_addr(cd1) == cmp_addr(cd2));
            return 1;
        }
        if (op_try_mt<ffi::METATYPE_FLAG_EQ>(L, cd1, cd2)) {
            return 1;
        }
        /* otherwise compare values */
        arith_64bit_cmp(L, ast::c_expr_binop::EQ);
        return 1;
    }

    template <ffi::metatype_flag mf1, ffi::metatype_flag mf2>
    static bool cmp_base(lua_State *L, ast::c_expr_binop op, ffi::cdata const *cd1, ffi::cdata const *cd2) {
        if (!cd1 || !cd2) {
            auto *ccd = (cd1 ? cd1 : cd2);
            if (!ccd->decl.arith() || !lua_isnumber(L, 2 - !cd1)) {
                if (op_try_mt<mf1>(L, cd1, cd2)) {
                    return true;
                } else if ((mf2 != mf1) && op_try_mt<mf2>(L, cd2, cd1)) {
                    lua_pushboolean(L, !lua_toboolean(L, -1));
                    return true;
                }
                luaL_error(L, "attempt to compare '%s' with '%s'", ffi::lua_serialize(L, 1), ffi::lua_serialize(L, 2));
            }
            arith_64bit_cmp(L, op);
            return true;
        }
        if (cd1->decl.arith() && cd2->decl.arith()) {
            /* compare values if both are arithmetic types */
            arith_64bit_cmp(L, op);
            return true;
        }
        /* compare only compatible pointers */
        if (((cd1->decl.type() != ast::C_BUILTIN_PTR) || (cd2->decl.type() != ast::C_BUILTIN_PTR)) || (!cd1->decl.ptr_base().is_same(cd2->decl.ptr_base(), true))) {
            if (op_try_mt<mf1>(L, cd1, cd2)) {
                return true;
            } else if ((mf2 != mf1) && op_try_mt<mf2>(L, cd2, cd1)) {
                lua_pushboolean(L, !lua_toboolean(L, -1));
                return true;
            }
            luaL_error(L, "attempt to compare '%s' with '%s'", ffi::lua_serialize(L, 1), ffi::lua_serialize(L, 2));
        }
        if (op_try_mt<mf1>(L, cd1, cd2)) {
            return true;
        } else if ((mf2 != mf1) && op_try_mt<mf2>(L, cd2, cd1)) {
            lua_pushboolean(L, !lua_toboolean(L, -1));
            return true;
        }
        return false;
    }

    static int lt(lua_State *L) {
        auto *cd1 = ffi::testcdata(L, 1);
        auto *cd2 = ffi::testcdata(L, 2);
        if (cmp_base<ffi::METATYPE_FLAG_LT, ffi::METATYPE_FLAG_LT>(L, ast::c_expr_binop::LT, cd1, cd2)) {
            return 1;
        }
        lua_pushboolean(L, cmp_addr(cd1) < cmp_addr(cd2));
        return 1;
    }

    static int le(lua_State *L) {
        auto *cd1 = ffi::testcdata(L, 1);
        auto *cd2 = ffi::testcdata(L, 2);
        /* tries both (a <= b) and not (b < a), like lua */
        if (cmp_base<ffi::METATYPE_FLAG_LE, ffi::METATYPE_FLAG_LT>(L, ast::c_expr_binop::LE, cd1, cd2)) {
            return 1;
        }
        lua_pushboolean(L, cmp_addr(cd1) <= cmp_addr(cd2));
        return 1;
    }

#if LUA_VERSION_NUM > 501
    static int pairs(lua_State *L) {
        auto *cd = ffi::testcdata(L, 1);
        if (op_try_mt<ffi::METATYPE_FLAG_PAIRS>(L, cd, nullptr, 3)) {
            return 3;
        }
        luaL_error(L, "attempt to iterate '%s'", ffi::lua_serialize(L, 1));
        return 0;
    }

#if LUA_VERSION_NUM == 502
    static int ipairs(lua_State *L) {
        auto *cd = ffi::testcdata(L, 1);
        if (op_try_mt<ffi::METATYPE_FLAG_IPAIRS>(L, cd, nullptr, 3)) {
            return 3;
        }
        luaL_error(L, "attempt to iterate '%s'", ffi::lua_serialize(L, 1));
        return 0;
    }
#endif

#if LUA_VERSION_NUM > 502
    template <ffi::metatype_flag mflag, ast::c_expr_binop bop>
    static int shift_bin(lua_State *L) {
        auto *cd1 = ffi::testcdata(L, 1);
        auto *cd2 = ffi::testcdata(L, 2);
        if (op_try_mt<mflag>(L, cd1, cd2)) {
            return 1;
        }
        ast::c_expr_type retp;
        ast::c_expr bexp{ast::C_TYPE_WEAK}, lhs, rhs;
        ast::c_expr_type lt = ffi::check_arith_expr(L, 1, lhs.val);
        ast::c_expr_type rt = ffi::check_arith_expr(L, 2, rhs.val);
        /* we're only promoting the left side in shifts */
        promote_long(lt);
        if (lt != ast::c_expr_type::ULLONG) {
            promote_to_64bit<long long, ast::c_expr_type::LLONG>(lt, &lhs.val);
        }
        lhs.type(lt);
        rhs.type(rt);
        bexp.type(ast::c_expr_type::BINARY);
        bexp.bin.op = bop;
        bexp.bin.lhs = &lhs;
        bexp.bin.rhs = &rhs;
        ast::c_value rv;
        if (!bexp.eval(L, rv, retp, true)) {
            lua_error(L);
        }
        ffi::make_cdata_arith(L, retp, rv);
        return 1;
    }

#if LUA_VERSION_NUM > 503
    static int close(lua_State *L) {
        auto *cd = ffi::testcdata(L, 1);
        if (cd && metatype_check<ffi::METATYPE_FLAG_CLOSE>(L, 1)) {
            lua_insert(L, 1);
            lua_call(L, 2, 0);
        }
        return 0;
    }
#endif /* LUA_VERSION_NUM > 503 */
#endif /* LUA_VERSION_NUM > 502 */
#endif /* LUA_VERSION_NUM > 501 */

    static void setup(lua_State *L) {
        if (!luaL_newmetatable(L, lua::CFFI_CDATA_MT)) {
            luaL_error(L, "unexpected error: registry reinitialized");
        }

        lua_pushliteral(L, "ffi");
        lua_setfield(L, -2, "__metatable");

        /* this will store registered permanent struct/union metatypes
         *
         * it's used instead of regular lua registry because there is no
         * way to reasonably garbage collect these references, and they die
         * with the rest of the ffi anyway, so...
         */
        lua_newtable(L);
        lua_setfield(L, -2, "__ffi_metatypes");

        lua_pushcfunction(L, tostring);
        lua_setfield(L, -2, "__tostring");

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");

        lua_pushcfunction(L, call);
        lua_setfield(L, -2, "__call");

        lua_pushcfunction(L, index);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, newindex);
        lua_setfield(L, -2, "__newindex");

        lua_pushcfunction(L, concat);
        lua_setfield(L, -2, "__concat");

        lua_pushcfunction(L, len);
        lua_setfield(L, -2, "__len");

        lua_pushcfunction(L, add);
        lua_setfield(L, -2, "__add");

        lua_pushcfunction(L, sub);
        lua_setfield(L, -2, "__sub");

        lua_pushcfunction(L, (arith_bin<ffi::METATYPE_FLAG_MUL, ast::c_expr_binop::MUL>));
        lua_setfield(L, -2, "__mul");

        lua_pushcfunction(L, (arith_bin<ffi::METATYPE_FLAG_DIV, ast::c_expr_binop::DIV>));
        lua_setfield(L, -2, "__div");

        lua_pushcfunction(L, (arith_bin<ffi::METATYPE_FLAG_MOD, ast::c_expr_binop::MOD>));
        lua_setfield(L, -2, "__mod");

        lua_pushcfunction(L, pow);
        lua_setfield(L, -2, "__pow");

        lua_pushcfunction(L, (arith_un<ffi::METATYPE_FLAG_UNM, ast::c_expr_unop::UNM>));
        lua_setfield(L, -2, "__unm");

        lua_pushcfunction(L, eq);
        lua_setfield(L, -2, "__eq");

        lua_pushcfunction(L, lt);
        lua_setfield(L, -2, "__lt");

        lua_pushcfunction(L, le);
        lua_setfield(L, -2, "__le");

#if LUA_VERSION_NUM > 501
        lua_pushcfunction(L, pairs);
        lua_setfield(L, -2, "__pairs");

#if LUA_VERSION_NUM == 502
        lua_pushcfunction(L, ipairs);
        lua_setfield(L, -2, "__ipairs");
#endif

#if LUA_VERSION_NUM > 502
        lua_pushcfunction(L, (arith_bin<ffi::METATYPE_FLAG_IDIV, ast::c_expr_binop::DIV>));
        lua_setfield(L, -2, "__idiv");

        lua_pushcfunction(L, (arith_bin<ffi::METATYPE_FLAG_BAND, ast::c_expr_binop::BAND>));
        lua_setfield(L, -2, "__band");

        lua_pushcfunction(L, (arith_bin<ffi::METATYPE_FLAG_BOR, ast::c_expr_binop::BOR>));
        lua_setfield(L, -2, "__bor");

        lua_pushcfunction(L, (arith_bin<ffi::METATYPE_FLAG_BXOR, ast::c_expr_binop::BXOR>));
        lua_setfield(L, -2, "__bxor");

        lua_pushcfunction(L, (arith_un<ffi::METATYPE_FLAG_BNOT, ast::c_expr_unop::BNOT>));
        lua_setfield(L, -2, "__bnot");

        lua_pushcfunction(L, (shift_bin<ffi::METATYPE_FLAG_SHL, ast::c_expr_binop::LSH>));
        lua_setfield(L, -2, "__shl");

        lua_pushcfunction(L, (shift_bin<ffi::METATYPE_FLAG_SHR, ast::c_expr_binop::RSH>));
        lua_setfield(L, -2, "__shr");

#if LUA_VERSION_NUM > 503
        lua_pushcfunction(L, close);
        lua_setfield(L, -2, "__close");
#endif /* LUA_VERSION_NUM > 503 */
#endif /* LUA_VERSION_NUM > 502 */
#endif /* LUA_VERSION_NUM > 501 */

        lua_pop(L, 1);
    }
};

/* the ffi module itself */
struct ffi_module {
    static int cdef_f(lua_State *L) {
        std::size_t slen;
        char const *inp = luaL_checklstring(L, 1, &slen);
        parser::parse(L, inp, inp + slen, (lua_gettop(L) > 1) ? 2 : -1);
        return 0;
    }

    /* either gets a ctype or makes a ctype from a string */
    static ast::c_type const &check_ct(lua_State *L, int idx, int paridx = -1) {
        if (ffi::iscval(L, idx)) {
            auto &cd = ffi::tocdata(L, idx);
            if (ffi::isctype(cd)) {
                return cd.decl;
            }
            auto &ct = ffi::newctype(L, cd.decl.copy());
            lua_replace(L, idx);
            return ct.decl;
        }
        std::size_t slen;
        char const *inp = luaL_checklstring(L, idx, &slen);
        auto &ct = ffi::newctype(L, parser::parse_type(L, inp, inp + slen, paridx));
        lua_replace(L, idx);
        return ct.decl;
    }

    static int new_f(lua_State *L) {
        ffi::make_cdata(L, check_ct(L, 1), ffi::RULE_CONV, 2);
        return 1;
    }

    static int cast_f(lua_State *L) {
        luaL_checkany(L, 2);
        ffi::make_cdata(L, check_ct(L, 1), ffi::RULE_CAST, 2);
        return 1;
    }

    static int metatype_f(lua_State *L) {
        auto &ct = check_ct(L, 1);
        luaL_argcheck(L, ct.type() == ast::C_BUILTIN_RECORD, 1, "invalid C type");
        int mflags;
        if (ct.record().metatype(mflags) != LUA_REFNIL) {
            luaL_error(L, "cannot change a protected metatable");
        }
        luaL_checktype(L, 2, LUA_TTABLE);

#define FIELD_CHECK(fname, flagn)                 \
    {                                             \
        lua_getfield(L, 2, "__" fname);           \
        if (!lua_isnil(L, -1)) {                  \
            mflags |= ffi::METATYPE_FLAG_##flagn; \
        }                                         \
        lua_pop(L, 1);                            \
    }

        FIELD_CHECK("add", ADD)
        FIELD_CHECK("sub", SUB)
        FIELD_CHECK("mul", MUL)
        FIELD_CHECK("div", DIV)
        FIELD_CHECK("mod", MOD)
        FIELD_CHECK("pow", POW)
        FIELD_CHECK("unm", UNM)
        FIELD_CHECK("concat", CONCAT)
        FIELD_CHECK("len", LEN)
        FIELD_CHECK("eq", EQ)
        FIELD_CHECK("lt", LT)
        FIELD_CHECK("le", LE)
        FIELD_CHECK("index", INDEX)
        FIELD_CHECK("newindex", NEWINDEX)
        FIELD_CHECK("call", CALL)
        FIELD_CHECK("new", NEW)
        FIELD_CHECK("gc", GC)
        FIELD_CHECK("tostring", TOSTRING)

#if LUA_VERSION_NUM > 501
        FIELD_CHECK("pairs", PAIRS)

#if LUA_VERSION_NUM == 502
        FIELD_CHECK("ipairs", IPAIRS)
#endif

#if LUA_VERSION_NUM > 502
        FIELD_CHECK("idiv", IDIV)
        FIELD_CHECK("band", BAND)
        FIELD_CHECK("bor", BOR)
        FIELD_CHECK("bxor", BXOR)
        FIELD_CHECK("bnot", BNOT)
        FIELD_CHECK("shl", SHL)
        FIELD_CHECK("shr", SHR)

        FIELD_CHECK("name", NAME)
#if LUA_VERSION_NUM > 503
        FIELD_CHECK("close", CLOSE)
#endif /* LUA_VERSION_NUM > 503 */
#endif /* LUA_VERSION_NUM > 502 */
#endif /* LUA_VERSION_NUM > 501 */

#undef FIELD_CHECK

        /* get the metatypes table on the stack */
        luaL_getmetatable(L, lua::CFFI_CDATA_MT);
        lua_getfield(L, -1, "__ffi_metatypes");
        /* the metatype */
        lua_pushvalue(L, 2);
        const_cast<ast::c_record &>(ct.record()).metatype(luaL_ref(L, -2), mflags);

        lua_pushvalue(L, 1);
        return 1; /* return the ctype */
    }

    static int load_f(lua_State *L) {
        char const *path = luaL_checkstring(L, 1);
        bool glob = (lua_gettop(L) >= 2) && lua_toboolean(L, 2);
        auto *c_ud = static_cast<lib::c_lib *>(lua_newuserdata(L, sizeof(lib::c_lib)));
        new (c_ud) lib::c_lib{};
        lib::load(c_ud, path, L, glob);
        return 1;
    }

    static int typeof_f(lua_State *L) {
        check_ct(L, 1, (lua_gettop(L) > 1) ? 2 : -1);
        /* make sure the type we've checked out is the result,
         * and not the last argument it's parameterized with
         */
        lua_pushvalue(L, 1);
        return 1;
    }

    static int addressof_f(lua_State *L) {
        auto &cd = ffi::checkcdata(L, 1);
        ffi::newcdata(L, ast::c_type{util::make_rc<ast::c_type>(util::move(cd.decl.unref())), 0, ast::C_BUILTIN_PTR}, sizeof(void *)).as<void *>() = cd.address_of();
        return 1;
    }

    static int gc_f(lua_State *L) {
        auto &cd = ffi::checkcdata(L, 1);
        if (lua_isnil(L, 2)) {
            /* if nil and there is an existing finalizer, unset */
            if (cd.gc_ref != LUA_REFNIL) {
                luaL_unref(L, LUA_REGISTRYINDEX, cd.gc_ref);
                cd.gc_ref = LUA_REFNIL;
            }
        } else {
            /* new finalizer can be any type, it's pcall'd */
            lua_pushvalue(L, 2);
            cd.gc_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }
        lua_pushvalue(L, 1); /* return the cdata */
        return 1;
    }

    static int sizeof_f(lua_State *L) {
        if (ffi::iscdata(L, 1)) {
            lua_pushinteger(L, ffi::cdata_value_size(L, 1));
            return 1;
        }
        auto get_vlasz = [L](std::size_t &sz, bool vla) -> bool {
            if (lua_isinteger(L, 2)) {
                auto isz = lua_tointeger(L, 2);
                if (isz < 0) {
                    return false;
                }
                sz = std::size_t(isz);
            } else if (lua_isnumber(L, 2)) {
                auto isz = lua_tonumber(L, 2);
                if (isz < 0) {
                    return false;
                }
                sz = std::size_t(isz);
            } else if (ffi::iscdata(L, 2)) {
                auto &cd = ffi::tocdata(L, 2);
                if (!cd.decl.integer()) {
                    luaL_checkinteger(L, 2);
                }
                if (cd.decl.is_unsigned()) {
                    sz = ffi::check_arith<std::size_t>(L, 2);
                } else {
                    auto isz = ffi::check_arith<long long>(L, 2);
                    if (isz < 0) {
                        return false;
                    }
                    sz = std::size_t(isz);
                }
            } else if (vla) {
                luaL_checkinteger(L, 2); /* this will longjmp */
                return false;
            } else {
                sz = 0;
            }
            return true;
        };
        auto &ct = check_ct(L, 1);
        if (ct.vla()) {
            std::size_t sz = 0;
            if (!get_vlasz(sz, true)) {
                return 0;
            }
            lua_pushinteger(L, ct.ptr_base().alloc_size() * sz);
            return 1;
        } else if (ct.flex()) {
            return 0;
        } else if (ct.type() == ast::C_BUILTIN_RECORD) {
            ast::c_type const *lf = nullptr;
            if (ct.record().flexible(&lf)) {
                std::size_t sz = 0;
                if (!get_vlasz(sz, lf->vla())) {
                    return 0;
                }
                lua_pushinteger(L, ct.alloc_size() + lf->ptr_base().alloc_size() * sz);
                return 1;
            }
        }
        lua_pushinteger(L, ct.alloc_size());
        return 1;
    }

    static int alignof_f(lua_State *L) {
        auto &ct = check_ct(L, 1);
        lua_pushinteger(L, ct.libffi_type()->alignment);
        return 1;
    }

    static int offsetof_f(lua_State *L) {
        auto &ct = check_ct(L, 1);
        char const *fname = luaL_checkstring(L, 2);
        if (ct.type() != ast::C_BUILTIN_RECORD) {
            return 0;
        }
        auto &cs = ct.record();
        if (cs.opaque()) {
            return 0;
        }
        ast::c_type const *tp;
        auto off = cs.field_offset(fname, tp);
        if (off >= 0) {
            lua_pushinteger(L, lua_Integer(off));
            return 1;
        }
        return 0;
    }

    static int istype_f(lua_State *L) {
        auto &ct = check_ct(L, 1);
        auto cd = ffi::testcval(L, 2);
        if (!cd) {
            lua_pushboolean(L, false);
            return 1;
        }
        if (ct.type() == ast::C_BUILTIN_RECORD) {
            /* if ct is a record, accept pointers to the struct */
            if (cd->decl.type() == ast::C_BUILTIN_PTR) {
                lua_pushboolean(L, ct.is_same(cd->decl.ptr_base(), true, true));
                return 1;
            }
        }
        lua_pushboolean(L, ct.is_same(cd->decl, true, true));
        return 1;
    }

    static int errno_f(lua_State *L) {
        int cur = errno;
        if (lua_gettop(L) >= 1) {
            errno = ffi::check_arith<int>(L, 1);
        }
        lua_pushinteger(L, cur);
        return 1;
    }

    static int string_f(lua_State *L) {
        if (!ffi::iscval(L, 1)) {
            if (lua_type(L, 1) == LUA_TSTRING) {
                /* allow passing through the string, but do not use
                 * lua_isstring as that allows numbers as well
                 */
                if (lua_gettop(L) <= 1) {
                    lua_pushvalue(L, 1);
                } else {
                    lua_pushlstring(L, lua_tostring(L, 1), ffi::check_arith<std::size_t>(L, 2));
                }
                return 1;
            }
            lua_pushfstring(L, "cannot convert '%s' to 'char const *'", luaL_typename(L, 1));
            luaL_argcheck(L, false, 1, lua_tostring(L, -1));
        }
        auto &ud = ffi::tocdata(L, 1);
        /* make sure we deal with cdata */
        if (ffi::isctype(ud)) {
            luaL_argcheck(L, false, 1, "cannot convert 'ctype' to 'char const *'");
        }
        /* handle potential ref case */
        void **valp = static_cast<void **>(ud.as_deref_ptr());
        if (lua_gettop(L) > 1) {
            /* if the length is given, special logic is used; any value can
             * be serialized here (addresses will be taken automatically)
             */
            auto slen = ffi::check_arith<std::size_t>(L, 2);
            switch (ud.decl.type()) {
                case ast::C_BUILTIN_PTR:
                case ast::C_BUILTIN_ARRAY:
                    lua_pushlstring(L, static_cast<char const *>(*valp), slen);
                    return 1;
                case ast::C_BUILTIN_RECORD:
                    lua_pushlstring(L, util::pun<char const *>(valp), slen);
                    return 1;
                default:
                    break;
            }
            goto converr;
        }
        /* if the length is not given, treat it like a string
         * the rules are still more loose here; arrays and pointers
         * are allowed, and their base type can be any kind of byte
         * signedness is not checked
         */
        if (!ud.decl.ptr_like()) {
            goto converr;
        }
        switch (ud.decl.ptr_base().type()) {
            case ast::C_BUILTIN_VOID:
            case ast::C_BUILTIN_CHAR:
            case ast::C_BUILTIN_SCHAR:
            case ast::C_BUILTIN_UCHAR:
                break;
            default:
                goto converr;
        }
        if (ud.decl.static_array()) {
            char const *strp = static_cast<char const *>(*valp);
            /* static arrays are special (no need for null termination) */
            auto slen = ud.decl.alloc_size();
            /* but if an embedded zero is found, terminate at that */
            auto *p = static_cast<char const *>(std::memchr(strp, '\0', slen));
            if (p) {
                slen = std::size_t(p - strp);
            }
            lua_pushlstring(L, strp, slen);
        } else {
            /* strings need to be null terminated */
            lua_pushstring(L, static_cast<char const *>(*valp));
        }
        return 1;
    converr:
        ud.decl.serialize(L);
        lua_pushfstring(L, "cannot convert '%s' to 'string'", lua_tostring(L, -1));
        luaL_argcheck(L, false, 1, lua_tostring(L, -1));
        return 1;
    }

    /* FIXME: type conversions (constness etc.) */
    static void *check_voidptr(lua_State *L, int idx) {
        if (ffi::iscval(L, idx)) {
            auto &cd = ffi::tocdata(L, idx);
            if (ffi::isctype(cd)) {
                luaL_argcheck(L, false, idx, "cannot convert 'ctype' to 'void *'");
            }
            if (cd.decl.ptr_like()) {
                return cd.as_deref<void *>();
            }
            if (cd.decl.is_ref()) {
                return cd.as_ptr();
            }
            cd.decl.serialize(L);
            lua_pushfstring(L, "cannot convert '%s' to 'void *'", lua_tostring(L, -1));
            goto argcheck;
        } else if (lua_isuserdata(L, idx)) {
            return lua_touserdata(L, idx);
        }
        lua_pushfstring(L, "cannot convert '%s' to 'void *'", luaL_typename(L, 1));
    argcheck:
        luaL_argcheck(L, false, idx, lua_tostring(L, -1));
        return nullptr;
    }

    /* FIXME: lengths (and character) in these APIs may be given by cdata... */

    static int copy_f(lua_State *L) {
        void *dst = check_voidptr(L, 1);
        void const *src;
        std::size_t len;
        if (lua_isstring(L, 2)) {
            src = lua_tostring(L, 2);
            if (lua_gettop(L) <= 2) {
                len = lua_rawlen(L, 2);
            } else {
                len = ffi::check_arith<std::size_t>(L, 3);
            }
        } else {
            src = check_voidptr(L, 2);
            len = ffi::check_arith<std::size_t>(L, 3);
        }
        std::memcpy(dst, src, len);
        return 0;
    }

    static int fill_f(lua_State *L) {
        void *dst = check_voidptr(L, 1);
        auto len = ffi::check_arith<std::size_t>(L, 2);
        int byte = int(luaL_optinteger(L, 3, 0));
        std::memset(dst, byte, len);
        return 0;
    }

    static int tonumber_f(lua_State *L) {
        auto *cd = ffi::testcdata(L, 1);
        if (cd) {
            if (cd->decl.arith()) {
                ffi::to_lua(L, cd->decl, cd->as_deref_ptr(), ffi::RULE_CONV, false, true);
                return 1;
            }
            switch (cd->decl.type()) {
                case ast::C_BUILTIN_PTR:
                case ast::C_BUILTIN_RECORD:
                case ast::C_BUILTIN_ARRAY:
                case ast::C_BUILTIN_FUNC:
                    /* these may appear */
                    lua_pushnil(L);
                    return 1;
                default:
                    /* these should not */
                    assert(false);
                    lua_pushnil(L);
                    return 1;
            }
        }
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_insert(L, 1);
        lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
        return lua_gettop(L);
    }

    static int toretval_f(lua_State *L) {
        auto &cd = ffi::checkcdata(L, 1);
        ffi::to_lua(L, cd.decl, &cd.as<void *>(), ffi::RULE_RET, false);
        return 1;
    }

    static int eval_f(lua_State *L) {
        /* TODO: accept expressions */
        char const *str = luaL_checkstring(L, 1);
        ast::c_value outv;
        auto v = parser::parse_number(L, outv, str, str + lua_rawlen(L, 1));
        ffi::make_cdata_arith(L, v, outv);
        return 1;
    }

    static int type_f(lua_State *L) {
        if (ffi::iscval(L, 1)) {
            lua_pushliteral(L, "cdata");
            return 1;
        }
        luaL_checkany(L, 1);
        lua_pushstring(L, luaL_typename(L, 1));
        return 1;
    }

    static int abi_f(lua_State *L) {
        luaL_checkstring(L, 1);
        lua_pushvalue(L, 1);
        lua_rawget(L, lua_upvalueindex(1));
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushboolean(L, false);
        }
        return 1;
    }

    static void setup_abi(lua_State *L) {
        lua_newtable(L);
        lua_pushboolean(L, true);
        if (sizeof(void *) == 8) {
            lua_setfield(L, -2, "64bit");
        } else if (sizeof(void *) == 4) {
            lua_setfield(L, -2, "32bit");
        } else {
            lua_pop(L, 1);
        }
        lua_pushboolean(L, true);
#if defined(FFI_BIG_ENDIAN)
        lua_setfield(L, -2, "be");
#else
        lua_setfield(L, -2, "le");
#endif
#ifdef FFI_WINDOWS_ABI
        lua_pushboolean(L, true);
        lua_setfield(L, -2, "win");
#endif
#ifdef FFI_WINDOWS_UWP
        lua_pushboolean(L, true);
        lua_setfield(L, -2, "uwp");
#endif
#ifdef FFI_ARM_EABI
        lua_pushboolean(L, true);
        lua_setfield(L, -2, "eabi");
#endif
#if FFI_ARCH == FFI_ARCH_PPC64 && defined(_CALL_ELF) && _CALL_ELF == 2
        lua_pushboolean(L, true);
        lua_setfield(L, -2, "elfv2");
#endif
#if FFI_ARCH_HAS_FPU == 1
        lua_pushboolean(L, true);
        lua_setfield(L, -2, "fpu");
#endif
        lua_pushboolean(L, true);
#if FFI_ARCH_SOFTFP == 1
        lua_setfield(L, -2, "softfp");
#else
        lua_setfield(L, -2, "hardfp");
#endif
#ifdef FFI_ABI_UNIONVAL
        lua_pushboolean(L, true);
        lua_setfield(L, -2, "unionval");
#endif
    }

    static void setup(lua_State *L) {
        static luaL_Reg const lib_def[] = {/* core */
                                           {"cdef", cdef_f},
                                           {"load", load_f},

                                           /* data handling */
                                           {"new", new_f},
                                           {"cast", cast_f},
                                           {"metatype", metatype_f},
                                           {"typeof", typeof_f},
                                           {"addressof", addressof_f},
                                           {"gc", gc_f},

                                           /* type info */
                                           {"sizeof", sizeof_f},
                                           {"alignof", alignof_f},
                                           {"offsetof", offsetof_f},
                                           {"istype", istype_f},

                                           /* utilities */
                                           {"errno", errno_f},
                                           {"string", string_f},
                                           {"copy", copy_f},
                                           {"fill", fill_f},
                                           {"toretval", toretval_f},
                                           {"eval", eval_f},
                                           {"type", type_f},

                                           {nullptr, nullptr}};
        luaL_newlib(L, lib_def);

        lua_pushliteral(L, FFI_OS_NAME);
        lua_setfield(L, -2, "os");

        lua_pushliteral(L, FFI_ARCH_NAME);
        lua_setfield(L, -2, "arch");

        setup_abi(L);
        lua_pushcclosure(L, abi_f, 1);
        lua_setfield(L, -2, "abi");

        /* FIXME: relying on the global table being intact */
        lua_getglobal(L, "tonumber");
        lua_pushcclosure(L, tonumber_f, 1);
        lua_setfield(L, -2, "tonumber");

        /* NULL = (void *)0 */
        ffi::newcdata(L, ast::c_type{util::make_rc<ast::c_type>(ast::C_BUILTIN_VOID, 0), 0, ast::C_BUILTIN_PTR}, sizeof(void *)).as<void *>() = nullptr;
        lua_setfield(L, -2, "nullptr");
    }

    static void setup_dstor(lua_State *L) {
        /* our declaration storage is a userdata in the registry */
        auto *ds = static_cast<ast::decl_store *>(lua_newuserdata(L, sizeof(ast::decl_store)));
        new (ds) ast::decl_store{};
        /* stack: dstor */
        lua_newtable(L);
        /* stack: dstor, mt */
        lua_pushcfunction(L, [](lua_State *LL) -> int {
            using T = ast::decl_store;
            auto *dsp = lua::touserdata<T>(LL, 1);
            dsp->~T();
            return 0;
        });
        /* stack: dstor, mt, __gc */
        lua_setfield(L, -2, "__gc");
        /* stack: dstor, __mt */
        lua_setmetatable(L, -2);
        /* stack: dstor */
        lua_setfield(L, LUA_REGISTRYINDEX, lua::CFFI_DECL_STOR);
        /* stack: empty */
    }

    static void open(lua_State *L) {
        setup_dstor(L); /* declaration store */
        parser::init(L);

        /* cdata handles */
        cdata_meta::setup(L);

        setup(L); /* push table to stack */

        /* lib handles, needs the module table on the stack */
        auto *c_ud = static_cast<lib::c_lib *>(lua_newuserdata(L, sizeof(lib::c_lib)));
        new (c_ud) lib::c_lib{};
        lib::load(c_ud, nullptr, L, false);
        lib_meta::setup(L);
    }
};

void ffi_module_open(lua_State *L) { ffi_module::open(L); }

namespace ffi {

static void *from_lua(lua_State *L, ast::c_type const &tp, void *stor, int index, std::size_t &dsz, int rule);

static inline void fail_convert_cd(lua_State *L, ast::c_type const &from, ast::c_type const &to) {
    from.serialize(L);
    to.serialize(L);
    luaL_error(L, "cannot convert '%s' to '%s'", lua_tostring(L, -2), lua_tostring(L, -1));
}

static inline void fail_convert_tp(lua_State *L, char const *from, ast::c_type const &to) {
    to.serialize(L);
    luaL_error(L, "cannot convert '%s' to '%s'", from, lua_tostring(L, -1));
}

static ffi_type *lua_to_vararg(lua_State *L, int index) {
    switch (lua_type(L, index)) {
        case LUA_TBOOLEAN:
            return &ffi_type_uchar;
        case LUA_TNUMBER:
            /* 5.3+; always returns false on <= 5.2 */
            if (lua_isinteger(L, index)) {
                return ffi_traits<lua_Integer>::type();
            }
            return ffi_traits<lua_Number>::type();
        case LUA_TNIL:
        case LUA_TSTRING:
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TTHREAD:
        case LUA_TLIGHTUSERDATA:
            return &ffi_type_pointer;
        case LUA_TUSERDATA: {
            auto *cd = testcdata(L, index);
            /* plain userdata or struct values are passed to varargs as ptrs */
            if (!cd || (cd->decl.type() == ast::C_BUILTIN_RECORD)) {
                return &ffi_type_pointer;
            }
            return cd->decl.libffi_type();
        }
        default:
            break;
    }
    assert(false);
    return &ffi_type_void;
}

static inline void *fdata_retval(fdata &fd) { return &fd.rarg; }

static inline ffi::scalar_stor_t *&fdata_get_aux(fdata &fd) { return *util::pun<ffi::scalar_stor_t **>(fd.args()); }

static inline void fdata_free_aux(lua_State *, fdata &fd) {
    auto &ptr = fdata_get_aux(fd);
    delete[] util::pun<unsigned char *>(ptr);
    ptr = nullptr;
}

static inline void fdata_new_aux(lua_State *, fdata &fd, std::size_t sz) { fdata_get_aux(fd) = util::pun<ffi::scalar_stor_t *>(new unsigned char[sz]); }

static inline ffi_type **fargs_types(ffi::scalar_stor_t *args, std::size_t nargs) {
    /* see memory layout comment below; this accesses the beginning
     * of the ffi_type section within the fdata structure
     */
    return util::pun<ffi_type **>(args + nargs);
}

static inline void **fargs_values(ffi::scalar_stor_t *args, std::size_t nargs) {
    /* this accesses the value pointers that follow the ffi_type pointers */
    return util::pun<void **>(fargs_types(args, nargs) + nargs);
}

void destroy_cdata(lua_State *L, cdata &cd) {
    if (cd.gc_ref >= 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, cd.gc_ref);
        lua_pushvalue(L, 1); /* the cdata */
        if (lua_pcall(L, 1, 0, 0)) {
            lua_pop(L, 1);
        }
        luaL_unref(L, LUA_REGISTRYINDEX, cd.gc_ref);
    }
    switch (cd.decl.type()) {
        case ast::C_BUILTIN_PTR:
            if (cd.decl.ptr_base().type() != ast::C_BUILTIN_FUNC) {
                break;
            }
            goto free_aux;
        free_aux:
        case ast::C_BUILTIN_FUNC: {
            if (!cd.decl.function()->variadic()) {
                break;
            }
            fdata_free_aux(L, cd.as<fdata>());
        }
        default:
            break;
    }
    using T = ast::c_type;
    cd.decl.~T();
}

void destroy_closure(lua_State *, closure_data *cd) {
    cd->~closure_data();
    delete[] util::pun<unsigned char *>(cd);
}

static void cb_bind(ffi_cif *, void *ret, void *args[], void *data) {
    auto &fud = *static_cast<cdata *>(data);
    auto &fun = fud.decl.function();
    auto &pars = fun->params();
    auto fargs = pars.size();

    closure_data &cd = *fud.as<fdata>().cd;
    lua_rawgeti(cd.L, LUA_REGISTRYINDEX, cd.fref);
    for (std::size_t i = 0; i < fargs; ++i) {
        to_lua(cd.L, pars[i].type(), args[i], RULE_PASS, false);
    }

    if (fun->result().type() != ast::C_BUILTIN_VOID) {
        lua_call(cd.L, int(fargs), 1);
        ffi::scalar_stor_t stor{};
        std::size_t rsz;
        void *rp = from_lua(cd.L, fun->result(), &stor, -1, rsz, RULE_RET);
        std::memcpy(ret, rp, rsz);
        lua_pop(cd.L, 1);
    } else {
        lua_call(cd.L, int(fargs), 0);
    }
}

#if defined(FFI_WINDOWS_ABI) && (FFI_ARCH == FFI_ARCH_X86)
static inline ffi_abi to_libffi_abi(int conv) {
    switch (conv) {
        case ast::C_FUNC_DEFAULT:
            return FFI_DEFAULT_ABI;
        case ast::C_FUNC_CDECL:
            return FFI_MS_CDECL;
        case ast::C_FUNC_FASTCALL:
            return FFI_FASTCALL;
        case ast::C_FUNC_STDCALL:
            return FFI_STDCALL;
        case ast::C_FUNC_THISCALL:
            return FFI_THISCALL;
        default:
            break;
    }
    assert(false);
    return FFI_DEFAULT_ABI;
}
#else
static inline ffi_abi to_libffi_abi(int) { return FFI_DEFAULT_ABI; }
#endif

/* this initializes a non-vararg cif with the given number of arguments
 * for variadics, this is initialized once for zero args, and then handled
 * dynamically before every call
 */
static bool prepare_cif(util::rc_obj<ast::c_function> const &func, ffi_cif &cif, ffi_type **targs, std::size_t nargs) {
    for (std::size_t i = 0; i < nargs; ++i) {
        targs[i] = func->params()[i].libffi_type();
    }
    using U = unsigned int;
    return (ffi_prep_cif(&cif, to_libffi_abi(func->callconv()), U(nargs), func->result().libffi_type(), targs) == FFI_OK);
}

static void make_cdata_func(lua_State *L, void (*funp)(), util::rc_obj<ast::c_function> func, bool fptr, closure_data *cd) {
    auto nargs = func->params().size();

    /* MEMORY LAYOUT:
     *
     * regular func:
     *
     * struct cdata {
     *     <cdata header>
     *     struct fdata {
     *         <fdata header>
     *         ffi::scalar_stor_t val1; // lua arg1
     *         ffi::scalar_stor_t val2; // lua arg2
     *         ffi::scalar_stor_t valN; // lua argN
     *         ffi_type *arg1; // type
     *         ffi_type *arg2; // type
     *         ffi_type *argN; // type
     *         void *valp1;    // &val1
     *         void *valpN;    // &val2
     *         void *valpN;    // &valN
     *     } val;
     * }
     *
     * vararg func:
     *
     * struct cdata {
     *     <cdata header>
     *     struct fdata {
     *         <fdata header>
     *         void *aux; // vals + types + args like above, but dynamic
     *     } val;
     * }
     */
    ast::c_type funct{func, 0, funp == nullptr};
    auto &fud = newcdata(L, fptr ? ast::c_type{util::make_rc<ast::c_type>(util::move(funct)), 0, ast::C_BUILTIN_PTR} : util::move(funct),
                         sizeof(fdata) + (func->variadic() ? sizeof(void *) : (sizeof(ffi::scalar_stor_t) * nargs + sizeof(void *) * nargs * 2)));
    fud.as<fdata>().sym = funp;

    if (func->variadic()) {
        fdata_get_aux(fud.as<fdata>()) = nullptr;
        if (!funp) {
            luaL_error(L, "variadic callbacks are not supported");
        }
        nargs = 0;
    }

    if (!prepare_cif(func, fud.as<fdata>().cif, fargs_types(fud.as<fdata>().args(), nargs), nargs)) {
        luaL_error(L, "unexpected failure setting up '%s'", func->name());
    }

    if (!funp) {
        /* no funcptr means we're setting up a callback */
        if (cd) {
            /* copying existing callback reference */
            fud.as<fdata>().cd = cd;
            return;
        }
        cd = util::pun<closure_data *>(new unsigned char[sizeof(closure_data) + nargs * sizeof(ffi_type *)]);
        new (cd) closure_data{};
        /* allocate a closure in it */
        void *symp;
        cd->closure = static_cast<ffi_closure *>(ffi_closure_alloc(sizeof(ffi_closure), &symp));
        std::memcpy(&fud.as<fdata>().sym, &symp, sizeof(void *));
        if (!cd->closure) {
            destroy_closure(L, cd);
            func->serialize(L);
            luaL_error(L, "failed allocating callback for '%s'", lua_tostring(L, -1));
        }
        /* arg pointers follow closure data struct, as allocated above */
        auto **targs = util::pun<ffi_type **>(cd + 1);
        if (!prepare_cif(fud.decl.function(), cd->cif, targs, nargs)) {
            destroy_closure(L, cd);
            luaL_error(L, "unexpected failure setting up '%s'", func->name());
        }
        if (ffi_prep_closure_loc(cd->closure, &fud.as<fdata>().cif, cb_bind, &fud, symp) != FFI_OK) {
            destroy_closure(L, cd);
            func->serialize(L);
            luaL_error(L, "failed initializing closure for '%s'", lua_tostring(L, -1));
        }
        cd->L = L;
        fud.as<fdata>().cd = cd;
    }
}

static bool prepare_cif_var(lua_State *L, cdata &fud, std::size_t nargs, std::size_t fargs) {
    auto &func = fud.decl.function();

    auto &auxptr = fdata_get_aux(fud.as<fdata>());
    if (auxptr && (nargs > std::size_t(fud.aux))) {
        fdata_free_aux(L, fud.as<fdata>());
    }
    if (!auxptr) {
        fdata_new_aux(L, fud.as<fdata>(), nargs * sizeof(ffi::scalar_stor_t) + 2 * nargs * sizeof(void *));
        fud.aux = int(nargs);
    }

    ffi_type **targs = fargs_types(auxptr, nargs);
    for (std::size_t i = 0; i < fargs; ++i) {
        targs[i] = func->params()[i].libffi_type();
    }
    for (std::size_t i = fargs; i < nargs; ++i) {
        targs[i] = lua_to_vararg(L, int(i + 2));
    }

    using U = unsigned int;
    return (ffi_prep_cif_var(&fud.as<fdata>().cif, to_libffi_abi(func->callconv()), U(fargs), U(nargs), func->result().libffi_type(), targs) == FFI_OK);
}

int call_cif(cdata &fud, lua_State *L, std::size_t largs) {
    auto &func = fud.decl.function();
    auto &pdecls = func->params();

    auto nargs = pdecls.size();
    auto targs = nargs;

    ffi::scalar_stor_t *pvals = fud.as<fdata>().args();
    void *rval = fdata_retval(fud.as<fdata>());

    if (func->variadic()) {
        targs = util::max(largs, nargs);
        if (!prepare_cif_var(L, fud, targs, nargs)) {
            luaL_error(L, "unexpected failure setting up '%s'", func->name());
        }
        pvals = fdata_get_aux(fud.as<fdata>());
    }

    void **vals = fargs_values(pvals, targs);
    /* fixed args */
    for (int i = 0; i < int(nargs); ++i) {
        std::size_t rsz;
        vals[i] = from_lua(L, pdecls[i].type(), &pvals[i], i + 2, rsz, RULE_PASS);
    }
    /* variable args */
    for (int i = int(nargs); i < int(targs); ++i) {
        std::size_t rsz;
        auto tp = ast::from_lua_type(L, i + 2);
        if (tp.type() == ast::C_BUILTIN_RECORD) {
            /* special case for vararg passing of records: by ptr */
            auto &cd = tocdata(L, i + 2);
            std::memcpy(&pvals[i], cd.as_ptr(), sizeof(void *));
            continue;
        }
        vals[i] = from_lua(L, util::move(tp), &pvals[i], i + 2, rsz, RULE_PASS);
    }

    ffi_call(&fud.as<fdata>().cif, fud.as<fdata>().sym, rval, vals);
    return to_lua(L, func->result(), rval, RULE_RET, true);
}

template <typename T>
static inline int push_int(lua_State *L, ast::c_type const &tp, void const *value, bool rv, bool lossy) {
#if LUA_VERSION_NUM < 503
    /* generally floats, so we're assuming IEEE754 binary floats */
    static_assert(util::limit_radix<lua_Number>() == 2, "unsupported lua_Number type");
    using LT = lua_Number;
#else
    /* on lua 5.3+, we can use integers builtin in the language instead */
    using LT = lua_Integer;
#endif
    T actual_val;
    if (rv && (sizeof(T) < sizeof(ffi_sarg))) {
        using U = ffi_sarg *;
        actual_val = T(*U(value));
    } else {
        using U = T *;
        actual_val = *U(value);
    }
    if ((util::limit_digits<T>() <= util::limit_digits<LT>()) || lossy) {
        lua_pushinteger(L, lua_Integer(actual_val));
        return 1;
    }
    /* doesn't fit into the range, so make scalar cdata */
    auto &cd = newcdata(L, tp, sizeof(T));
    std::memcpy(cd.as_ptr(), &actual_val, sizeof(T));
    return 1;
}

template <typename T>
static inline int push_flt(lua_State *L, ast::c_type const &tp, void const *value, bool lossy) {
    /* probably not the best check */
    if ((util::limit_max<T>() <= util::limit_max<lua_Number>()) || lossy) {
        using U = T *;
        lua_pushnumber(L, lua_Number(*U(value)));
        return 1;
    }
    auto &cd = newcdata(L, tp, sizeof(T));
    std::memcpy(cd.as_ptr(), value, sizeof(T));
    return 1;
}

int to_lua(lua_State *L, ast::c_type const &tp, void const *value, int rule, bool ffi_ret, bool lossy) {
    if (tp.is_ref()) {
        /* dereference regardless */
        auto *dval = *static_cast<void *const *>(value);
        if (tp.type() == ast::C_BUILTIN_FUNC) {
            make_cdata_func(L, util::pun<void (*)()>(dval), tp.function(), rule != RULE_CONV, nullptr);
            return 1;
        }
        if (rule == RULE_CONV) {
            /* dereference, continue as normal */
            value = dval;
            ffi_ret = false;
        } else {
            /* reference cdata */
            newcdata(L, tp, sizeof(void *)).as<void *>() = dval;
            return 1;
        }
    }

    switch (ast::c_builtin(tp.type())) {
        /* no retval */
        case ast::C_BUILTIN_VOID:
            return 0;
        /* convert to lua boolean */
        case ast::C_BUILTIN_BOOL:
            lua_pushboolean(L, *static_cast<bool const *>(value));
            return 1;
        /* convert to lua number */
        case ast::C_BUILTIN_FLOAT:
            return push_flt<float>(L, tp, value, lossy);
        case ast::C_BUILTIN_DOUBLE:
            return push_flt<double>(L, tp, value, lossy);
        case ast::C_BUILTIN_LDOUBLE:
            return push_flt<long double>(L, tp, value, lossy);
        case ast::C_BUILTIN_CHAR:
            return push_int<char>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_SCHAR:
            return push_int<signed char>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_UCHAR:
            return push_int<unsigned char>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_SHORT:
            return push_int<short>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_USHORT:
            return push_int<unsigned short>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_INT:
            return push_int<int>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_UINT:
            return push_int<unsigned int>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_LONG:
            return push_int<long>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_ULONG:
            return push_int<unsigned long>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_LLONG:
            return push_int<long long>(L, tp, value, ffi_ret, lossy);
        case ast::C_BUILTIN_ULLONG:
            return push_int<unsigned long long>(L, tp, value, ffi_ret, lossy);

        case ast::C_BUILTIN_PTR:
            if (tp.ptr_base().type() == ast::C_BUILTIN_FUNC) {
                return to_lua(L, tp.ptr_base(), value, rule, false, lossy);
            }
            /* pointers should be handled like large cdata, as they need
             * to be represented as userdata objects on lua side either way
             */
            newcdata(L, tp, sizeof(void *)).as<void *>() = *static_cast<void *const *>(value);
            return 1;

        case ast::C_BUILTIN_VA_LIST:
            newcdata(L, tp, sizeof(void *)).as<void *>() = *static_cast<void *const *>(value);
            return 1;

        case ast::C_BUILTIN_FUNC: {
            make_cdata_func(L, util::pun<void (*)()>(*static_cast<void *const *>(value)), tp.function(), true, nullptr);
            return 1;
        }

        case ast::C_BUILTIN_ENUM:
            /* TODO: large enums */
            return push_int<int>(L, tp, value, ffi_ret, lossy);

        case ast::C_BUILTIN_ARRAY: {
            if (rule == RULE_PASS) {
                /* pass rule: only when passing to array args in callbacks
                 * in this case we just drop the array bit and use a pointer
                 */
                newcdata(L, tp.as_type(ast::C_BUILTIN_PTR), sizeof(void *)).as<void *>() = *static_cast<void *const *>(value);
                return 1;
            }
            /* this case may be encountered twice, when retrieving array
             * members of cdata, or when retrieving global array cdata; any
             * other cases are not possible (e.g. you can't return an array)
             *
             * we need to create a C++ style reference in possible cases
             */
            auto &cd = newcdata(L, tp.as_ref(), sizeof(void *) * 2);
            cd.as<void const *[2]>()[1] = value;
            cd.as<void const *[2]>()[0] = &cd.as<void const *[2]>()[1];
            return 1;
        }

        case ast::C_BUILTIN_RECORD: {
            if (rule == RULE_CONV) {
                newcdata(L, tp.as_ref(), sizeof(void *)).as<void const *>() = value;
                return 1;
            }
            auto sz = tp.alloc_size();
            auto &cd = newcdata(L, tp, sz);
            std::memcpy(cd.as_ptr(), value, sz);
            return 1;
        }

        case ast::C_BUILTIN_INVALID:
            break;
    }

    luaL_error(L, "unexpected error: unhandled type %d", tp.type());
    return 0;
}

template <typename T>
static inline void write_int(lua_State *L, int index, void *stor, std::size_t &s) {
    if (lua_isinteger(L, index)) {
        *static_cast<T *>(stor) = T(lua_tointeger(L, index));
    } else if (lua_isboolean(L, index)) {
        *static_cast<T *>(stor) = T(lua_toboolean(L, index));
    } else {
        *static_cast<T *>(stor) = T(lua_tonumber(L, index));
    }
    s = sizeof(T);
}

template <typename T>
static inline void write_flt(lua_State *L, int index, void *stor, std::size_t &s) {
    lua_Number v = lua_isboolean(L, index) ? lua_toboolean(L, index) : lua_tonumber(L, index);
    *static_cast<T *>(stor) = T(v);
    s = sizeof(T);
}

static void from_lua_num(lua_State *L, ast::c_type const &tp, void *stor, int index, std::size_t &dsz, int rule) {
    if (tp.is_ref() && (rule == RULE_CAST)) {
        dsz = sizeof(void *);
        *static_cast<void **>(stor) = util::pun<void *>(std::size_t(lua_tointeger(L, index)));
        return;
    }

    switch (ast::c_builtin(tp.type())) {
        case ast::C_BUILTIN_FLOAT:
            write_flt<float>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_DOUBLE:
            write_flt<double>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_LDOUBLE:
            write_flt<long double>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_BOOL:
            write_int<bool>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_CHAR:
            write_int<char>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_SCHAR:
            write_int<signed char>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_UCHAR:
            write_int<unsigned char>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_SHORT:
            write_int<short>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_USHORT:
            write_int<unsigned short>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_INT:
            write_int<int>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_UINT:
            write_int<unsigned int>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_LONG:
            write_int<long>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_ULONG:
            write_int<unsigned long>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_LLONG:
            write_int<long long>(L, index, stor, dsz);
            return;
        case ast::C_BUILTIN_ULLONG:
            write_int<unsigned long long>(L, index, stor, dsz);
            return;

        case ast::C_BUILTIN_ENUM:
            /* TODO: large enums */
            write_int<int>(L, index, stor, dsz);
            return;

        case ast::C_BUILTIN_PTR:
            if (rule == RULE_CAST) {
                dsz = sizeof(void *);
                *static_cast<void **>(stor) = util::pun<void *>(std::size_t(lua_tointeger(L, index)));
                return;
            }
            goto converr;

        converr:
        case ast::C_BUILTIN_VOID:
        case ast::C_BUILTIN_RECORD:
        case ast::C_BUILTIN_ARRAY:
        case ast::C_BUILTIN_VA_LIST:
            fail_convert_tp(L, lua_typename(L, lua_type(L, index)), tp);
            break;

        case ast::C_BUILTIN_FUNC:
        case ast::C_BUILTIN_INVALID:
            /* this should not happen */
            tp.serialize(L);
            luaL_error(L, "bad argument type '%s'", lua_tostring(L, -1));
            break;
    }
    assert(false);
}

static inline bool cv_convertible(int scv, int tcv) {
    if (!(tcv & ast::C_CV_CONST) && (scv & ast::C_CV_CONST)) {
        return false;
    }
    if (!(tcv & ast::C_CV_VOLATILE) && (scv & ast::C_CV_VOLATILE)) {
        return false;
    }
    return true;
}

static inline bool func_convertible(ast::c_function const &from, ast::c_function const &to) {
    if (from.variadic() != to.variadic()) {
        return false;
    }
    if (from.params().size() != to.params().size()) {
        return false;
    }
    return true;
}

static inline bool ptr_convertible(ast::c_type const &from, ast::c_type const &to) {
    auto &fpb = from.is_ref() ? from : from.ptr_base();
    auto &tpb = to.is_ref() ? to : to.ptr_base();
    if (!cv_convertible(fpb.cv(), tpb.cv())) {
        return false;
    }
    if ((fpb.type() == ast::C_BUILTIN_VOID) || (tpb.type() == ast::C_BUILTIN_VOID)) {
        /* from or to void pointer is always ok */
        return true;
    }
    if ((fpb.type() == ast::C_BUILTIN_PTR) && (tpb.type() == ast::C_BUILTIN_PTR)) {
        return ptr_convertible(fpb, tpb);
    }
    return fpb.is_same(tpb, true, true);
}

/* converting from cdata: pointer */
static void from_lua_cdata_ptr(lua_State *L, ast::c_type const &cd, ast::c_type const &tp, int rule) {
    /* converting to reference */
    if (tp.is_ref()) {
        goto isptr;
    }
    switch (tp.type()) {
        /* converting to pointer */
        case ast::C_BUILTIN_PTR:
            break;
        /* converting to array is okay when passing,
         * e.g. void foo(int[2]) being given int *
         */
        case ast::C_BUILTIN_ARRAY:
            if (rule == RULE_PASS) {
                break;
            }
            fail_convert_cd(L, cd, tp);
            break;
        /* converting to anything else */
        default:
            if ((rule == RULE_CAST) && tp.integer()) {
                /* casting to integer types is fine, it's the user's
                 * responsibility to ensure it's safe by using correct types
                 */
                return;
            }
            fail_convert_cd(L, cd, tp);
            break;
    }

isptr:
    if (rule == RULE_CAST) {
        /* casting: disregard any typing rules */
        return;
    }
    /* initializing a function pointer/reference */
    if (tp.ptr_ref_base().type() == ast::C_BUILTIN_FUNC) {
        if (cd.type() == ast::C_BUILTIN_FUNC) {
            /* plain function: check convertible, init from addr */
            if (!func_convertible(*cd.function(), *tp.ptr_base().function())) {
                fail_convert_cd(L, cd, tp);
            }
            return;
        } else if ((cd.type() != ast::C_BUILTIN_PTR) && !cd.is_ref()) {
            /* otherwise given value must be a pointer/ref */
            fail_convert_cd(L, cd, tp);
        }
        /* it must be a pointer/ref to function */
        if (cd.ptr_ref_base().type() != ast::C_BUILTIN_FUNC) {
            fail_convert_cd(L, cd, tp);
        }
        /* and it must satisfy convertible check */
        if (!func_convertible(*cd.ptr_ref_base().function(), *tp.ptr_ref_base().function())) {
            fail_convert_cd(L, cd, tp);
        }
        /* then init from address */
        return;
    }
    if (!ptr_convertible(cd, tp)) {
        fail_convert_cd(L, cd, tp);
    }
}

template <typename T>
static void from_lua_cnumber(lua_State *L, ast::c_type const &cd, ast::c_type const &tp, void *sval, void *stor, std::size_t &dsz, int rule) {
#define CONV_CASE(name, U)                             \
    case ast::C_BUILTIN_##name: {                      \
        dsz = sizeof(U);                               \
        T val;                                         \
        std::memcpy(&val, sval, sizeof(T));            \
        *static_cast<U *>(stor) = static_cast<U>(val); \
        return;                                        \
    }

    if (tp.is_ref()) {
        goto ptr_ref;
    }

    switch (tp.type()) {
        case ast::C_BUILTIN_PTR:
            goto ptr_ref;
            CONV_CASE(ENUM, int)
            CONV_CASE(BOOL, bool)
            CONV_CASE(CHAR, char)
            CONV_CASE(SCHAR, signed char)
            CONV_CASE(UCHAR, unsigned char)
            CONV_CASE(SHORT, short)
            CONV_CASE(USHORT, unsigned short)
            CONV_CASE(INT, int)
            CONV_CASE(UINT, unsigned int)
            CONV_CASE(LONG, long)
            CONV_CASE(ULONG, unsigned long)
            CONV_CASE(LLONG, long long)
            CONV_CASE(ULLONG, unsigned long long)
            CONV_CASE(FLOAT, float)
            CONV_CASE(DOUBLE, double)
            CONV_CASE(LDOUBLE, long double)
        default:
            fail_convert_cd(L, cd, tp);
            return;
    }

#undef CONV_CASE

ptr_ref:
    /* only for cast we can initialize pointers with integer addrs */
    if (rule != RULE_CAST) {
        fail_convert_cd(L, cd, tp);
        return;
    }
    /* must not be floating point */
    if (!util::is_int<T>::value) {
        fail_convert_cd(L, cd, tp);
        return;
    }
    dsz = sizeof(void *);
    *static_cast<void **>(stor) = util::pun<void *>(std::size_t(*static_cast<T *>(sval)));
}

static void *from_lua_cdata(lua_State *L, ast::c_type const &cd, ast::c_type const &tp, void *sval, void *stor, std::size_t &dsz, int rule) {
    /* arrays always decay to pointers first */
    if (cd.type() == ast::C_BUILTIN_ARRAY) {
        return from_lua_cdata(L, cd.as_type(ast::C_BUILTIN_PTR), tp, sval, stor, dsz, rule);
    }
    /* we're passing an argument and the expected type is a reference...
     * this is a special case, the given type must be either a non-reference
     * type that matches the base type of the reference and has same or weaker
     * qualifiers - then its address is taken - or a matching reference type,
     * then it's passed as-is
     */
    if ((rule == RULE_PASS) && tp.is_ref()) {
        if (cd.is_ref()) {
            return from_lua_cdata(L, cd.unref(), tp, *static_cast<void **>(sval), stor, dsz, rule);
        }
        if (!cv_convertible(cd.cv(), tp.cv())) {
            fail_convert_cd(L, cd, tp);
        }
        if (!cd.is_same(tp, true, true)) {
            fail_convert_cd(L, cd, tp);
        }
        dsz = sizeof(void *);
        return &(*static_cast<void **>(stor) = sval);
    }
    if (cd.is_ref()) {
        /* always dereference */
        return from_lua_cdata(L, cd.unref(), tp, *static_cast<void **>(sval), stor, dsz, rule);
    }
    switch (cd.type()) {
        case ast::C_BUILTIN_PTR:
            from_lua_cdata_ptr(L, cd, tp, rule);
            dsz = sizeof(void *);
            return sval;
        case ast::C_BUILTIN_FUNC:
            if ((tp.type() != ast::C_BUILTIN_PTR) && !tp.is_ref()) {
                /* converting from func: must be to some kind of pointer */
                fail_convert_cd(L, cd, tp);
            }
            if (rule == RULE_CAST) {
                /* casting: ignore rules, convert to any pointer */
                dsz = sizeof(void *);
                return sval;
            }
            /* not casting: some rules must be followed */
            if (tp.ptr_ref_base().type() != ast::C_BUILTIN_FUNC) {
                fail_convert_cd(L, cd, tp);
            }
            if (!func_convertible(*cd.function(), *tp.ptr_ref_base().function())) {
                fail_convert_cd(L, cd, tp);
            }
            dsz = sizeof(void *);
            return sval;
        case ast::C_BUILTIN_RECORD: {
            /* we can pass structs by value in non-cast context,
             * as well as pointers and references by address
             */
            bool do_copy = ((tp.type() != ast::C_BUILTIN_PTR) && !tp.is_ref());
            if (do_copy && (rule == RULE_CAST)) {
                break;
            }
            if (rule != RULE_CAST) {
                if (do_copy) {
                    if (!cd.is_same(tp, true)) {
                        break;
                    }
                } else {
                    if (!cv_convertible(cd.cv(), tp.ptr_ref_base().cv())) {
                        break;
                    }
                }
            }
            if (do_copy) {
                dsz = cd.alloc_size();
                return sval;
            } else {
            }
            dsz = sizeof(void *);
            return &(*static_cast<void **>(stor) = sval);
        }
        default:
            if (cd.is_same(tp, true)) {
                dsz = cd.alloc_size();
                return sval;
            }
            break;
    }

#define CONV_CASE(name, T)                                     \
    case ast::C_BUILTIN_##name:                                \
        from_lua_cnumber<T>(L, cd, tp, sval, stor, dsz, rule); \
        return stor;

    switch (cd.type()) {
        CONV_CASE(ENUM, int)
        CONV_CASE(BOOL, bool)
        CONV_CASE(CHAR, char)
        CONV_CASE(SCHAR, signed char)
        CONV_CASE(UCHAR, unsigned char)
        CONV_CASE(SHORT, short)
        CONV_CASE(USHORT, unsigned short)
        CONV_CASE(INT, int)
        CONV_CASE(UINT, unsigned int)
        CONV_CASE(LONG, long)
        CONV_CASE(ULONG, unsigned long)
        CONV_CASE(LLONG, long long)
        CONV_CASE(ULLONG, unsigned long long)
        CONV_CASE(FLOAT, float)
        CONV_CASE(DOUBLE, double)
        CONV_CASE(LDOUBLE, long double)
        default:
            break;
    }

#undef CONV_CASE

    fail_convert_cd(L, cd, tp);
    return nullptr;
}

/* this returns a pointer to a C value counterpart of the Lua value
 * on the stack (as given by `index`) while checking types (`rule`)
 *
 * necessary conversions are done according to `tp`; `stor` is used to
 * write scalar values (therefore its alignment and size must be enough
 * to fit the converted value - the ffi::scalar_stor_t type can store any
 * scalar so you can use that) while non-scalar values may have their address
 * returned directly
 */
static void *from_lua(lua_State *L, ast::c_type const &tp, void *stor, int index, std::size_t &dsz, int rule) {
    /* sanitize the output type early on */
    switch (tp.type()) {
        case ast::C_BUILTIN_FUNC:
        case ast::C_BUILTIN_VOID:
        case ast::C_BUILTIN_INVALID:
            luaL_error(L, "invalid C type");
            break;
        case ast::C_BUILTIN_ARRAY:
            /* special cased for passing because those are passed by ptr */
            if (rule != RULE_PASS) {
                luaL_error(L, "invalid C type");
            }
            break;
        case ast::C_BUILTIN_RECORD:
            /* structs can be copied via new/pass but not casted */
            if (rule == RULE_CAST) {
                luaL_error(L, "invalid C type");
            }
        default:
            break;
    }
    auto vtp = lua_type(L, index);
    switch (vtp) {
        case LUA_TNIL:
            if (tp.is_ref() || (tp.type() == ast::C_BUILTIN_PTR)) {
                dsz = sizeof(void *);
                return &(*static_cast<void **>(stor) = nullptr);
            }
            fail_convert_tp(L, "nil", tp);
            break;
        case LUA_TNUMBER:
        case LUA_TBOOLEAN:
            from_lua_num(L, tp, stor, index, dsz, rule);
            return stor;
        case LUA_TSTRING:
            if ((rule == RULE_CAST) ||
                ((tp.type() == ast::C_BUILTIN_PTR) && ((tp.ptr_base().type() == ast::C_BUILTIN_CHAR) || (tp.ptr_base().type() == ast::C_BUILTIN_VOID)) && (tp.ptr_base().cv() & ast::C_CV_CONST))) {
                dsz = sizeof(char const *);
                return &(*static_cast<char const **>(stor) = lua_tostring(L, index));
            }
            fail_convert_tp(L, "string", tp);
            break;
        case LUA_TUSERDATA: {
            if (iscdata(L, index)) {
                auto &cd = *lua::touserdata<cdata>(L, index);
                return from_lua_cdata(L, cd.decl, tp, cd.as_ptr(), stor, dsz, rule);
            }
            auto tpt = tp.type();
            if (tpt == ast::C_BUILTIN_PTR) {
                dsz = sizeof(void *);
                /* special handling for FILE handles */
                void *ud = lua_touserdata(L, index);
                if (luaL_testudata(L, index, LUA_FILEHANDLE)) {
                    FILE **f = static_cast<FILE **>(ud);
                    return &(*static_cast<void **>(stor) = *f);
                }
                /* other userdata: convert to any pointer when
                 * casting, otherwise only to a void pointer
                 */
                if ((rule == RULE_CAST) || (tp.ptr_base().type() == ast::C_BUILTIN_VOID)) {
                    return &(*static_cast<void **>(stor) = ud);
                }
            } else if (tp.is_ref() && (rule == RULE_CAST)) {
                /* when casting we can initialize refs from userdata */
                void *ud = lua_touserdata(L, index);
                if (luaL_testudata(L, index, LUA_FILEHANDLE)) {
                    FILE **f = static_cast<FILE **>(ud);
                    return &(*static_cast<void **>(stor) = *f);
                }
                return &(*static_cast<void **>(stor) = ud);
            }
            /* error in other cases */
            if (isctype(L, index)) {
                fail_convert_tp(L, "ctype", tp);
            } else {
                fail_convert_tp(L, "userdata", tp);
            }
            break;
        }
        case LUA_TLIGHTUSERDATA:
            if (tp.type() == ast::C_BUILTIN_PTR) {
                dsz = sizeof(void *);
                return &(*static_cast<void **>(stor) = lua_touserdata(L, index));
            } else {
                fail_convert_tp(L, "lightuserdata", tp);
            }
            break;
        case LUA_TTABLE:
            /* we can't handle table initializers here because the memory
             * for the new cdata doesn't exist yet by this point, and it's
             * this function that tells the caller how much memory we'll
             * actually need, and return a pointer to copy from...
             *
             * there are only three special cases where initialization from
             * table is supported, and that is ffi.new, assignment to struct
             * or array members of structs or arrays, and global variable
             * assignment, and those are all handled much earlier so this
             * is never reached
             *
             * so here, we just error, as it definitely means a bad case
             */
            fail_convert_tp(L, "table", tp);
            break;
        case LUA_TFUNCTION:
            if (!tp.callable()) {
                fail_convert_tp(L, "function", tp);
            }
            lua_pushvalue(L, index);
            *static_cast<int *>(stor) = luaL_ref(L, LUA_REGISTRYINDEX);
            /* we don't have a value to store */
            return nullptr;
        default:
            fail_convert_tp(L, lua_typename(L, lua_type(L, index)), tp);
            break;
    }
    assert(false);
    return nullptr;
}

static inline void push_init(lua_State *L, int tidx, int iidx) {
    if (!tidx) {
        lua_pushvalue(L, iidx);
    } else {
        lua_rawgeti(L, tidx, iidx);
    }
}

static void from_lua_table(lua_State *L, ast::c_type const &decl, void *stor, std::size_t rsz, int tidx, int sidx, int ninit);

static void from_lua_table(lua_State *L, ast::c_type const &decl, void *stor, std::size_t rsz, int tidx);

static void from_lua_str(lua_State *L, ast::c_type const &decl, void *stor, std::size_t dsz, int idx, std::size_t nelems = 1, std::size_t bsize = 0) {
    std::size_t vsz;
    ffi::scalar_stor_t sv{};
    void const *vp;
    auto *val = static_cast<unsigned char *>(stor);
    /* not a string: let whatever default behavior happen */
    if (lua_type(L, idx) != LUA_TSTRING) {
        goto fallback;
    }
    /* not an array: we can just initialize normally too */
    if (decl.type() != ast::C_BUILTIN_ARRAY) {
        goto fallback;
    }
    /* string value, not byte array: let it fail */
    if (!decl.ptr_base().byte()) {
        goto fallback;
    }
    /* char-like array, string value */
    vp = lua_tolstring(L, idx, &vsz);
    /* add 1 because of null terminator, but use at most the given space */
    vsz = util::min(vsz + 1, dsz);
    goto cloop;
fallback:
    vp = from_lua(L, decl, &sv, idx, vsz, RULE_CONV);
cloop:
    while (nelems) {
        std::memcpy(val, vp, vsz);
        val += bsize;
        --nelems;
    }
}

static void from_lua_table_record(lua_State *L, ast::c_type const &decl, void *stor, std::size_t rsz, int tidx, int sidx, int ninit) {
    auto &sb = decl.record();
    bool uni = sb.is_union();
    auto *val = static_cast<unsigned char *>(stor);
    bool filled = false;
    bool empty = true;
    sb.iter_fields([L, rsz, val, &decl, &sidx, &filled, &empty, &ninit, tidx, uni](char const *fname, ast::c_type const &fld, std::size_t off) {
        empty = false;
        if (tidx && (ninit < 0)) {
            lua_getfield(L, tidx, fname);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                /* only end at flex members; with unions we want to check
                 * if we can init any other field, with structs we want
                 * to continue initializing other fields
                 */
                return fld.flex();
            }
        } else if (ninit) {
            push_init(L, tidx, sidx++);
            --ninit;
        } else {
            /* no more items to initialize */
            return true;
        }
        /* flex array members */
        if (!uni && fld.flex()) {
            /* the size of the struct minus the flex member plus padding */
            std::size_t ssz = decl.alloc_size();
            /* initialize the last part as in array */
            std::size_t asz = rsz - ssz;
            if (lua_istable(L, -1)) {
                from_lua_table(L, fld, &val[ssz], asz, lua_gettop(L));
                lua_pop(L, 1);
            } else if (!ninit) {
                from_lua_table(L, fld, &val[ssz], asz, 0, lua_gettop(L), 1);
                lua_pop(L, 1);
            } else {
                lua_pop(L, 1);
                from_lua_table(L, fld, &val[ssz], asz, tidx, sidx - 1, ninit + 1);
            }
            return true;
        }
        bool elem_struct = (fld.type() == ast::C_BUILTIN_RECORD);
        bool elem_arr = (fld.type() == ast::C_BUILTIN_ARRAY);
        if ((elem_arr || elem_struct) && lua_istable(L, -1)) {
            from_lua_table(L, fld, &val[off], fld.alloc_size(), lua_gettop(L));
        } else {
            from_lua_str(L, fld, &val[off], fld.alloc_size(), -1);
        }
        filled = true;
        lua_pop(L, 1);
        /* with unions we're only ever initializing one field */
        return uni;
    });
    if (empty) {
        return;
    }
    if (uni && !filled) {
        std::memset(stor, 0, rsz);
    }
}

/* this can't be done in from_lua, because when from_lua is called, the
 * memory is not allocated yet... so do it here, as a special case
 */
static void from_lua_table(lua_State *L, ast::c_type const &decl, void *stor, std::size_t rsz, int tidx, int sidx, int ninit) {
    if (decl.type() == ast::C_BUILTIN_RECORD) {
        from_lua_table_record(L, decl, stor, rsz, tidx, sidx, ninit);
        return;
    }

    if (ninit <= 0) {
        std::memset(stor, 0, rsz);
        return;
    }

    auto *val = static_cast<unsigned char *>(stor);
    auto &pb = decl.ptr_base();
    auto bsize = pb.alloc_size();
    auto nelems = rsz / bsize;

    bool base_array = (pb.type() == ast::C_BUILTIN_ARRAY);
    bool base_struct = (pb.type() == ast::C_BUILTIN_RECORD);

    if (!decl.flex()) {
        if (ninit > int(nelems)) {
            luaL_error(L, "too many initializers");
            return;
        } else if (ninit == 1) {
            /* special case: initialize aggregate with a single value */
            push_init(L, tidx, sidx);
            from_lua_str(L, pb, val, bsize, -1, nelems, bsize);
            lua_pop(L, 1);
            return;
        }
    }

    for (int rinit = ninit; rinit; --rinit) {
        if ((base_array || base_struct) && lua_istable(L, -1)) {
            from_lua_table(L, pb, val, bsize, lua_gettop(L));
        } else {
            push_init(L, tidx, sidx++);
            from_lua_str(L, pb, val, bsize, -1);
        }
        val += bsize;
        lua_pop(L, 1);
    }
    if (ninit < int(nelems)) {
        /* fill possible remaining space with zeroes */
        std::memset(val, 0, bsize * (nelems - ninit));
    }
}

static void from_lua_table(lua_State *L, ast::c_type const &decl, void *stor, std::size_t rsz, int tidx) {
    int ninit;
    auto rl = lua_rawlen(L, tidx);
    lua_rawgeti(L, tidx, 0);
    int sidx = 1;
    if (!lua_isnil(L, -1)) {
        ++rl;
        sidx = 0;
    }
    lua_pop(L, 1);
    if (rl > 0) {
        ninit = int(rl);
    } else {
        ninit = -1;
        sidx = -1;
    }
    from_lua_table(L, decl, stor, rsz, tidx, sidx, ninit);
}

/* a unified entrypoint for initializing complex aggregates
 *
 * if false is returned, we're not initializing a complex aggregate,
 * so appropriate steps can be taken according to where this is used
 */
static bool from_lua_aggreg(lua_State *L, ast::c_type const &decl, void *stor, std::size_t msz, int ninit, int idx) {
    /* bail out early */
    if (decl.is_ref() || !ninit) {
        return false;
    }
    switch (decl.type()) {
        case ast::C_BUILTIN_RECORD:
            /* record types are simpler */
            if (ninit > 1) {
                /* multiple initializers are clear, init members */
                from_lua_table(L, decl, stor, msz, 0, idx, ninit);
            } else if (!lua_istable(L, idx)) {
                /* single non-table initializer case */
                if (iscdata(L, idx)) {
                    /* got cdata as initializer */
                    auto &cd = *lua::touserdata<cdata>(L, idx);
                    if (cd.decl.is_same(decl, true, true)) {
                        /* it's a compatible type: do a copy */
                        std::size_t vsz;
                        ffi::scalar_stor_t sv{};
                        auto *vp = from_lua(L, decl, &sv, idx, vsz, RULE_CONV);
                        std::memcpy(stor, vp, msz);
                        return true;
                    }
                }
                /* otherwise, just init members using the single value */
                from_lua_table(L, decl, stor, msz, 0, idx, ninit);
            } else {
                /* table initializer: init members */
                from_lua_table(L, decl, stor, msz, idx);
            }
            return true;
        case ast::C_BUILTIN_ARRAY:
            break;
        default:
            return false;
    }
    /* arrays are more complicated, let's start with the clear
     * case which is multiple initializers, no choices there
     */
    if (ninit > 1) {
        from_lua_table(L, decl, stor, msz, 0, idx, ninit);
        return true;
    }
    /* single string initializer */
    auto carr = decl.ptr_base().byte();
    if (carr && (lua_type(L, idx) == LUA_TSTRING)) {
        from_lua_str(L, decl, stor, msz, idx);
        return true;
    }
    /* single table initializer */
    if (lua_istable(L, idx)) {
        from_lua_table(L, decl, stor, msz, idx);
        return true;
    }
    /* single initializer that is a compatible array
     *
     * VLAs are not allowed for this kind of initialization
     * the other array must have the same size
     */
    if (!decl.vla() && iscdata(L, idx)) {
        auto &cd = *lua::touserdata<cdata>(L, idx);
        if (cd.decl.is_same(decl, true, true) || (carr && cd.decl.ptr_base().byte() && (cd.decl.array_size() == decl.array_size()))) {
            /* exact copy by value */
            std::memcpy(stor, *static_cast<void **>(cd.as_deref_ptr()), msz);
            return true;
        }
    }
    /* a single non-table initializer that is not a compatible array */
    from_lua_table(L, decl, stor, msz, 0, idx, ninit);
    return true;
}

void from_lua(lua_State *L, ast::c_type const &decl, void *stor, int idx) {
    if (decl.cv() & ast::C_CV_CONST) {
        luaL_error(L, "attempt to write to constant location");
    }
    /* attempt aggregate initialization */
    if (!from_lua_aggreg(L, decl, stor, decl.alloc_size(), 1, idx)) {
        /* fall back to regular initialization */
        ffi::scalar_stor_t sv{};
        std::size_t rsz;
        auto *vp = from_lua(L, decl, &sv, idx, rsz, RULE_CONV);
        if (decl.callable() && !vp) {
            make_cdata_func(L, nullptr, decl.function(), decl.type() == ast::C_BUILTIN_PTR, nullptr);
            auto &fd = tocdata(L, -1);
            fd.as<fdata>().cd->fref = util::pun<int>(sv);
            *static_cast<void (**)()>(stor) = fd.as<fdata>().sym;
            lua_pop(L, 1);
        } else {
            std::memcpy(stor, vp, rsz);
        }
    }
}

void get_global(lua_State *L, lib::c_lib const *dl, const char *sname) {
    auto &ds = ast::decl_store::get_main(L);
    auto const *decl = ds.lookup(sname);

    auto tp = ast::c_object_type::INVALID;
    if (decl) {
        tp = decl->obj_type();
    }

    switch (tp) {
        case ast::c_object_type::VARIABLE: {
            auto &var = decl->as<ast::c_variable>();
            void *symp = lib::get_sym(dl, L, var.sym());
            if (var.type().type() == ast::C_BUILTIN_FUNC) {
                make_cdata_func(L, util::pun<void (*)()>(symp), var.type().function(), false, nullptr);
            } else {
                to_lua(L, var.type(), symp, RULE_RET, false);
            }
            return;
        }
        case ast::c_object_type::CONSTANT: {
            auto &cd = decl->as<ast::c_constant>();
            to_lua(L, cd.type(), &cd.value(), RULE_RET, false);
            return;
        }
        default:
            luaL_error(L, "missing declaration for symbol '%s'", sname);
            return;
    }
}

void set_global(lua_State *L, lib::c_lib const *dl, char const *sname, int idx) {
    auto &ds = ast::decl_store::get_main(L);
    auto const *decl = ds.lookup(sname);
    if (!decl) {
        luaL_error(L, "missing declaration for symbol '%s'", sname);
        return;
    }
    if (decl->obj_type() != ast::c_object_type::VARIABLE) {
        luaL_error(L, "symbol '%s' is not mutable", decl->name());
    }
    auto &cv = decl->as<ast::c_variable>();
    if (cv.type().type() == ast::C_BUILTIN_FUNC) {
        luaL_error(L, "symbol '%s' is not mutable", decl->name());
    }
    from_lua(L, cv.type(), lib::get_sym(dl, L, cv.sym()), idx);
}

void make_cdata(lua_State *L, ast::c_type const &decl, int rule, int idx) {
    switch (decl.type()) {
        case ast::C_BUILTIN_FUNC:
            luaL_error(L, "invalid C type");
            break;
        default:
            break;
    }
    ffi::scalar_stor_t stor{};
    void *cdp = nullptr;
    std::size_t rsz = 0, narr = 0;
    int iidx = idx, ninits;
    if (rule == RULE_CAST) {
        goto definit;
    }
    if (decl.type() == ast::C_BUILTIN_ARRAY) {
        if (decl.vla()) {
            auto arrs = luaL_checkinteger(L, idx);
            if (arrs < 0) {
                luaL_error(L, "size of C type is unknown");
            }
            ++iidx;
            ninits = lua_gettop(L) - iidx + 1;
            narr = std::size_t(arrs);
            rsz = decl.ptr_base().alloc_size() * narr;
            /* see below */
            rsz += sizeof(ffi::scalar_stor_t);
            goto newdata;
        } else if (decl.flex()) {
            luaL_error(L, "size of C type is unknown");
        }
        ninits = lua_gettop(L) - iidx + 1;
        narr = decl.array_size();
        rsz = decl.ptr_base().alloc_size() * narr;
        /* owned arrays consist of an ffi::scalar_stor_t part, which is an
         * ffi::scalar_stor_t because that has the greatest alignment of all
         * scalars and thus is good enough to follow up with any type after
         * that, and the array part; the ffi::scalar_stor_t part contains a
         * pointer to the array part right in the beginning, so we can freely
         * cast between any array and a pointer, even an owned one
         */
        rsz += sizeof(ffi::scalar_stor_t);
        goto newdata;
    } else if (decl.type() == ast::C_BUILTIN_RECORD) {
        ast::c_type const *lf = nullptr;
        if (decl.record().flexible(&lf)) {
            auto arrs = luaL_checkinteger(L, idx);
            if (arrs < 0) {
                luaL_error(L, "size of C type is unknown");
            }
            ++iidx;
            ninits = lua_gettop(L) - iidx + 1;
            rsz = decl.alloc_size() + (std::size_t(arrs) * lf->ptr_base().alloc_size());
            goto newdata;
        }
        ninits = lua_gettop(L) - iidx + 1;
        rsz = decl.alloc_size();
        goto newdata;
    }
definit:
    ninits = lua_gettop(L) - iidx + 1;
    if (ninits > 1) {
        luaL_error(L, "too many initializers");
    } else if (ninits == 1) {
        cdp = from_lua(L, decl, &stor, idx, rsz, rule);
    } else {
        rsz = decl.alloc_size();
    }
newdata:
    if (decl.callable()) {
        closure_data *cd = nullptr;
        if (cdp && iscdata(L, idx)) {
            /* special handling for closures */
            auto &fcd = tocdata(L, idx);
            if (fcd.decl.closure()) {
                cd = fcd.as<fdata>().cd;
                cdp = nullptr;
            }
        }
        void (*symp)() = nullptr;
        if (cdp) {
            std::memcpy(&symp, cdp, sizeof(symp));
        }
        make_cdata_func(L, symp, decl.function(), decl.type() == ast::C_BUILTIN_PTR, cd);
        if (!cdp && !cd) {
            tocdata(L, -1).as<fdata>().cd->fref = util::pun<int>(stor);
        }
    } else {
        auto &cd = newcdata(L, decl, rsz);
        void *dptr = nullptr;
        std::size_t msz = rsz;
        if (!cdp) {
            std::memset(cd.as_ptr(), 0, rsz);
            if (decl.type() == ast::C_BUILTIN_ARRAY) {
                auto *bval = static_cast<unsigned char *>(cd.as_ptr());
                dptr = bval + sizeof(ffi::scalar_stor_t);
                cd.as<void *>() = dptr;
                msz = rsz - sizeof(ffi::scalar_stor_t);
            } else {
                dptr = cd.as_ptr();
            }
        } else if (decl.type() == ast::C_BUILTIN_ARRAY) {
            std::size_t esz = (rsz - sizeof(ffi::scalar_stor_t)) / narr;
            /* the base of the alloated block */
            auto *bval = static_cast<unsigned char *>(cd.as_ptr());
            /* the array memory begins after the first ffi::scalar_stor_t */
            auto *val = bval + sizeof(ffi::scalar_stor_t);
            dptr = val;
            /* we can treat an array like a pointer, always */
            cd.as<void *>() = dptr;
            /* write initializers into the array part */
            for (std::size_t i = 0; i < narr; ++i) {
                std::memcpy(&val[i * esz], cdp, esz);
            }
            msz = rsz - sizeof(ffi::scalar_stor_t);
        } else {
            dptr = cd.as_ptr();
            std::memcpy(dptr, cdp, rsz);
        }
        /* perform aggregate initialization */
        from_lua_aggreg(L, decl, dptr, msz, ninits, iidx);
        /* set a gc finalizer if provided in metatype */
        if (decl.type() == ast::C_BUILTIN_RECORD) {
            int mf;
            int mt = decl.record().metatype(mf);
            if (mf & METATYPE_FLAG_GC) {
                if (metatype_getfield(L, mt, "__gc")) {
                    cd.gc_ref = luaL_ref(L, LUA_REGISTRYINDEX);
                }
            }
        }
    }
}

} /* namespace ffi */

namespace ast {

/* This unholy spaghetti implements integer promotions as well as conversion
 * rules in arithmetic operations etc. as the C standard defines it...
 */

static void promote_int(c_value &v, c_expr_type &et) {
    switch (et) {
        case c_expr_type::BOOL:
            v.i = int(v.b);
            et = c_expr_type::INT;
            break;
        case c_expr_type::CHAR:
            v.i = int(v.c);
            et = c_expr_type::INT;
            break;
        default:
            break;
    }
}

/* only integers have ranks but for our purposes this is fine */
static int get_rank(lua_State *L, c_expr_type type) WARN_UNUSED_RET;
static int get_rank(lua_State *L, c_expr_type type) {
    switch (type) {
        case c_expr_type::INT:
            return 0;
        case c_expr_type::UINT:
            return 0;
        case c_expr_type::LONG:
            return 1;
        case c_expr_type::ULONG:
            return 1;
        case c_expr_type::LLONG:
            return 2;
        case c_expr_type::ULLONG:
            return 2;
        case c_expr_type::FLOAT:
            return 3;
        case c_expr_type::DOUBLE:
            return 4;
        case c_expr_type::LDOUBLE:
            return 5;
        default:
            lua_pushliteral(L, "invalid type for operand");
            break;
    }
    return -1;
}

static bool is_signed(c_expr_type type) WARN_UNUSED_RET;
static bool is_signed(c_expr_type type) {
    switch (type) {
        case c_expr_type::CHAR:
            return util::is_signed<char>::value;
        case c_expr_type::INT:
        case c_expr_type::LONG:
        case c_expr_type::LLONG:
        case c_expr_type::FLOAT:
        case c_expr_type::DOUBLE:
        case c_expr_type::LDOUBLE:
            return true;
        default:
            break;
    }
    return false;
}

static bool convert_bin(lua_State *L, c_value &lval, c_expr_type &let, c_value &rval, c_expr_type &ret) WARN_UNUSED_RET;

static bool convert_bin(lua_State *L, c_value &lval, c_expr_type &let, c_value &rval, c_expr_type &ret) {
    /* same types, likely case, bail out early */
    if (let == ret) {
        return true;
    }

    int lrank = get_rank(L, let);
    int rrank = get_rank(L, ret);
    if ((lrank < 0) || (rrank < 0)) {
        return false;
    }

    /* if right operand is higher ranked, treat it as left operand */
    if (rrank > lrank) {
        return convert_bin(L, rval, ret, lval, let);
    }

#define CONVERT_RVAL(lv)                \
    {                                   \
        using LT = decltype(rval.lv);   \
        switch (ret) {                  \
            case c_expr_type::DOUBLE:   \
                rval.lv = LT(rval.d);   \
                break;                  \
            case c_expr_type::FLOAT:    \
                rval.lv = LT(rval.f);   \
                break;                  \
            case c_expr_type::INT:      \
                rval.lv = LT(rval.i);   \
                break;                  \
            case c_expr_type::UINT:     \
                rval.lv = LT(rval.u);   \
                break;                  \
            case c_expr_type::LONG:     \
                rval.lv = LT(rval.l);   \
                break;                  \
            case c_expr_type::ULONG:    \
                rval.lv = LT(rval.ul);  \
                break;                  \
            case c_expr_type::LLONG:    \
                rval.lv = LT(rval.ll);  \
                break;                  \
            case c_expr_type::ULLONG:   \
                rval.lv = LT(rval.ull); \
                break;                  \
            default:                    \
                break;                  \
        }                               \
    }

    /* at this point it's guaranteed that left rank is higer or equal */

    /* left operand is a float, convert to it */
    switch (let) {
        case c_expr_type::LDOUBLE:
            CONVERT_RVAL(ld);
            ret = let;
            return true;
        case c_expr_type::DOUBLE:
            CONVERT_RVAL(d);
            ret = let;
            return true;
        case c_expr_type::FLOAT:
            CONVERT_RVAL(f);
            ret = let;
            return true;
        default:
            break;
    }

    /* both operands are integers */

    bool lsig = is_signed(let);
    bool rsig = is_signed(ret);

    /* same signedness, convert lower ranked type to higher ranked */
    if (lsig == rsig) {
        switch (let) {
            case c_expr_type::ULLONG:
                CONVERT_RVAL(ull);
                ret = let;
                return true;
            case c_expr_type::LLONG:
                CONVERT_RVAL(ll);
                ret = let;
                return true;
            case c_expr_type::ULONG:
                CONVERT_RVAL(ul);
                ret = let;
                return true;
            case c_expr_type::LONG:
                CONVERT_RVAL(l);
                ret = let;
                return true;
            case c_expr_type::UINT:
                CONVERT_RVAL(u);
                ret = let;
                return true;
            case c_expr_type::INT:
                CONVERT_RVAL(i);
                ret = let;
                return true;
            default:
                break;
        }
        LUA_BUG_MSG(L, "unreachable code");
        return false;
    }

    /* unsigned type has greater or equal rank */
    if (rsig) {
        switch (let) {
            case c_expr_type::ULLONG:
                CONVERT_RVAL(ull);
                ret = let;
                return true;
            case c_expr_type::ULONG:
                CONVERT_RVAL(ul);
                ret = let;
                return true;
            case c_expr_type::UINT:
                CONVERT_RVAL(u);
                ret = let;
                return true;
            default:
                break;
        }
        LUA_BUG_MSG(L, "unreachable code");
        return false;
    }

#define CONVERT_RVAL_BOUNDED(lv)                           \
    switch (ret) {                                         \
        case c_expr_type::ULONG:                           \
            if (sizeof(unsigned long) < sizeof(lval.lv)) { \
                rval.lv = rval.ul;                         \
                ret = let;                                 \
                return true;                               \
            }                                              \
            break;                                         \
        case c_expr_type::UINT:                            \
            if (sizeof(unsigned int) < sizeof(lval.lv)) {  \
                rval.lv = rval.u;                          \
                ret = let;                                 \
                return true;                               \
            }                                              \
            break;                                         \
        default:                                           \
            break;                                         \
    }

    /* at this point left operand is always signed */

    /* try to fit right operand into it (may work if left has greater rank) */
    switch (let) {
        case c_expr_type::LLONG:
            CONVERT_RVAL_BOUNDED(ll);
            break;
        case c_expr_type::LONG:
            CONVERT_RVAL_BOUNDED(l);
            break;
        case c_expr_type::INT:
            break;
        default:
            break;
    }

#undef CONVERT_RVAL_BOUNDED

    /* does not fit; in that case, convert both to unsigned version of left */
    switch (let) {
        case c_expr_type::LLONG:
            lval.ull = lval.ll;
            CONVERT_RVAL(ull);
            let = ret = c_expr_type::ULLONG;
            return true;
        case c_expr_type::LONG:
            lval.ul = lval.l;
            CONVERT_RVAL(ul);
            let = ret = c_expr_type::ULONG;
            return true;
        case c_expr_type::INT:
            lval.u = lval.i;
            CONVERT_RVAL(u);
            let = ret = c_expr_type::UINT;
            return true;
        default:
            break;
    }

#undef CONVERT_RVAL

    LUA_BUG_MSG(L, "unreachable code");
    return false;
}

static bool eval_unary(lua_State *L, c_value &baseval, c_expr const &e, c_expr_type &et) WARN_UNUSED_RET;

static bool eval_unary(lua_State *L, c_value &baseval, c_expr const &e, c_expr_type &et) {
    if (!e.un.expr->eval(L, baseval, et, false)) {
        return false;
    }
    switch (e.un.op) {
        case c_expr_unop::UNP:
            promote_int(baseval, et);
            switch (et) {
                case c_expr_type::INT:
                case c_expr_type::UINT:
                case c_expr_type::LONG:
                case c_expr_type::ULONG:
                case c_expr_type::LLONG:
                case c_expr_type::ULLONG:
                    return true;
                default:
                    break;
            }
            lua_pushliteral(L, "invalid type for +(expr)");
            return false;
        case c_expr_unop::UNM:
            promote_int(baseval, et);
            switch (et) {
                case c_expr_type::INT:
                    baseval.i = -baseval.i;
                    return true;
                case c_expr_type::UINT:
                    if (baseval.u) {
                        baseval.u = UINT_MAX - baseval.u + 1;
                    }
                    return true;
                case c_expr_type::LONG:
                    baseval.l = -baseval.l;
                    return true;
                case c_expr_type::ULONG:
                    if (baseval.ul) {
                        baseval.ul = ULONG_MAX - baseval.ul + 1;
                    }
                    return true;
                case c_expr_type::LLONG:
                    baseval.ll = -baseval.ll;
                    return true;
                case c_expr_type::ULLONG:
                    if (baseval.ull) {
                        baseval.ull = ULLONG_MAX - baseval.ull + 1;
                    }
                    return true;
                default:
                    break;
            }
            lua_pushliteral(L, "invalid type for -(expr)");
            return false;
        case c_expr_unop::NOT:
            switch (et) {
                case c_expr_type::BOOL:
                    baseval.b = !baseval.b;
                    return true;
                case c_expr_type::CHAR:
                    baseval.c = !baseval.c;
                    return true;
                case c_expr_type::INT:
                    baseval.i = !baseval.i;
                    return true;
                case c_expr_type::UINT:
                    baseval.u = !baseval.u;
                    return true;
                case c_expr_type::LONG:
                    baseval.l = !baseval.l;
                    return true;
                case c_expr_type::ULONG:
                    baseval.ul = !baseval.ul;
                    return true;
                case c_expr_type::LLONG:
                    baseval.ll = !baseval.ll;
                    return true;
                case c_expr_type::ULLONG:
                    baseval.ull = !baseval.ull;
                    return true;
                default:
                    break;
            }
            lua_pushliteral(L, "invalid type for !(expr)");
            return false;
        case c_expr_unop::BNOT:
            promote_int(baseval, et);
            switch (et) {
                case c_expr_type::INT:
                    baseval.i = ~baseval.i;
                    return true;
                case c_expr_type::UINT:
                    baseval.u = ~baseval.u;
                    return true;
                case c_expr_type::LONG:
                    baseval.l = ~baseval.l;
                    return true;
                case c_expr_type::ULONG:
                    baseval.ul = ~baseval.ul;
                    return true;
                case c_expr_type::LLONG:
                    baseval.ll = ~baseval.ll;
                    return true;
                case c_expr_type::ULLONG:
                    baseval.ull = ~baseval.ull;
                    return true;
                default:
                    break;
            }
            lua_pushliteral(L, "invalid type for ~(expr)");
            return false;
        default:
            break;
    }
    LUA_BUG_MSG(L, "unreachable code");
    return false;
}

static bool eval_binary(lua_State *L, c_value &retv, c_expr const &e, c_expr_type &et) WARN_UNUSED_RET;

static bool eval_binary(lua_State *L, c_value &retv, c_expr const &e, c_expr_type &et) {
    c_expr_type let, ret;
    c_value lval, rval;
    if (!e.bin.lhs->eval(L, lval, let, false)) {
        return false;
    }
    if (!e.bin.rhs->eval(L, rval, ret, false)) {
        return false;
    }

#define BINOP_CASE(opn, op)                                                      \
    case c_expr_binop::opn:                                                      \
        promote_int(lval, let);                                                  \
        promote_int(rval, ret);                                                  \
        if (!convert_bin(L, lval, let, rval, ret)) {                             \
            return false;                                                        \
        }                                                                        \
        et = let;                                                                \
        switch (let) {                                                           \
            case c_expr_type::INT:                                               \
                retv.i = lval.i op rval.i;                                       \
                break;                                                           \
            case c_expr_type::UINT:                                              \
                retv.u = lval.u op rval.u;                                       \
                break;                                                           \
            case c_expr_type::LONG:                                              \
                retv.l = lval.l op rval.l;                                       \
                break;                                                           \
            case c_expr_type::ULONG:                                             \
                retv.ul = lval.ul op rval.ul;                                    \
                break;                                                           \
            case c_expr_type::LLONG:                                             \
                retv.ll = lval.ll op rval.ll;                                    \
                break;                                                           \
            case c_expr_type::ULLONG:                                            \
                retv.ull = lval.ull op rval.ull;                                 \
                break;                                                           \
            case c_expr_type::FLOAT:                                             \
                retv.f = lval.f op rval.f;                                       \
                break;                                                           \
            case c_expr_type::DOUBLE:                                            \
                retv.d = lval.d op rval.d;                                       \
                break;                                                           \
            case c_expr_type::LDOUBLE:                                           \
                retv.ld = lval.ld op rval.ld;                                    \
                break;                                                           \
            default:                                                             \
                lua_pushliteral(L, "invalid type(s) for (expr1 " #op " expr2)"); \
                return false;                                                    \
        }                                                                        \
        break;

#define CMP_BOOL_CASE(opn, op)                                                   \
    case c_expr_binop::opn:                                                      \
        promote_int(lval, let);                                                  \
        promote_int(rval, ret);                                                  \
        if (!convert_bin(L, lval, let, rval, ret)) {                             \
            return false;                                                        \
        }                                                                        \
        et = c_expr_type::BOOL;                                                  \
        switch (let) {                                                           \
            case c_expr_type::INT:                                               \
                retv.b = lval.i op rval.i;                                       \
                break;                                                           \
            case c_expr_type::UINT:                                              \
                retv.b = lval.u op rval.u;                                       \
                break;                                                           \
            case c_expr_type::LONG:                                              \
                retv.b = lval.l op rval.l;                                       \
                break;                                                           \
            case c_expr_type::ULONG:                                             \
                retv.b = lval.ul op rval.ul;                                     \
                break;                                                           \
            case c_expr_type::LLONG:                                             \
                retv.b = lval.ll op rval.ll;                                     \
                break;                                                           \
            case c_expr_type::ULLONG:                                            \
                retv.b = lval.ull op rval.ull;                                   \
                break;                                                           \
            case c_expr_type::FLOAT:                                             \
                retv.b = lval.f op rval.f;                                       \
                break;                                                           \
            case c_expr_type::DOUBLE:                                            \
                retv.b = lval.d op rval.d;                                       \
                break;                                                           \
            case c_expr_type::LDOUBLE:                                           \
                retv.b = lval.ld op rval.ld;                                     \
                break;                                                           \
            default:                                                             \
                lua_pushliteral(L, "invalid type(s) for (expr1 " #op " expr2)"); \
                return false;                                                    \
        }                                                                        \
        break;

#define BINOP_CASE_NOFLT(opn, op)                                                                 \
    case c_expr_binop::opn:                                                                       \
        promote_int(lval, let);                                                                   \
        promote_int(rval, ret);                                                                   \
        if (!convert_bin(L, lval, let, rval, ret)) {                                              \
            return false;                                                                         \
        }                                                                                         \
        et = let;                                                                                 \
        switch (let) {                                                                            \
            case c_expr_type::INT:                                                                \
                retv.i = lval.i op rval.i;                                                        \
                break;                                                                            \
            case c_expr_type::UINT:                                                               \
                retv.u = lval.u op rval.u;                                                        \
                break;                                                                            \
            case c_expr_type::LONG:                                                               \
                retv.l = lval.l op rval.l;                                                        \
                break;                                                                            \
            case c_expr_type::ULONG:                                                              \
                retv.ul = lval.ul op rval.ul;                                                     \
                break;                                                                            \
            case c_expr_type::LLONG:                                                              \
                retv.ll = lval.ll op rval.ll;                                                     \
                break;                                                                            \
            case c_expr_type::ULLONG:                                                             \
                retv.ull = lval.ull op rval.ull;                                                  \
                break;                                                                            \
            case c_expr_type::FLOAT:                                                              \
            case c_expr_type::DOUBLE:                                                             \
            case c_expr_type::LDOUBLE:                                                            \
                lua_pushliteral(L, "operator " #op " cannot be applied to floating point types"); \
                return false;                                                                     \
            default:                                                                              \
                lua_pushliteral(L, "invalid type(s) for (expr1 " #op " expr2)");                  \
                return false;                                                                     \
        }                                                                                         \
        break;

#define SHIFT_CASE_INNER(fn, op, nop)                                        \
    /* shift by negative number is undefined in C, so define it as */        \
    /* shifting in the other direction; this works like lua 5.3    */        \
    switch (ret) {                                                           \
        case c_expr_type::INT:                                               \
            if (rval.i < 0) {                                                \
                retv.fn = lval.fn nop - rval.i;                              \
            } else {                                                         \
                retv.fn = lval.fn op rval.i;                                 \
            }                                                                \
            break;                                                           \
        case c_expr_type::UINT:                                              \
            retv.fn = lval.fn op rval.u;                                     \
            break;                                                           \
        case c_expr_type::LONG:                                              \
            if (rval.l < 0) {                                                \
                retv.fn = lval.fn nop - rval.l;                              \
            } else {                                                         \
                retv.fn = lval.fn op rval.l;                                 \
            }                                                                \
            break;                                                           \
        case c_expr_type::ULONG:                                             \
            retv.fn = lval.fn op rval.u;                                     \
            break;                                                           \
        case c_expr_type::LLONG:                                             \
            if (rval.ll < 0) {                                               \
                retv.fn = lval.fn nop - rval.ll;                             \
            } else {                                                         \
                retv.fn = lval.fn op rval.ll;                                \
            }                                                                \
            break;                                                           \
        case c_expr_type::ULLONG:                                            \
            retv.fn = lval.fn op rval.ull;                                   \
            break;                                                           \
        default:                                                             \
            lua_pushliteral(L, "invalid type(s) for (expr1 " #op " expr2)"); \
            return false;                                                    \
    }

#define SHIFT_CASE(opn, op, nop)                                                 \
    case c_expr_binop::opn:                                                      \
        promote_int(lval, let);                                                  \
        promote_int(rval, ret);                                                  \
        et = let;                                                                \
        switch (let) {                                                           \
            case c_expr_type::INT:                                               \
                SHIFT_CASE_INNER(i, op, nop);                                    \
                break;                                                           \
            case c_expr_type::UINT:                                              \
                SHIFT_CASE_INNER(u, op, nop);                                    \
                break;                                                           \
            case c_expr_type::LONG:                                              \
                SHIFT_CASE_INNER(l, op, nop);                                    \
                break;                                                           \
            case c_expr_type::ULONG:                                             \
                SHIFT_CASE_INNER(ul, op, nop);                                   \
                break;                                                           \
            case c_expr_type::LLONG:                                             \
                SHIFT_CASE_INNER(ll, op, nop);                                   \
                break;                                                           \
            case c_expr_type::ULLONG:                                            \
                SHIFT_CASE_INNER(ull, op, nop);                                  \
                break;                                                           \
            default:                                                             \
                lua_pushliteral(L, "invalid type(s) for (expr1 " #op " expr2)"); \
                return false;                                                    \
        }                                                                        \
        break;

#define BOOL_CASE_INNER(lv, op)                                              \
    switch (ret) {                                                           \
        case c_expr_type::INT:                                               \
            retv.b = lv op rval.i;                                           \
            break;                                                           \
        case c_expr_type::UINT:                                              \
            retv.b = lv op rval.u;                                           \
            break;                                                           \
        case c_expr_type::LONG:                                              \
            retv.b = lv op rval.l;                                           \
            break;                                                           \
        case c_expr_type::ULONG:                                             \
            retv.b = lv op rval.ul;                                          \
            break;                                                           \
        case c_expr_type::LLONG:                                             \
            retv.b = lv op rval.ll;                                          \
            break;                                                           \
        case c_expr_type::ULLONG:                                            \
            retv.b = lv op rval.ull;                                         \
            break;                                                           \
        case c_expr_type::FLOAT:                                             \
            retv.b = lv op rval.f;                                           \
            break;                                                           \
        case c_expr_type::DOUBLE:                                            \
            retv.b = lv op rval.d;                                           \
            break;                                                           \
        case c_expr_type::LDOUBLE:                                           \
            retv.b = lv op rval.ld;                                          \
            break;                                                           \
        case c_expr_type::STRING:                                            \
            retv.b = lv op true;                                             \
            break;                                                           \
        case c_expr_type::CHAR:                                              \
            retv.b = lv op rval.c;                                           \
            break;                                                           \
        case c_expr_type::NULLPTR:                                           \
            retv.b = lv op nullptr;                                          \
            break;                                                           \
        case c_expr_type::BOOL:                                              \
            retv.b = lv op rval.b;                                           \
            break;                                                           \
        default:                                                             \
            lua_pushliteral(L, "invalid type(s) for (expr1 " #op " expr2)"); \
            return false;                                                    \
    }

#define BOOL_CASE(opn, op)                                                       \
    case c_expr_binop::opn:                                                      \
        et = c_expr_type::BOOL;                                                  \
        switch (let) {                                                           \
            case c_expr_type::INT:                                               \
                BOOL_CASE_INNER(lval.i, op);                                     \
                break;                                                           \
            case c_expr_type::UINT:                                              \
                BOOL_CASE_INNER(lval.u, op);                                     \
                break;                                                           \
            case c_expr_type::LONG:                                              \
                BOOL_CASE_INNER(lval.l, op);                                     \
                break;                                                           \
            case c_expr_type::ULONG:                                             \
                BOOL_CASE_INNER(lval.ul, op);                                    \
                break;                                                           \
            case c_expr_type::LLONG:                                             \
                BOOL_CASE_INNER(lval.ll, op);                                    \
                break;                                                           \
            case c_expr_type::ULLONG:                                            \
                BOOL_CASE_INNER(lval.ull, op);                                   \
                break;                                                           \
            case c_expr_type::FLOAT:                                             \
                BOOL_CASE_INNER(lval.f, op);                                     \
                break;                                                           \
            case c_expr_type::DOUBLE:                                            \
                BOOL_CASE_INNER(lval.d, op);                                     \
                break;                                                           \
            case c_expr_type::LDOUBLE:                                           \
                BOOL_CASE_INNER(lval.ld, op);                                    \
                break;                                                           \
            case c_expr_type::STRING:                                            \
                BOOL_CASE_INNER(true, op);                                       \
                break;                                                           \
            case c_expr_type::CHAR:                                              \
                BOOL_CASE_INNER(lval.c, op);                                     \
                break;                                                           \
            case c_expr_type::NULLPTR:                                           \
                BOOL_CASE_INNER(nullptr, op);                                    \
                break;                                                           \
            case c_expr_type::BOOL:                                              \
                BOOL_CASE_INNER(lval.b, op);                                     \
                break;                                                           \
            default:                                                             \
                lua_pushliteral(L, "invalid type(s) for (expr1 " #op " expr2)"); \
                return false;                                                    \
        }                                                                        \
        break;

    switch (e.bin.op) {
        BINOP_CASE(ADD, +)
        BINOP_CASE(SUB, -)
        BINOP_CASE(MUL, *)
        BINOP_CASE(DIV, /)
        BINOP_CASE_NOFLT(MOD, %)

        CMP_BOOL_CASE(EQ, ==)
        CMP_BOOL_CASE(NEQ, !=)
        CMP_BOOL_CASE(GT, >)
        CMP_BOOL_CASE(LT, <)
        CMP_BOOL_CASE(GE, >=)
        CMP_BOOL_CASE(LE, <=)

        BOOL_CASE(AND, &&)
        BOOL_CASE(OR, ||)

        BINOP_CASE_NOFLT(BAND, &)
        BINOP_CASE_NOFLT(BOR, |)
        BINOP_CASE_NOFLT(BXOR, ^)
        SHIFT_CASE(LSH, <<, >>)
        SHIFT_CASE(RSH, >>, <<)

        default:
            LUA_BUG_MSG(L, "unhandled operator ");
            lua_pushfstring(L, "%d", int(e.bin.op));
            lua_concat(L, 2);
            return false;
    }

#undef BOOL_CASE
#undef BOOL_CASE_INNER
#undef SHIFT_CASE
#undef SHIFT_CASE_INNER
#undef CMP_BOOL_CASE
#undef BINOP_CASE

    return true;
}

static bool eval_ternary(lua_State *L, c_value &ret, c_expr const &e, c_expr_type &et) WARN_UNUSED_RET;

static bool eval_ternary(lua_State *L, c_value &ret, c_expr const &e, c_expr_type &et) {
    c_expr_type cet;
    c_value cval;
    if (!e.tern.cond->eval(L, cval, cet, false)) {
        return false;
    }
    bool tval = false;
    switch (cet) {
        case c_expr_type::INT:
            tval = cval.i;
            break;
        case c_expr_type::UINT:
            tval = cval.u;
            break;
        case c_expr_type::LONG:
            tval = cval.l;
            break;
        case c_expr_type::ULONG:
            tval = cval.ul;
            break;
        case c_expr_type::LLONG:
            tval = cval.ll;
            break;
        case c_expr_type::ULLONG:
            tval = cval.ull;
            break;
        case c_expr_type::FLOAT:
            tval = cval.f;
            break;
        case c_expr_type::DOUBLE:
            tval = cval.d;
            break;
        case c_expr_type::LDOUBLE:
            tval = cval.ld;
            break;
        case c_expr_type::STRING:
            tval = true;
            break;
        case c_expr_type::CHAR:
            tval = cval.c;
            break;
        case c_expr_type::NULLPTR:
            tval = false;
            break;
        case c_expr_type::BOOL:
            tval = cval.b;
            break;
        default:
            lua_pushliteral(L, "invalid ternary condition");
            return false;
    }
    if (tval) {
        return e.tern.texpr->eval(L, ret, et, true);
    }
    return e.tern.fexpr->eval(L, ret, et, true);
}

static bool c_expr_eval(lua_State *L, c_value &ret, c_expr const &ce, c_expr_type &et, bool promote) WARN_UNUSED_RET;

static bool c_expr_eval(lua_State *L, c_value &ret, c_expr const &ce, c_expr_type &et, bool promote) {
    switch (ce.type()) {
        case c_expr_type::BINARY:
            return eval_binary(L, ret, ce, et);
        case c_expr_type::UNARY:
            return eval_unary(L, ret, ce, et);
        case c_expr_type::TERNARY:
            return eval_ternary(L, ret, ce, et);
        case c_expr_type::INT:
            ret.i = ce.val.i;
            et = c_expr_type::INT;
            break;
        case c_expr_type::UINT:
            ret.u = ce.val.u;
            et = c_expr_type::UINT;
            break;
        case c_expr_type::LONG:
            ret.l = ce.val.l;
            et = c_expr_type::LONG;
            break;
        case c_expr_type::ULONG:
            ret.ul = ce.val.ul;
            et = c_expr_type::ULONG;
            break;
        case c_expr_type::LLONG:
            ret.ll = ce.val.ll;
            et = c_expr_type::LLONG;
            break;
        case c_expr_type::ULLONG:
            ret.ull = ce.val.ull;
            et = c_expr_type::ULLONG;
            break;
        case c_expr_type::FLOAT:
            ret.f = ce.val.f;
            et = c_expr_type::FLOAT;
            break;
        case c_expr_type::DOUBLE:
            ret.d = ce.val.d;
            et = c_expr_type::DOUBLE;
            break;
        case c_expr_type::CHAR:
            ret.c = ce.val.c;
            et = c_expr_type::CHAR;
            break;
        case c_expr_type::BOOL:
            ret.b = ce.val.b;
            et = c_expr_type::BOOL;
            break;
        default:
            ret.i = 0;
            et = c_expr_type::INVALID;
            lua_pushliteral(L, "invalid expression type");
            return false;
    }
    if (promote) {
        promote_int(ret, et);
    }
    return true;
}

bool c_expr::eval(lua_State *L, c_value &v, c_expr_type &et, bool promote) const {
    /* clear first */
    v = c_value{};
    return c_expr_eval(L, v, *this, et, promote);
}

c_object::~c_object() {}

/* params ignore continuation func */
void c_param::do_serialize(util::strbuf &o, c_object_cont_f, void *) const {
    p_type.do_serialize(
            o,
            [](util::strbuf &out, void *data) {
                auto &p = *static_cast<c_param const *>(data);
                if (!p.p_name.empty()) {
                    if (out.back() != '*') {
                        out.append(' ');
                    }
                    out.append(p.p_name);
                }
            },
            const_cast<c_param *>(this));
}

void c_function::do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const {
    using D = struct {
        c_object_cont_f cont;
        void *data;
    };
    D val{cont, data};
    p_result.do_serialize(
            o,
            [](util::strbuf &out, void *idata) {
                D &d = *static_cast<D *>(idata);
                /* if cont is nullptr, we still need this for the final () anyway */
                if ((out.back() != '&') && (out.back() != '*') && (out.back() != ']') && (out.back() != ')') && (out.back() != '(')) {
                    out.append(' ');
                }
                if (d.cont) {
                    out.append('(');
                    auto sz = out.size();
                    d.cont(out, d.data);
                    if (sz == out.size()) {
                        out.pop_back();
                    } else {
                        out.append(')');
                    }
                }
            },
            &val);
    o.append("()");
}

void c_type::clear() {
    int tp = type();
    if (tp == C_BUILTIN_FUNC) {
        using T = util::rc_obj<c_function>;
        p_func.~T();
    } else if ((tp == C_BUILTIN_PTR) || (tp == C_BUILTIN_ARRAY)) {
        using T = util::rc_obj<c_type>;
        p_ptr.~T();
    }
}

void c_type::copy(c_type const &v) {
    p_asize = v.p_asize;
    p_ttype = v.p_ttype;
    p_flags = v.p_flags;
    p_cv = v.p_cv;

    int tp = type();
    if (tp == C_BUILTIN_FUNC) {
        new (&p_func) util::rc_obj<c_function>{v.p_func};
    } else if ((tp == C_BUILTIN_PTR) || (tp == C_BUILTIN_ARRAY)) {
        new (&p_ptr) util::rc_obj<c_type>{v.p_ptr};
    } else if ((tp == C_BUILTIN_RECORD) || (tp == C_BUILTIN_ENUM)) {
        p_crec = v.p_crec;
    }
}

c_type::c_type(c_type &&v) : p_asize{v.p_asize}, p_ttype{v.p_ttype}, p_flags{v.p_flags}, p_cv{v.p_cv} {
    v.p_ttype = C_BUILTIN_INVALID;
    v.p_flags = 0;
    v.p_cv = 0;
    auto tp = type();
    if ((tp == C_BUILTIN_PTR) || (tp == C_BUILTIN_ARRAY)) {
        using T = util::rc_obj<c_type>;
        new (&p_ptr) T{v.p_ptr};
        v.p_ptr.~T();
    } else if (tp == C_BUILTIN_FUNC) {
        using T = util::rc_obj<c_function>;
        new (&p_func) T{v.p_func};
        v.p_func.~T();
    } else {
        p_crec = v.p_crec;
        v.p_crec = nullptr;
    }
}

c_type &c_type::operator=(c_type &&v) {
    clear();
    p_asize = v.p_asize;
    p_ttype = v.p_ttype;
    p_flags = v.p_flags;
    p_cv = v.p_cv;
    v.p_ttype = C_BUILTIN_INVALID;
    v.p_flags = 0;
    v.p_cv = 0;
    auto tp = type();
    if ((tp == C_BUILTIN_PTR) || (tp == C_BUILTIN_ARRAY)) {
        using T = util::rc_obj<c_type>;
        new (&p_ptr) T{v.p_ptr};
        v.p_ptr.~T();
    } else if (tp == C_BUILTIN_FUNC) {
        using T = util::rc_obj<c_function>;
        new (&p_func) T{v.p_func};
        v.p_func.~T();
    } else {
        p_crec = v.p_crec;
        v.p_crec = nullptr;
    }
    return *this;
}

static inline bool is_token(char c) {
    switch (c) {
        case '&':
        case '*':
        case '[':
        case ']':
        case '(':
        case ')':
            return true;
        default:
            break;
    }
    return false;
}

static inline void add_cv(util::strbuf &o, int cv, bool pre) {
    if (cv & C_CV_CONST) {
        if (!pre) {
            o.append(' ');
        }
        o.append("const");
        if (pre) {
            o.append(' ');
        }
    }
    if (cv & C_CV_VOLATILE) {
        if (!pre) {
            o.append(' ');
        }
        o.append("volatile");
        if (pre) {
            o.append(' ');
        }
    }
}

void c_type::do_serialize(util::strbuf &o, c_object_cont_f cont, void *data) const {
    using D = struct {
        c_object_cont_f cont;
        void *data;
        c_type const *ct;
        int cv;
    };
    D val{cont, data, this, cv()};
    /* FIXME: don't use unref() */
    if (is_ref()) {
        unref().do_serialize(
                o,
                [](util::strbuf &out, void *idata) {
                    D &d = *static_cast<D *>(idata);
                    if (!is_token(out.back())) {
                        out.append(' ');
                    }
                    out.append('&');
                    if (d.cont) {
                        d.cont(out, d.data);
                    }
                },
                &val);
        return;
    }
    switch (type()) {
        case C_BUILTIN_PTR:
            p_ptr->do_serialize(
                    o,
                    [](util::strbuf &out, void *idata) {
                        D &d = *static_cast<D *>(idata);
                        if (!is_token(out.back())) {
                            out.append(' ');
                        }
                        out.append('*');
                        add_cv(out, d.cv, false);
                        if (d.cont) {
                            d.cont(out, d.data);
                        }
                    },
                    &val);
            break;
        case C_BUILTIN_ARRAY:
            p_ptr->do_serialize(
                    o,
                    [](util::strbuf &out, void *idata) {
                        D &d = *static_cast<D *>(idata);
                        if (!is_token(out.back())) {
                            out.append(' ');
                        }
                        out.append('(');
                        auto sz = out.size();
                        if (d.cont) {
                            d.cont(out, d.data);
                        }
                        add_cv(out, d.ct->cv(), false);
                        if (sz == out.size()) {
                            out.pop_back();
                            out.pop_back();
                        } else if (out.back() != ']') {
                            out.append(')');
                        } else {
                            out.remove(sz - 1);
                        }
                        out.append('[');
                        if (d.ct->vla()) {
                            out.append('?');
                        } else if (!d.ct->flex()) {
                            char buf[32];
                            util::write_u(buf, sizeof(buf), d.ct->array_size());
                            out.append(buf);
                        }
                        out.append(']');
                    },
                    &val);
            break;
        case C_BUILTIN_FUNC:
            p_func->do_serialize(o, cont, data);
            return;
        case C_BUILTIN_RECORD:
            p_crec->do_serialize(o, cont, data);
            break;
        default:
            add_cv(o, val.cv, true);
            o.append(name());
            if (cont) {
                cont(o, data);
            }
            break;
    }
}

bool c_type::passable() const {
    switch (type()) {
        case C_BUILTIN_RECORD:
            return p_crec->passable();
        case C_BUILTIN_VOID:
        case C_BUILTIN_INVALID:
            return false;
        default:
            break;
    }
    return true;
}

#define C_BUILTIN_CASE(bt) \
    case C_BUILTIN_##bt:   \
        return ast::builtin_ffi_type<C_BUILTIN_##bt>();

ffi_type *c_type::libffi_type() const {
    if (is_ref()) {
        return &ffi_type_pointer;
    }

    switch (c_builtin(type())) {
        C_BUILTIN_CASE(VOID)
        C_BUILTIN_CASE(PTR)
        C_BUILTIN_CASE(ARRAY)
        C_BUILTIN_CASE(VA_LIST)

        case C_BUILTIN_FUNC:
            return p_func->libffi_type();

        case C_BUILTIN_RECORD:
            return p_crec->libffi_type();
        case C_BUILTIN_ENUM:
            return p_cenum->libffi_type();

            C_BUILTIN_CASE(FLOAT)
            C_BUILTIN_CASE(DOUBLE)
            C_BUILTIN_CASE(LDOUBLE)

            C_BUILTIN_CASE(BOOL)

            C_BUILTIN_CASE(CHAR)
            C_BUILTIN_CASE(SCHAR)
            C_BUILTIN_CASE(UCHAR)
            C_BUILTIN_CASE(SHORT)
            C_BUILTIN_CASE(USHORT)
            C_BUILTIN_CASE(INT)
            C_BUILTIN_CASE(UINT)
            C_BUILTIN_CASE(LONG)
            C_BUILTIN_CASE(ULONG)
            C_BUILTIN_CASE(LLONG)
            C_BUILTIN_CASE(ULLONG)

        case C_BUILTIN_INVALID:
            break;

            /* intentionally no default so that missing members are caught */
    }

    assert(false);
    return nullptr;
}

std::size_t c_type::alloc_size() const {
    switch (c_builtin(type())) {
        case C_BUILTIN_FUNC:
            return p_func->alloc_size();
        case C_BUILTIN_RECORD:
            return p_crec->alloc_size();
        case C_BUILTIN_ENUM:
            return p_cenum->alloc_size();
        case C_BUILTIN_ARRAY:
            /* may occasionally be zero sized, particularly where
             * dealt with entirely on the C side (so we don't know the
             * allocation size). That's fine, this is never relied upon
             * in contexts where that would be important
             */
            return p_asize * p_ptr->alloc_size();
        default:
            break;
    }
    return libffi_type()->size;
}

#undef C_BUILTIN_CASE

/* these sameness implementations are basic and non-compliant for now, just
 * to have something to get started with, edge cases will be covered later
 */

bool c_type::is_same(c_type const &other, bool ignore_cv, bool ignore_ref) const {
    if (!ignore_cv && (cv() != other.cv())) {
        return false;
    }
    if (!ignore_ref && (is_ref() != other.is_ref())) {
        return false;
    }
    /* again manually covering all cases to make sure we really have them */
    switch (c_builtin(type())) {
        case C_BUILTIN_VOID:
        case C_BUILTIN_BOOL:
        case C_BUILTIN_VA_LIST:
        case C_BUILTIN_CHAR:
        case C_BUILTIN_SCHAR:
        case C_BUILTIN_UCHAR:
        case C_BUILTIN_SHORT:
        case C_BUILTIN_USHORT:
        case C_BUILTIN_INT:
        case C_BUILTIN_UINT:
        case C_BUILTIN_LONG:
        case C_BUILTIN_ULONG:
        case C_BUILTIN_LLONG:
        case C_BUILTIN_ULLONG:
        case C_BUILTIN_FLOAT:
        case C_BUILTIN_DOUBLE:
        case C_BUILTIN_LDOUBLE:
            return type() == other.type();

        case C_BUILTIN_FUNC:
            if (other.type() == C_BUILTIN_PTR) {
                if (other.ptr_base().type() == C_BUILTIN_FUNC) {
                    return is_same(other.ptr_base(), false, ignore_ref);
                }
                return false;
            } else if (other.type() == C_BUILTIN_FUNC) {
                return p_func->is_same(*other.p_func);
            }
            return false;

        case C_BUILTIN_ENUM:
            if (type() != other.type()) {
                return false;
            }
            return (p_cenum == other.p_cenum);

        case C_BUILTIN_RECORD:
            if (type() != other.type()) {
                return false;
            }
            return p_crec->is_same(*other.p_crec);

        case C_BUILTIN_PTR:
            if (other.type() == C_BUILTIN_FUNC) {
                if (ptr_base().type() == C_BUILTIN_FUNC) {
                    return ptr_base().is_same(other);
                }
                return false;
            }
            if (type() != other.type()) {
                return false;
            }
            return p_ptr->is_same(*other.p_ptr);

        case C_BUILTIN_ARRAY:
            if (type() != other.type()) {
                return false;
            }
            if (p_asize != other.p_asize) {
                return false;
            }
            return p_ptr->is_same(*other.p_ptr);

        case C_BUILTIN_INVALID:
            break;
    }

    assert(false);
    return false;
}

bool c_function::is_same(c_function const &other) const {
    if (!p_result.is_same(other.p_result)) {
        return false;
    }
    if (p_variadic != other.p_variadic) {
        return false;
    }
    if (p_params.size() != other.p_params.size()) {
        return false;
    }
    for (std::size_t i = 0; i < p_params.size(); ++i) {
        if (!p_params[i].type().is_same(other.p_params[i].type())) {
            return false;
        }
    }
    return true;
}

bool c_record::is_same(c_record const &other) const { return &other == this; }

std::ptrdiff_t c_record::field_offset(char const *fname, c_type const *&fld) const {
    std::ptrdiff_t ret = -1;
    iter_fields([fname, &ret, &fld](char const *ffname, ast::c_type const &ffld, std::size_t off) {
        if (!std::strcmp(fname, ffname)) {
            ret = std::ptrdiff_t(off);
            fld = &ffld;
            return true;
        }
        return false;
    });
    return ret;
}

static inline ffi_type *libffi_base(ast::c_type const &tp, std::size_t &asz) {
    if (!tp.builtin_array()) {
        asz = 1;
        return tp.libffi_type();
    }
    auto sz = tp.array_size();
    auto *pb = &tp.ptr_base();
    while (pb->builtin_array()) {
        sz *= pb->array_size();
        pb = &pb->ptr_base();
    }
    asz = sz;
    return pb->libffi_type();
}

std::size_t c_record::iter_fields(bool (*cb)(char const *fname, ast::c_type const &type, std::size_t off, void *data), void *data, std::size_t obase, bool &end) const {
    std::size_t base = 0;
    std::size_t nflds = p_fields.size();
    bool flex = false;
    bool uni = is_union();
    if (!uni && nflds && p_fields.back().type.flex()) {
        flex = true;
        --nflds;
    }
    for (std::size_t i = 0; i < nflds; ++i) {
        std::size_t asz;
        auto *tp = libffi_base(p_fields[i].type, asz);
        std::size_t align = tp->alignment;
        base = ((base + align - 1) / align) * align;
        if (p_fields[i].name.empty()) {
            /* transparent record is like a real member */
            assert(p_fields[i].type.type() == ast::C_BUILTIN_RECORD);
            p_fields[i].type.record().iter_fields(cb, data, base, end);
            if (end) {
                return base;
            }
        } else {
            end = cb(p_fields[i].name.data(), p_fields[i].type, obase + base, data);
            if (end) {
                return base;
            }
        }
        if (!uni) {
            base += tp->size * asz;
        }
    }
    if (flex) {
        base = p_ffi_type.size;
        end = cb(p_fields.back().name.data(), p_fields.back().type, obase + base, data);
    }
    return base;
}

#if FFI_CPU(ARM64) || defined(FFI_ARCH_PPC64_ELFV2)
#define FFI_UNION_HAGGREG 1
#endif

#ifdef FFI_UNION_HAGGREG
static ffi_type *union_base_type(c_type const &ct, std::size_t &asz) {
    ffi_type *ret = nullptr;

    if (ct.is_ref()) {
        return nullptr;
    }

    switch (ct.type()) {
        case C_BUILTIN_ARRAY:
            asz *= ct.array_size();
            return union_base_type(ct.ptr_base(), asz);

        case C_BUILTIN_FLOAT:
            return &ffi_type_float;
        case C_BUILTIN_DOUBLE:
            return &ffi_type_double;
        case C_BUILTIN_LDOUBLE:
            return &ffi_type_longdouble;

        case C_BUILTIN_RECORD:
            break;

        default:
            return nullptr;
    }

    auto &rec = ct.record();
    auto &flds = rec.raw_fields();

    for (std::size_t i = 0; i < flds.size(); ++i) {
        std::size_t nasz = 1;
        auto &fld = flds[i];
        ffi_type *hg = union_base_type(fld.type, nasz);
        if (!hg || (ret && (hg != ret))) {
            return nullptr;
        }
        asz += nasz;
        ret = hg;
    }
    return ret;
}
#endif

static ffi_type **resolve_union(util::vector<c_record::field> const &flds, ffi_type &fft) {
    std::size_t nflds = flds.size();
    std::size_t usize = 0;
    unsigned short ualign = 0;
    bool maybe_homog = true;
    ffi_type *ubase = nullptr;

    for (std::size_t i = 0; i < nflds; ++i) {
        std::size_t asz = 1;
        ffi_type *try_ubase;
#ifdef FFI_UNION_HAGGREG
        try_ubase = union_base_type(flds[i].type, asz);
#else
        try_ubase = nullptr;
#endif
        if (!try_ubase) {
            maybe_homog = false;
            try_ubase = libffi_base(flds[i].type, asz);
        } else if (ubase && (try_ubase != ubase)) {
            maybe_homog = false;
        }
        if (try_ubase->alignment > ualign) {
            ualign = try_ubase->alignment;
        }
        if ((try_ubase->size * asz) > usize) {
            usize = try_ubase->size * asz;
        }
        if (maybe_homog) {
            ubase = try_ubase;
        }
    }

    fft.type = FFI_TYPE_STRUCT;
    /* the alignment is one thing we always know for sure */
    fft.alignment = ualign;

    if (maybe_homog) {
        /* homogenous aggregates: our size and alignment are specific and
         * strict, so just make up a structure that is N times our wanted
         * type where N is the number of times the type can fit in the size
         */
        std::size_t nelem = (usize / ubase->size);
        ffi_type **elems = new ffi_type *[nelem + 1];
        for (std::size_t i = 0; i < nelem; ++i) {
            elems[i] = ubase;
        }
        elems[nelem] = nullptr;
        fft.size = usize;
        fft.elements = &elems[0];
        return elems;
    }

    /* other types, fill with biggest integers that have an alignment
     * same or looser than the alignment of the biggest field, and size
     * same or smaller than the alignment to prevent overpadding
     */
    auto check_ubase = [ualign](auto &tp) {
        if (ualign % tp.alignment) {
            return false;
        }
        return (tp.alignment >= tp.size);
    };
    if (check_ubase(ffi_type_uint64)) {
        ubase = &ffi_type_uint64;
    } else if (check_ubase(ffi_type_uint32)) {
        ubase = &ffi_type_uint32;
    } else if (check_ubase(ffi_type_uint16)) {
        ubase = &ffi_type_uint16;
    } else {
        ubase = &ffi_type_uint8;
    }

    /* pad size to multiple of alignment: this is the real size of the type */
    usize = ((usize + ualign - 1) / ualign) * ualign;
    /* this is how many of selected base type can fit in wholly */
    std::size_t nelem = usize / ubase->size;
    /* pad the rest with bytes */
    std::size_t npad = usize - (nelem * ubase->size);
    /* the actual number of fields: whole elements + pad bytes + terminator */
    ffi_type **elems = new ffi_type *[nelem + npad + 1];
    for (std::size_t i = 0; i < nelem; ++i) {
        elems[i] = ubase;
    }
    for (std::size_t i = 0; i < npad; ++i) {
        elems[nelem + i] = &ffi_type_uchar;
    }
    elems[nelem] = nullptr;
    fft.size = usize;
    fft.elements = &elems[0];
    return elems;
}

void c_record::set_fields(util::vector<field> fields) {
    assert(p_fields.empty());
    assert(!p_elements);

    p_fields = util::move(fields);

    /* unions are handled specially; they are a struct that is filled
     * to the correct size and with correct types to satisfy ABI (when
     * passing is allowed); alignment is handled manually
     */
    if (is_union()) {
        p_elements = resolve_union(p_fields, p_ffi_type);
        return;
    }

    /* when dealing with flexible array members, we will need to pad the
     * struct to satisfy alignment of the flexible member, and use that
     * as the last member of the struct
     *
     * when the last member is a VLA, we don't know the size, so do the
     * same thing as when flexible, but make the VLA inaccessible
     */
    bool flex = !p_fields.empty() && p_fields.back().type.flex();
    std::size_t nfields = p_fields.size();
    std::size_t ffields = flex ? (nfields - 1) : nfields;

    std::size_t nelements = 0;
    for (std::size_t i = 0; i < ffields; ++i) {
        std::size_t asz;
        /* the type itself is unimportant for now but we need the count */
        libffi_base(p_fields[i].type, asz);
        nelements += asz;
    }

    p_elements = new ffi_type *[nelements + 1];

    p_ffi_type.size = p_ffi_type.alignment = 0;
    p_ffi_type.type = FFI_TYPE_STRUCT;

    p_ffi_type.elements = &p_elements[0];
    p_elements[nelements] = nullptr;

    for (std::size_t i = 0, e = 0; i < ffields; ++i) {
        std::size_t asz;
        auto *ft = libffi_base(p_fields[i].type, asz);
        for (std::size_t j = 0; j < asz; ++j) {
            p_elements[e + j] = ft;
        }
        e += asz;
    }

    if (flex) {
        /* for now null it, so ffi_prep_cif ignores it */
        p_elements[nelements] = nullptr;
    }

    /* fill in the size and alignment with an ugly hack
     *
     * we can make use of the size/alignment at runtime, so make sure
     * it's guaranteed to be properly filled in, even if the type has
     * not been used with a function
     *
     * for flexible array members the resulting size will need to get
     * padded a bit, do that afterwards
     */
    ffi_cif cif;
    /* this should generally not fail, as we're using the default ABI
     * and validating our type definitions beforehand, but maybe make
     * it a real error?
     */
    auto ret = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 0, &p_ffi_type, nullptr);
    static_cast<void>(ret); /* silence NDEBUG warnings */
    /* this should always succeed */
    assert(ret == FFI_OK);

    if (!flex) {
        return;
    }

    /* alignment of the base type of the final array */
    auto falign = p_fields.back().type.ptr_base().libffi_type()->alignment;
    auto padn = p_ffi_type.size % falign;

    /* the current size is an actual multiple, so no padding needed */
    if (!padn) {
        return;
    }
    /* otherwise create the padding struct */
    padn = falign - padn;
    p_felems = new ffi_type *[padn + 1];

    /* we know the size and alignment, since it's just padding bytes */
    p_ffi_flex.size = padn;
    p_ffi_flex.alignment = 1;
    for (std::size_t i = 0; i < padn; ++i) {
        p_felems[i] = &ffi_type_uchar;
    }
    p_felems[padn] = nullptr;
    p_ffi_flex.elements = &p_felems[0];

    /* and add it as a member + bump the size */
    p_elements[nelements] = &p_ffi_flex;
    p_ffi_type.size += padn;
}

/* decl store implementation, with overlaying for staging */

c_object const *decl_store::add(c_object *decl) {
    auto *oldecl = lookup(decl->name());
    if (oldecl) {
        auto ot = decl->obj_type();
        if ((ot != ast::c_object_type::VARIABLE) && (ot != ast::c_object_type::TYPEDEF)) {
            delete decl;
            return oldecl;
        } else {
            /* redefinitions of vars and funcs are okay
             * luajit doesn't check them so we don't either
             */
            delete decl;
            return nullptr;
        }
    }

    p_dlist.emplace_back(decl);
    auto &d = *p_dlist.back().value;
    p_dmap.insert(d.name(), &d);
    return nullptr;
}

void decl_store::commit() {
    /* this should only ever be used when staging */
    assert(p_base);
    /* reserve all space at once */
    p_base->p_dlist.reserve(p_base->p_dlist.size() + p_dlist.size());
    /* move all */
    for (std::size_t i = 0; i < p_dlist.size(); ++i) {
        p_base->p_dlist.push_back(util::move(p_dlist[i]));
    }
    /* set up mappings in base */
    p_dmap.for_each([this](char const *key, c_object *value) { p_base->p_dmap.insert(key, value); });
    p_base->name_counter += name_counter;
    drop();
}

void decl_store::drop() {
    p_dmap.clear();
    p_dlist.clear();
    name_counter = 0;
}

c_object const *decl_store::lookup(char const *name) const {
    auto *o = p_dmap.find(name);
    if (o) {
        return *o;
    }
    if (p_base) {
        return p_base->lookup(name);
    }
    return nullptr;
}

c_object *decl_store::lookup(char const *name) {
    auto *o = p_dmap.find(name);
    if (o) {
        return *o;
    }
    if (p_base) {
        return p_base->lookup(name);
    }
    return nullptr;
}

std::size_t decl_store::request_name(char *buf, std::size_t bufsize) {
    /* could do something better, this will do to avoid clashes for now... */
    std::size_t n = name_counter++;
    for (auto *pb = p_base; pb; pb = pb->p_base) {
        n += pb->name_counter;
    }
    return util::write_u(buf, bufsize, n);
}

c_type from_lua_type(lua_State *L, int index) {
    switch (lua_type(L, index)) {
        case LUA_TNIL:
            return c_type{util::make_rc<c_type>(C_BUILTIN_VOID, 0), 0, C_BUILTIN_PTR};
        case LUA_TBOOLEAN:
            return c_type{C_BUILTIN_BOOL, 0};
        case LUA_TNUMBER:
            static_assert(builtin_v<lua_Number> != C_BUILTIN_INVALID, "invalid lua_Number definition");
            static_assert(builtin_v<lua_Integer> != C_BUILTIN_INVALID, "invalid lua_Integer definition");
            /* 5.3+; always returns false on <= 5.2 */
            if (lua_isinteger(L, index)) {
                return c_type{builtin_v<lua_Integer>, 0};
            }
            return c_type{builtin_v<lua_Number>, 0};
        case LUA_TSTRING:
            return c_type{util::make_rc<c_type>(C_BUILTIN_CHAR, C_CV_CONST), 0, C_BUILTIN_PTR};
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TTHREAD:
        case LUA_TLIGHTUSERDATA:
            /* by default use a void pointer, some will fail, that's ok */
            return c_type{util::make_rc<c_type>(C_BUILTIN_VOID, 0), 0, C_BUILTIN_PTR};
        case LUA_TUSERDATA: {
            auto *cd = ffi::testcdata(L, index);
            if (!cd) {
                return c_type{util::make_rc<c_type>(C_BUILTIN_VOID, 0), 0, C_BUILTIN_PTR};
            }
            return cd->decl.copy();
        }
        default:
            break;
    }
    assert(false);
    return c_type{C_BUILTIN_INVALID, 0};
}

} /* namespace ast */

#if defined(__CYGWIN__) || (defined(_WIN32) && !defined(_XBOX_VER))
#ifdef CFFI_LUA_DLL
#define CFFI_LUA_EXPORT __declspec(dllexport)
#else
#define CFFI_LUA_EXPORT
#endif
#else
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define CFFI_LUA_EXPORT __attribute__((visibility("default")))
#else
#define CFFI_LUA_EXPORT
#endif
#endif

void ffi_module_open(lua_State *L);
