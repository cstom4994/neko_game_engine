#include "engine/base.hpp"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <array>
#include <new>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/base/color.hpp"
#include "engine/bootstrap.h"
#include "engine/graphics.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/scripting.h"
#include "vendor/luaalloc.h"

void neko_log(const char *file, int line, const char *fmt, ...) {

    LockGuard<Mutex> lock(g_app->log_mtx);

    typedef struct {
        va_list ap;
        const char *fmt;
        const char *file;
        u32 time;
        FILE *udata;
        int line;
    } neko_log_event;

    static auto init_event = [](neko_log_event *ev, void *udata) {
        static u32 t = 0;
        if (!ev->time) {
            ev->time = ++t;
        }
        ev->udata = (FILE *)udata;
    };

    neko_log_event ev = {
            .fmt = fmt,
            .file = file,
            .line = line,
    };

    init_event(&ev, stderr);
    va_start(ev.ap, fmt);
    // fprintf(ev.udata, "%s:%d: ", neko_util_get_filename(ev.file), ev.line);
    vfprintf(ev.udata, ev.fmt, ev.ap);
    fprintf(ev.udata, "\n");
    fflush(ev.udata);
    va_end(ev.ap);
}

static void _error(const char *s) { script_error(s); }

void errorf(const char *fmt, ...) {
    va_list ap1, ap2;
    unsigned int n;
    char *s;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    // how much space do we need?
    n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    // allocate, sprintf, print
    s = (char *)mem_alloc(n + 1);
    vsprintf(s, fmt, ap1);
    va_end(ap1);
    _error(s);
    mem_free(s);
}

// ===============================================================

#if 0

vec2 vec2_add(vec2 u, vec2 v) { return luavec2(u.x + v.x, u.y + v.y); }
vec2 vec2_sub(vec2 u, vec2 v) { return luavec2(u.x - v.x, u.y - v.y); }
vec2 vec2_mul(vec2 u, vec2 v) { return luavec2(u.x * v.x, u.y * v.y); }
vec2 vec2_div(vec2 u, vec2 v) { return luavec2(u.x / v.x, u.y / v.y); }

vec2 vec2_neg(vec2 v) { return luavec2(-v.x, -v.y); }

Scalar vec2_len(vec2 v) { return scalar_sqrt(v.x * v.x + v.y * v.y); }
vec2 vec2_normalize(vec2 v) {
    if (v.x == 0 && v.y == 0) return v;
    return vec2_scalar_div(v, vec2_len(v));
}
Scalar vec2_dot(vec2 u, vec2 v) { return u.x * v.x + u.y * v.y; }
Scalar vec2_dist(vec2 u, vec2 v) { return vec2_len(vec2_sub(u, v)); }

Scalar vec2_atan2(vec2 v) { return scalar_atan2(v.y, v.x); }

#endif

vec2 vec2_add(vec2 v0, vec2 v1) { return vec2_ctor(v0.x + v1.x, v0.y + v1.y); }

vec2 vec2_sub(vec2 v0, vec2 v1) { return vec2_ctor(v0.x - v1.x, v0.y - v1.y); }

vec2 vec2_mul(vec2 v0, vec2 v1) { return vec2_ctor(v0.x * v1.x, v0.y * v1.y); }

vec2 vec2_div(vec2 v0, vec2 v1) { return vec2_ctor(v0.x / v1.x, v0.y / v1.y); }

vec2 vec2_zero = {0.0, 0.0};

vec2 vec2_rot(vec2 v, Scalar rot) { return luavec2(v.x * scalar_cos(rot) - v.y * scalar_sin(rot), v.x * scalar_sin(rot) + v.y * scalar_cos(rot)); }

vec2 vec2_scalar_mul(vec2 v, Scalar f) { return luavec2(v.x * f, v.y * f); }
vec2 vec2_scalar_div(vec2 v, Scalar f) { return luavec2(v.x / f, v.y / f); }
vec2 scalar_vec2_div(Scalar f, vec2 v) { return luavec2(f / v.x, f / v.y); }

#undef luavec2
vec2 luavec2(Scalar x, Scalar y) { return vec2{x, y}; }

void vec2_save(vec2 *v, const char *n, Store *s) {
    Store *t;

    if (store_child_save_compressed(&t, n, s)) {
        scalar_save(&v->x, "x", t);
        scalar_save(&v->y, "y", t);
    }
}
bool vec2_load(vec2 *v, const char *n, vec2 d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        scalar_load(&v->x, "x", 0, t);
        scalar_load(&v->y, "y", 0, t);
    } else
        *v = d;
    return t != NULL;
}

#if 0

mat3 mat3_mul(mat3 m, mat3 n) {
    return luamat3(m.m[0][0] * n.m[0][0] + m.m[1][0] * n.m[0][1] + m.m[2][0] * n.m[0][2], m.m[0][1] * n.m[0][0] + m.m[1][1] * n.m[0][1] + m.m[2][1] * n.m[0][2],
                   m.m[0][2] * n.m[0][0] + m.m[1][2] * n.m[0][1] + m.m[2][2] * n.m[0][2],

                   m.m[0][0] * n.m[1][0] + m.m[1][0] * n.m[1][1] + m.m[2][0] * n.m[1][2], m.m[0][1] * n.m[1][0] + m.m[1][1] * n.m[1][1] + m.m[2][1] * n.m[1][2],
                   m.m[0][2] * n.m[1][0] + m.m[1][2] * n.m[1][1] + m.m[2][2] * n.m[1][2],

                   m.m[0][0] * n.m[2][0] + m.m[1][0] * n.m[2][1] + m.m[2][0] * n.m[2][2], m.m[0][1] * n.m[2][0] + m.m[1][1] * n.m[2][1] + m.m[2][1] * n.m[2][2],
                   m.m[0][2] * n.m[2][0] + m.m[1][2] * n.m[2][1] + m.m[2][2] * n.m[2][2]);
}


mat3 mat3_inverse(mat3 m) {
    Scalar det;
    mat3 inv;

    inv.m[0][0] = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];`
    inv.m[0][1] = m.m[0][2] * m.m[2][1] - m.m[0][1] * m.m[2][2];
    inv.m[0][2] = m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1];
    inv.m[1][0] = m.m[1][2] * m.m[2][0] - m.m[1][0] * m.m[2][2];
    inv.m[1][1] = m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0];
    inv.m[1][2] = m.m[0][2] * m.m[1][0] - m.m[0][0] * m.m[1][2];
    inv.m[2][0] = m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0];
    inv.m[2][1] = m.m[0][1] * m.m[2][0] - m.m[0][0] * m.m[2][1];
    inv.m[2][2] = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];

    det = m.m[0][0] * inv.m[0][0] + m.m[0][1] * inv.m[1][0] + m.m[0][2] * inv.m[2][0];

    if (det <= 10e-8) return inv;  // TODO: figure out what to do if not invertible

    inv.m[0][0] /= det;
    inv.m[0][1] /= det;
    inv.m[0][2] /= det;
    inv.m[1][0] /= det;
    inv.m[1][1] /= det;
    inv.m[1][2] /= det;
    inv.m[2][0] /= det;
    inv.m[2][1] /= det;
    inv.m[2][2] /= det;

    return inv;
}

#endif

vec2 mat3_transform(mat3 m, vec2 v) { return luavec2(m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0], m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1]); }

mat3 mat3_scaling_rotation_translation(vec2 scale, Scalar rot, vec2 trans) {
    return luamat3(scale.x * scalar_cos(rot), scale.x * scalar_sin(rot), 0.0f, scale.y * -scalar_sin(rot), scale.y * scalar_cos(rot), 0.0f, trans.x, trans.y, 1.0f);
}

vec2 mat3_get_translation(mat3 m) { return luavec2(m.m[2][0], m.m[2][1]); }
Scalar mat3_get_rotation(mat3 m) { return scalar_atan2(m.m[0][1], m.m[0][0]); }
vec2 mat3_get_scale(mat3 m) { return luavec2(scalar_sqrt(m.m[0][0] * m.m[0][0] + m.m[0][1] * m.m[0][1]), scalar_sqrt(m.m[1][0] * m.m[1][0] + m.m[1][1] * m.m[1][1])); }

void mat3_save(mat3 *m, const char *n, Store *s) {
    Store *t;
    unsigned int i, j;

    if (store_child_save_compressed(&t, n, s))
        for (i = 0; i < 3; ++i)
            for (j = 0; j < 3; ++j) scalar_save(&m->m[i][j], NULL, t);
}
bool mat3_load(mat3 *m, const char *n, mat3 d, Store *s) {
    Store *t;
    unsigned int i, j;

    if (store_child_load(&t, n, s))
        for (i = 0; i < 3; ++i)
            for (j = 0; j < 3; ++j) scalar_load(&m->m[i][j], NULL, 0, t);
    else
        *m = d;
    return t != NULL;
}

#undef luamat3
mat3 luamat3(Scalar m00, Scalar m01, Scalar m02, Scalar m10, Scalar m11, Scalar m12, Scalar m20, Scalar m21, Scalar m22) { return mat3{.m = {{m00, m01, m02}, {m10, m11, m12}, {m20, m21, m22}}}; }

mat3 mat3_identity() { return mat3_diag(1.f); }

BBox bbox_merge(BBox a, BBox b) { return bbox(luavec2(scalar_min(a.min.x, b.min.x), scalar_min(a.min.y, b.min.y)), luavec2(scalar_max(a.max.x, b.max.x), scalar_max(a.max.y, b.max.y))); }
BBox bbox_bound(vec2 a, vec2 b) { return bbox(luavec2(scalar_min(a.x, b.x), scalar_min(a.y, b.y)), luavec2(scalar_max(a.x, b.x), scalar_max(a.y, b.y))); }
bool bbox_contains(BBox b, vec2 p) { return b.min.x <= p.x && p.x <= b.max.x && b.min.y <= p.y && p.y <= b.max.y; }

BBox bbox(vec2 min, vec2 max) {
    BBox bb;
    bb.min = min;
    bb.max = max;
    return bb;
}

BBox bbox_transform(mat3 m, BBox b) {
    vec2 v1, v2, v3, v4;

    v1 = mat3_transform(m, luavec2(b.min.x, b.min.y));
    v2 = mat3_transform(m, luavec2(b.max.x, b.min.y));
    v3 = mat3_transform(m, luavec2(b.max.x, b.max.y));
    v4 = mat3_transform(m, luavec2(b.min.x, b.max.y));

    return bbox_merge(bbox_bound(v1, v2), bbox_bound(v3, v4));
}

Color color_black = {0.0, 0.0, 0.0, 1.0};
Color color_white = {1.0, 1.0, 1.0, 1.0};
Color color_gray = {0.5, 0.5, 0.5, 1.0};
Color color_red = {1.0, 0.0, 0.0, 1.0};
Color color_green = {0.0, 1.0, 0.0, 1.0};
Color color_blue = {0.0, 0.0, 1.0, 1.0};
Color color_clear = {0.0, 0.0, 0.0, 0.0};

void color_save(Color *c, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) {
        scalar_save(&c->r, "r", t);
        scalar_save(&c->g, "g", t);
        scalar_save(&c->b, "b", t);
        scalar_save(&c->a, "a", t);
    }
}
bool color_load(Color *c, const char *n, Color d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        scalar_load(&c->r, "r", 0, t);
        scalar_load(&c->g, "g", 0, t);
        scalar_load(&c->b, "b", 0, t);
        scalar_load(&c->a, "a", 0, t);
    } else
        *c = d;
    return t != NULL;
}

#undef color_opaque
Color color_opaque(f32 r, f32 g, f32 b) { return color(r, g, b, 1); }

#undef color
Color color(f32 r, f32 g, f32 b, f32 a) { return Color{r, g, b, a}; }

typedef struct Stream Stream;
struct Stream {
    char *buf;
    size_t pos;
    size_t cap;
};

struct Store {
    char *name;
    Stream sm[1];
    bool compressed;

    Store *child;
    Store *parent;
    Store *sibling;

    Store *iterchild;

    char *str;
};

static void _stream_init(Stream *sm) {
    sm->buf = NULL;
    sm->pos = 0;
    sm->cap = 0;
}

static void _stream_fini(Stream *sm) { mem_free(sm->buf); }

static void _stream_grow(Stream *sm, size_t pos) {
    if (pos >= sm->cap) {
        if (sm->cap < 2) sm->cap = 2;
        while (pos >= sm->cap) sm->cap <<= 1;
        sm->buf = (char *)mem_realloc(sm->buf, sm->cap);
    }
}

static void _stream_printf(Stream *sm, const char *fmt, ...) {
    va_list ap1, ap2;
    size_t new_pos;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    new_pos = sm->pos + vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    _stream_grow(sm, new_pos);
    vsprintf(sm->buf + sm->pos, fmt, ap1);
    sm->pos = new_pos;
    va_end(ap1);
}

static void _stream_scanf_(Stream *sm, const char *fmt, int *n, ...) {
    va_list ap;

    error_assert(sm->buf, "stream buffer should be initialized");

    /*
     * scanf is tricky because we need to move forward by number of
     * scanned characters -- *n will store number of characters read,
     * needs to also be put at end of parameter list (see
     * _stream_scanf(...) macro), and "%n" needs to be appended at
     * end of original fmt
     */

    va_start(ap, n);
    vsscanf(&sm->buf[sm->pos], fmt, ap);
    va_end(ap);
    sm->pos += *n;
}
#define _stream_scanf(sm, fmt, ...)                                        \
    do {                                                                   \
        int n_read__;                                                      \
        _stream_scanf_(sm, fmt "%n", &n_read__, ##__VA_ARGS__, &n_read__); \
    } while (0)

static void _stream_write_string(Stream *sm, const char *s) {

    if (!s) {
        _stream_printf(sm, "n ");
        return;
    }

    _stream_printf(sm, "\"");

    for (; *s; ++s) {
        _stream_grow(sm, sm->pos);

        if (*s == '"') {
            sm->buf[sm->pos++] = '\\';
            _stream_grow(sm, sm->pos);
        }

        sm->buf[sm->pos++] = *s;
    }

    _stream_printf(sm, "\" ");
}

static char *_stream_read_string_(Stream *sm, size_t *plen) {
    Stream rm[1];

    if (sm->buf[sm->pos] == 'n') {
        if (strncmp(&sm->buf[sm->pos], "n ", 2)) error("corrupt save");
        sm->pos += 2;
        return NULL;
    }

    _stream_init(rm);

    if (sm->buf[sm->pos] != '"') error("corrupt save");
    ++sm->pos;

    while (sm->buf[sm->pos] != '"') {
        _stream_grow(rm, rm->pos);

        if (sm->buf[sm->pos] == '\\' && sm->buf[sm->pos + 1] == '"') {
            rm->buf[rm->pos++] = '"';
            sm->pos += 2;
        } else
            rm->buf[rm->pos++] = sm->buf[sm->pos++];
    }
    sm->pos += 2;

    _stream_grow(rm, rm->pos);
    rm->buf[rm->pos] = '\0';

    if (plen) *plen = rm->cap;

    return rm->buf;
}
#define _stream_read_string(sm) _stream_read_string_(sm, NULL)

static Store *_store_new(Store *parent) {
    Store *s = (Store *)mem_alloc(sizeof(Store));

    s->name = NULL;
    _stream_init(s->sm);
    s->compressed = false;

    s->parent = parent;
    s->child = NULL;
    s->sibling = s->parent ? s->parent->child : NULL;
    if (s->parent) s->parent->iterchild = s->parent->child = s;

    s->iterchild = NULL;
    s->str = NULL;

    return s;
}

static void _store_free(Store *s) {
    Store *t;

    while (s->child) {
        t = s->child->sibling;
        _store_free(s->child);
        s->child = t;
    }

    mem_free(s->name);
    _stream_fini(s->sm);
    mem_free(s->str);

    mem_free(s);
}

static void _store_write(Store *s, Stream *sm) {
    Store *c;

    _stream_printf(sm, s->compressed ? "[ " : "{ ");
    _stream_write_string(sm, s->name);
    _stream_write_string(sm, s->sm->buf);
    for (c = s->child; c; c = c->sibling) _store_write(c, sm);
    _stream_printf(sm, s->compressed ? "] " : "} ");
}

#define INDENT 2

static void _store_write_pretty(Store *s, unsigned int indent, Stream *sm) {
    Store *c;

    if (s->compressed) {
        _stream_printf(sm, "%*s", indent, "");
        _store_write(s, sm);
        _stream_printf(sm, "\n");
        return;
    }

    _stream_printf(sm, "%*s{ ", indent, "");

    _stream_write_string(sm, s->name);
    _stream_write_string(sm, s->sm->buf);
    if (s->child) _stream_printf(sm, "\n");

    for (c = s->child; c; c = c->sibling) _store_write_pretty(c, indent + INDENT, sm);

    if (s->child)
        _stream_printf(sm, "%*s}\n", indent, "");
    else
        _stream_printf(sm, "}\n");
}

static Store *_store_read(Store *parent, Stream *sm) {
    char close_brace = '}';
    Store *s = _store_new(parent);

    if (sm->buf[sm->pos] == '[') {
        s->compressed = true;
        close_brace = ']';
    } else if (sm->buf[sm->pos] != '{' && sm->buf[sm->pos] != '\n')
        error("corrupt save");
    while (isspace(sm->buf[++sm->pos]));

    s->name = _stream_read_string(sm);
    s->sm->buf = _stream_read_string_(sm, &s->sm->cap);
    s->sm->pos = 0;

    for (;;) {
        while (isspace(sm->buf[sm->pos])) ++sm->pos;

        if (sm->buf[sm->pos] == close_brace) {
            ++sm->pos;
            break;
        }

        _store_read(s, sm);
    }

    return s;
}

bool store_child_save(Store **sp, const char *name, Store *parent) {
    Store *s;

    if (parent->compressed) return (*sp = parent) != NULL;

    s = _store_new(parent);
    if (name) {
        s->name = (char *)mem_alloc(strlen(name) + 1);
        strcpy(s->name, name);
    }
    return (*sp = s) != NULL;
}

bool store_child_save_compressed(Store **sp, const char *name, Store *parent) {
    bool r = store_child_save(sp, name, parent);
    (*sp)->compressed = true;
    return r;
}

bool store_child_load(Store **sp, const char *name, Store *parent) {
    Store *s;

    if (parent->compressed) return (*sp = parent) != NULL;

    if (!name) {
        s = parent->iterchild;
        if (parent->iterchild) parent->iterchild = parent->iterchild->sibling;
        return (*sp = s) != NULL;
    }

    for (s = parent->child; s && (!s->name || strcmp(s->name, name)); s = s->sibling);
    return (*sp = s) != NULL;
}

Store *store_open() { return _store_new(NULL); }

Store *store_open_str(const char *str) {
    Stream sm = {(char *)str, 0, 0};
    return _store_read(NULL, &sm);
}
const char *store_write_str(Store *s) {
    Stream sm[1];

    _stream_init(sm);
    _store_write_pretty(s, 0, sm);
    mem_free(s->str);
    s->str = sm->buf;
    return s->str;
}

Store *store_open_file(const char *filename) {
    Store *s;
    unsigned int n;
    char *str;

    vfs_file f = neko_capi_vfs_fopen(filename);
    error_assert(f.data, "file '%s' must be open for reading", filename);

    neko_capi_vfs_fscanf(&f, "%u\n", &n);
    str = (char *)mem_alloc(n + 1);
    neko_capi_vfs_fread(str, 1, n, &f);
    neko_capi_vfs_fclose(&f);
    str[n] = '\0';
    s = store_open_str(str);
    mem_free(str);
    return s;
}

void store_write_file(Store *s, const char *filename) {
    FILE *f;
    const char *str;
    unsigned int n;

    f = fopen(filename, "w");
    error_assert(f, "file '%s' must be open for writing", filename);

    str = store_write_str(s);
    n = strlen(str);
    fprintf(f, "%u\n", n);
    fwrite(str, 1, n, f);
    fclose(f);
}

void store_close(Store *s) { _store_free(s); }

#define _store_printf(s, fmt, ...) _stream_printf(s->sm, fmt, ##__VA_ARGS__)
#define _store_scanf(s, fmt, ...) _stream_scanf(s->sm, fmt, ##__VA_ARGS__)

void scalar_save(const Scalar *f, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) {
        if (*f == SCALAR_INFINITY)
            _store_printf(t, "i ");
        else
            _store_printf(t, "%f ", *f);
    }
}
bool scalar_load(Scalar *f, const char *n, Scalar d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        if (t->sm->buf[t->sm->pos] == 'i') {
            *f = SCALAR_INFINITY;
            _store_scanf(t, "i ");
        } else
            _store_scanf(t, "%f ", f);
        return true;
    }

    *f = d;
    return false;
}

void uint_save(const unsigned int *u, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) _store_printf(t, "%u ", *u);
}
bool uint_load(unsigned int *u, const char *n, unsigned int d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s))
        _store_scanf(t, "%u ", u);
    else
        *u = d;
    return t != NULL;
}

void int_save(const int *i, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) _store_printf(t, "%d ", *i);
}
bool int_load(int *i, const char *n, int d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s))
        _store_scanf(t, "%d ", i);
    else
        *i = d;
    return t != NULL;
}

void bool_save(const bool *b, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) _store_printf(t, "%d ", (int)*b);
}
bool bool_load(bool *b, const char *n, bool d, Store *s) {
    int i = d;
    Store *t;

    if (store_child_load(&t, n, s)) _store_scanf(t, "%d ", &i);
    *b = i;
    return t != NULL;
}

void string_save(const char **c, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) _stream_write_string(t->sm, *c);
}
bool string_load(char **c, const char *n, const char *d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        *c = _stream_read_string(t->sm);
        return true;
    }

    if (d) {
        *c = (char *)mem_alloc(strlen(d) + 1);
        strcpy(*c, d);
    } else
        *c = NULL;
    return false;
}

#if 0

int main()
{
    Store *s, *d, *sprite_s, *pool_s, *elem_s;
    char *c;
    Scalar r;

    s = store_open();
    {
        if (store_child_save(&sprite_s, "sprite", s))
        {
            c = "hello, world";
            string_save(&c, "prop1", sprite_s);

            c = "hello, world ... again";
            string_save(&c, "prop2", sprite_s);

            r = SCALAR_INFINITY;
            scalar_save(&r, "prop6", sprite_s);

            if (store_child_save(&pool_s, "pool", sprite_s))
            {
                store_child_save(&elem_s, "elem1", pool_s);
                store_child_save(&elem_s, "elem2", pool_s);
            }
        }
    }
    store_write_file(s, "test.sav");
    store_close(s);

    // ----

    d = store_open_file("test.sav");
    {
        if (store_child_load(&sprite_s, "sprite", d))
        {
            printf("%s\n", sprite_s->name);

            string_load(&c, "prop1", "hai", sprite_s);
            printf("    prop1: %s\n", c);

            string_load(&c, "prop3", "hai", sprite_s);
            printf("    prop3: %s\n", c);

            string_load(&c, "prop2", "hai", sprite_s);
            printf("    prop2: %s\n", c);

            scalar_load(&r, "prop6", 4.2, sprite_s);
            printf("    prop6: %f\n", r);

            if (store_child_load(&pool_s, "pool", sprite_s))
                while (store_child_load(&elem_s, NULL, pool_s))
                    printf("        %s\n", elem_s->name);
        }
    }
    store_close(d);

    return 0;
}

#endif

/*================================================================================
// Reflection
================================================================================*/

REGISTER_TYPE_DF(i8)
REGISTER_TYPE_DF(i16)
REGISTER_TYPE_DF(i32)
REGISTER_TYPE_DF(i64)
REGISTER_TYPE_DF(u8)
REGISTER_TYPE_DF(u16)
REGISTER_TYPE_DF(u32)
REGISTER_TYPE_DF(u64)
REGISTER_TYPE_DF(bool)
// REGISTER_TYPE_DF(b32)
REGISTER_TYPE_DF(f32)
REGISTER_TYPE_DF(f64)
REGISTER_TYPE_DF(const_str)
REGISTER_TYPE_DF(String)

/*================================================================================
// Deps
================================================================================*/

#ifdef _WIN32
#pragma comment(lib, "ws2_32")
#endif

#define SOKOL_TIME_IMPL
#include "vendor/sokol_time.h"

#define STBI_MALLOC(sz) mem_alloc(sz)
#define STBI_REALLOC(p, newsz) mem_realloc(p, newsz)
#define STBI_FREE(p) mem_free(p)

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

#define STBIR_MALLOC(size, user_data) ((void)(user_data), mem_alloc(size))
#define STBIR_FREE(ptr, user_data) ((void)(user_data), mem_free(ptr))

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "vendor/stb_image_resize2.h"

#define STBIW_MALLOC(sz) mem_alloc(sz)
#define STBIW_REALLOC(p, newsz) mem_realloc(p, newsz)
#define STBIW_FREE(p) mem_free(p)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendor/stb_image_write.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "vendor/stb_rect_pack.h"

#define STBTT_malloc(x, u) ((void)(u), mem_alloc(x))
#define STBTT_free(x, u) ((void)(u), mem_free(x))

#define STB_TRUETYPE_IMPLEMENTATION
#include "vendor/stb_truetype.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#if NEKO_AUDIO == 1
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_WEBAUDIO
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif
