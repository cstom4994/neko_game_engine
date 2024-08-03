#include "neko_base.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "neko_os.h"

SplitLinesIterator &SplitLinesIterator::operator++() {
    if (&view.data[view.len] == &data.data[data.len]) {
        view = {&data.data[data.len], 0};
        return *this;
    }

    String next = {};
    next.data = view.data + view.len + 1;

    u64 end = 0;
    while (&next.data[end] < &data.data[data.len] && next.data[end] != '\n' && next.data[end] != 0) {
        end++;
    }
    next.len = end;

    view = next;
    return *this;
}

bool operator!=(SplitLinesIterator lhs, SplitLinesIterator rhs) {
    return lhs.data.data != rhs.data.data || lhs.data.len != rhs.data.len || lhs.view.data != rhs.view.data || lhs.view.len != rhs.view.len;
}

SplitLinesIterator SplitLines::begin() {
    char *data = str.data;
    u64 end = 0;
    while (data[end] != '\n' && data[end] != 0) {
        end++;
    }

    String view = {str.data, end};
    return {str, view};
}

SplitLinesIterator SplitLines::end() {
    String view = {str.data + str.len, 0};
    return {str, view};
}

i32 utf8_size(u8 c) {
    if (c == '\0') {
        return 0;
    }

    if ((c & 0xF8) == 0xF0) {
        return 4;
    } else if ((c & 0xF0) == 0xE0) {
        return 3;
    } else if ((c & 0xE0) == 0xC0) {
        return 2;
    } else {
        return 1;
    }
}

u32 Rune::charcode() {
    u32 charcode = 0;

    u8 c0 = value >> 0;
    u8 c1 = value >> 8;
    u8 c2 = value >> 16;
    u8 c3 = value >> 24;

    switch (utf8_size(c0)) {
        case 1:
            charcode = c0;
            break;
        case 2:
            charcode = c0 & 0x1F;
            charcode = (charcode << 6) | (c1 & 0x3F);
            break;
        case 3:
            charcode = c0 & 0x0F;
            charcode = (charcode << 6) | (c1 & 0x3F);
            charcode = (charcode << 6) | (c2 & 0x3F);
            break;
        case 4:
            charcode = c0 & 0x07;
            charcode = (charcode << 6) | (c1 & 0x3F);
            charcode = (charcode << 6) | (c2 & 0x3F);
            charcode = (charcode << 6) | (c3 & 0x3F);
            break;
    }

    return charcode;
}

bool Rune::is_whitespace() {
    switch (value) {
        case '\n':
        case '\r':
        case '\t':
        case ' ':
            return true;
    }
    return false;
}

bool Rune::is_digit() { return value >= '0' && value <= '9'; }

Rune rune_from_string(const char *data) {
    u32 rune = 0;
    i32 len = utf8_size(data[0]);
    for (i32 i = len - 1; i >= 0; i--) {
        rune <<= 8;
        rune |= (u8)(data[i]);
    }

    return {rune};
}

static void next_rune(UTF8Iterator *it) {
    if (it->cursor == it->str.len) {
        it->rune.value = 0;
        return;
    }

    char *data = &it->str.data[it->cursor];
    i32 len = utf8_size(data[0]);
    Rune rune = rune_from_string(data);

    it->cursor += len;
    it->rune = rune;
}

UTF8Iterator &UTF8Iterator::operator++() {
    next_rune(this);
    return *this;
}

bool operator!=(UTF8Iterator lhs, UTF8Iterator rhs) { return lhs.str.data != rhs.str.data || lhs.str.len != rhs.str.len || lhs.cursor != rhs.cursor || lhs.rune.value != rhs.rune.value; }

UTF8Iterator UTF8::begin() {
    UTF8Iterator it = {};
    it.str = str;
    next_rune(&it);
    return it;
}

UTF8Iterator UTF8::end() { return {str, str.len, {}}; }

static char s_empty[1] = {0};

StringBuilder::StringBuilder() {
    data = s_empty;
    len = 0;
    capacity = 0;
}

void StringBuilder::trash() {
    if (data != s_empty) {
        mem_free(data);
    }
}

void StringBuilder::reserve(u64 cap) {
    if (cap > capacity) {
        char *buf = (char *)mem_alloc(cap);
        memset(buf, 0, cap);
        memcpy(buf, data, len);

        if (data != s_empty) {
            mem_free(data);
        }

        data = buf;
        capacity = cap;
    }
}

void StringBuilder::clear() {
    len = 0;
    if (data != s_empty) {
        data[0] = 0;
    }
}

void StringBuilder::swap_filename(String filepath, String file) {
    clear();

    u64 slash = filepath.last_of('/');
    if (slash != (u64)-1) {
        String path = filepath.substr(0, slash + 1);
        *this << path;
    }

    *this << file;
}

void StringBuilder::concat(String str, i32 times) {
    for (i32 i = 0; i < times; i++) {
        *this << str;
    }
}

StringBuilder &StringBuilder::operator<<(String str) {
    u64 desired = len + str.len + 1;
    u64 cap = capacity;

    if (desired >= cap) {
        u64 growth = cap > 0 ? cap * 2 : 8;
        if (growth <= desired) {
            growth = desired;
        }

        reserve(growth);
    }

    memcpy(&data[len], str.data, str.len);
    len += str.len;
    data[len] = 0;
    return *this;
}

StringBuilder::operator String() { return {data, len}; }

String str_fmt(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    i32 len = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (len > 0) {
        char *data = (char *)mem_alloc(len + 1);
        va_start(args, fmt);
        vsnprintf(data, len + 1, fmt, args);
        va_end(args);
        return {data, (u64)len};
    }

    return {};
}

String tmp_fmt(const char *fmt, ...) {
    static char s_buf[1024] = {};

    va_list args;
    va_start(args, fmt);
    i32 len = vsnprintf(s_buf, sizeof(s_buf), fmt, args);
    va_end(args);
    return {s_buf, (u64)len};
}

double string_to_double(String str) {
    double n = 0;
    double sign = 1;

    if (str.len == 0) {
        return n;
    }

    u64 i = 0;
    if (str.data[0] == '-' && str.len > 1 && is_digit(str.data[1])) {
        i++;
        sign = -1;
    }

    while (i < str.len) {
        if (!is_digit(str.data[i])) {
            break;
        }

        n = n * 10 + (str.data[i] - '0');
        i++;
    }

    if (i < str.len && str.data[i] == '.') {
        i++;
        double place = 10;
        while (i < str.len) {
            if (!is_digit(str.data[i])) {
                break;
            }

            n += (str.data[i] - '0') / place;
            place *= 10;
            i++;
        }
    }

    return n * sign;
}

struct ArenaNode {
    ArenaNode *next;
    u64 capacity;
    u64 allocd;
    u64 prev;
    u8 buf[1];
};

static u64 align_forward(u64 p, u32 align) {
    if ((p & (align - 1)) != 0) {
        p += align - (p & (align - 1));
    }
    return p;
}

static ArenaNode *arena_block_make(u64 capacity) {
    u64 page = 4096 - offsetof(ArenaNode, buf);
    if (capacity < page) {
        capacity = page;
    }

    ArenaNode *a = (ArenaNode *)mem_alloc(offsetof(ArenaNode, buf[capacity]));
    a->next = nullptr;
    a->allocd = 0;
    a->capacity = capacity;
    return a;
}

void Arena::trash() {
    ArenaNode *a = head;
    while (a != nullptr) {
        ArenaNode *rm = a;
        a = a->next;
        mem_free(rm);
    }
}

void *Arena::bump(u64 size) {
    if (head == nullptr) {
        head = arena_block_make(size);
    }

    u64 next = 0;
    do {
        next = align_forward(head->allocd, 16);
        if (next + size <= head->capacity) {
            break;
        }

        ArenaNode *block = arena_block_make(size);
        block->next = head;

        head = block;
    } while (true);

    void *ptr = &head->buf[next];
    head->allocd = next + size;
    head->prev = next;
    return ptr;
}

void *Arena::rebump(void *ptr, u64 old, u64 size) {
    if (head == nullptr || ptr == nullptr || old == 0) {
        return bump(size);
    }

    if (&head->buf[head->prev] == ptr) {
        u64 resize = head->prev + size;
        if (resize <= head->capacity) {
            head->allocd = resize;
            return ptr;
        }
    }

    void *new_ptr = bump(size);

    u64 copy = old < size ? old : size;
    memmove(new_ptr, ptr, copy);

    return new_ptr;
}

String Arena::bump_string(String s) {
    if (s.len > 0) {
        char *cstr = (char *)bump(s.len + 1);
        memcpy(cstr, s.data, s.len);
        cstr[s.len] = '\0';
        return {cstr, s.len};
    } else {
        return {};
    }
}

Scanner::Scanner(String str) {
    data = str.data;
    len = str.len;
    pos = 0;
    end = 0;
}

static void advance(Scanner *s) { s->end += utf8_size(s->data[s->end]); }
static bool at_end(Scanner *s) { return s->end >= s->len; }

static Rune peek(Scanner *s) {
    if (at_end(s)) {
        return {0};
    } else {
        return rune_from_string(&s->data[s->end]);
    }
}

static void skip_whitespace(Scanner *s) {
    while (peek(s).is_whitespace() && !at_end(s)) {
        advance(s);
    }
}

String Scanner::next_string() {
    skip_whitespace(this);
    pos = end;

    if (at_end(this)) {
        return "";
    }

    while (!peek(this).is_whitespace() && !at_end(this)) {
        advance(this);
    }

    return {&data[pos], end - pos};
}

i32 Scanner::next_int() {
    skip_whitespace(this);
    pos = end;

    if (at_end(this)) {
        return 0;
    }

    i32 sign = 1;
    if (peek(this).value == '-') {
        sign = -1;
        advance(this);
    }

    i32 num = 0;
    while (peek(this).is_digit()) {
        num *= 10;
        num += peek(this).value - '0';
        advance(this);
    }

    return num * sign;
}

/*========================
// neko_byte_buffer
========================*/

void neko_byte_buffer_init(neko_byte_buffer_t *buffer) {
    buffer->data = (u8 *)mem_alloc(NEKO_BYTE_BUFFER_DEFAULT_CAPCITY);
    buffer->capacity = NEKO_BYTE_BUFFER_DEFAULT_CAPCITY;
    buffer->size = 0;
    buffer->position = 0;
}

neko_byte_buffer_t neko_byte_buffer_new() {
    neko_byte_buffer_t buffer;
    neko_byte_buffer_init(&buffer);
    return buffer;
}

void neko_byte_buffer_free(neko_byte_buffer_t *buffer) {
    if (buffer && buffer->data) {
        mem_free(buffer->data);
    }
}

void neko_byte_buffer_clear(neko_byte_buffer_t *buffer) {
    buffer->size = 0;
    buffer->position = 0;
}

bool neko_byte_buffer_empty(neko_byte_buffer_t *buffer) { return (buffer->size == 0); }

size_t neko_byte_buffer_size(neko_byte_buffer_t *buffer) { return buffer->size; }

void neko_byte_buffer_resize(neko_byte_buffer_t *buffer, size_t sz) {

    // if (sz == 4096) NEKO_ASSERT(0);

    u8 *data = (u8 *)mem_realloc(buffer->data, sz);

    if (data == NULL) {
        return;
    }

    buffer->data = data;
    buffer->capacity = (u32)sz;
}

void neko_byte_buffer_copy_contents(neko_byte_buffer_t *dst, neko_byte_buffer_t *src) {
    neko_byte_buffer_seek_to_beg(dst);
    neko_byte_buffer_seek_to_beg(src);
    neko_byte_buffer_write_bulk(dst, src->data, src->size);
}

void neko_byte_buffer_seek_to_beg(neko_byte_buffer_t *buffer) { buffer->position = 0; }

void neko_byte_buffer_seek_to_end(neko_byte_buffer_t *buffer) { buffer->position = buffer->size; }

void neko_byte_buffer_advance_position(neko_byte_buffer_t *buffer, size_t sz) { buffer->position += (u32)sz; }

void neko_byte_buffer_write_bulk(neko_byte_buffer_t *buffer, void *src, size_t size) {
    // 检查是否需要调整大小
    size_t total_write_size = buffer->position + size;
    if (total_write_size >= (size_t)buffer->capacity) {
        size_t capacity = buffer->capacity * 2;
        while (capacity <= total_write_size) {
            capacity *= 2;
        }

        neko_byte_buffer_resize(buffer, capacity);
    }

    // memcpy data
    memcpy((buffer->data + buffer->position), src, size);

    buffer->size += (u32)size;
    buffer->position += (u32)size;
}

void neko_byte_buffer_read_bulk(neko_byte_buffer_t *buffer, void **dst, size_t size) {
    memcpy(*dst, (buffer->data + buffer->position), size);
    buffer->position += (u32)size;
}

void neko_byte_buffer_write_str(neko_byte_buffer_t *buffer, const char *str) {
    // 写入字符串的大小
    u32 str_len = neko_string_length(str);
    neko_byte_buffer_write(buffer, uint16_t, str_len);

    size_t i;
    for (i = 0; i < str_len; ++i) {
        neko_byte_buffer_write(buffer, u8, str[i]);
    }
}

void neko_byte_buffer_read_str(neko_byte_buffer_t *buffer, char *str) {
    // 从缓冲区读取字符串的大小
    uint16_t sz;
    neko_byte_buffer_read(buffer, uint16_t, &sz);

    u32 i;
    for (i = 0; i < sz; ++i) {
        neko_byte_buffer_read(buffer, u8, &str[i]);
    }
    str[i] = '\0';
}

bool neko_byte_buffer_write_to_file(neko_byte_buffer_t *buffer, const char *output_path) { return neko_os_write_file_contents(output_path, "wb", buffer->data, buffer->size); }

bool neko_byte_buffer_read_from_file(neko_byte_buffer_t *buffer, const char *file_path) {
    if (!buffer) return false;

    if (buffer->data) {
        neko_byte_buffer_free(buffer);
    }

    buffer->data = (u8 *)neko_os_read_file_contents(file_path, "rb", (size_t *)&buffer->size);
    if (!buffer->data) {
        NEKO_ASSERT(false);
        return false;
    }

    buffer->position = 0;
    buffer->capacity = buffer->size;
    return true;
}

void neko_byte_buffer_memset(neko_byte_buffer_t *buffer, u8 val) { memset(buffer->data, val, buffer->capacity); }

/*========================
// Dynamic Array
========================*/

void *neko_dyn_array_resize_impl(void *arr, size_t sz, size_t amount) {
    size_t capacity;

    if (arr) {
        capacity = amount;
    } else {
        capacity = 0;
    }

    // 仅使用标头信息创建新的 neko_dyn_array
    neko_dyn_array *data = (neko_dyn_array *)mem_realloc(arr ? neko_dyn_array_head(arr) : 0, capacity * sz + sizeof(neko_dyn_array));

    if (data) {
        if (!arr) {
            data->size = 0;
        }
        data->capacity = (i32)capacity;
        return ((i32 *)data + 2);
    }

    return NULL;
}

void **neko_dyn_array_init(void **arr, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array *data = (neko_dyn_array *)mem_alloc(val_len + sizeof(neko_dyn_array));  // Allocate capacity of one
        data->size = 0;
        data->capacity = 1;
        *arr = ((i32 *)data + 2);
    }
    return arr;
}

void neko_dyn_array_push_data(void **arr, void *val, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array_init(arr, val_len);
    }
    if (neko_dyn_array_need_grow(*arr, 1)) {
        i32 capacity = neko_dyn_array_capacity(*arr) * 2;

        // Create new neko_dyn_array with just the header information
        neko_dyn_array *data = (neko_dyn_array *)mem_realloc(neko_dyn_array_head(*arr), capacity * val_len + sizeof(neko_dyn_array));

        if (data) {
            data->capacity = capacity;
            *arr = ((i32 *)data + 2);
        }
    }
    size_t offset = neko_dyn_array_size(*arr);
    memcpy(((u8 *)(*arr)) + offset * val_len, val, val_len);
    neko_dyn_array_head(*arr)->size++;
}

/*========================
// Hash Table
========================*/

void __neko_hash_table_init_impl(void **ht, size_t sz) { *ht = mem_alloc(sz); }

/*========================
// Slot Array
========================*/

void **neko_slot_array_init(void **sa, size_t sz) {
    if (*sa == NULL) {
        *sa = mem_alloc(sz);
        memset(*sa, 0, sz);
        return sa;
    } else {
        return NULL;
    }
}

/*========================
// Slot Map
========================*/

void **neko_slot_map_init(void **sm) {
    if (*sm == NULL) {
        (*sm) = mem_alloc(sizeof(size_t) * 2);
        memset((*sm), 0, sizeof(size_t) * 2);
        return sm;
    }
    return NULL;
}

// ===============================================================

#define MIN_CAPACITY 2

struct CArray {
    char *buf;              // this is a char * for pointer arithmetic
    unsigned int capacity;  // alloc'd size of buf
    unsigned int length;    // number of objects
    size_t object_size;     // size of each element
};

CArray *array_new_(size_t object_size) {
    CArray *arr;

    arr = (CArray *)mem_alloc(sizeof(CArray));
    arr->object_size = object_size;
    arr->capacity = MIN_CAPACITY;
    arr->buf = (char *)mem_alloc(arr->object_size * arr->capacity);
    arr->length = 0;

    return arr;
}
void array_free(CArray *arr) {
    mem_free(arr->buf);
    mem_free(arr);
}

void *array_get(CArray *arr, unsigned int i) { return arr->buf + arr->object_size * i; }
void *array_top(CArray *arr) { return arr->buf + arr->object_size * (arr->length - 1); }
unsigned int array_length(CArray *arr) { return arr->length; }

void *array_begin(CArray *arr) { return arr->buf; }
void *array_end(CArray *arr) { return arr->buf + arr->object_size * arr->length; }

void *array_add(CArray *arr) {
    // too small? double it
    if (++arr->length > arr->capacity) arr->buf = (char *)mem_realloc(arr->buf, arr->object_size * (arr->capacity = arr->capacity << 1));
    return arr->buf + arr->object_size * (arr->length - 1);
}
void array_reset(CArray *arr, unsigned int num) {
    mem_free(arr->buf);

    arr->length = num;
    arr->capacity = num < MIN_CAPACITY ? MIN_CAPACITY : num;
    arr->buf = (char *)mem_alloc(arr->object_size * arr->capacity);
}
void array_pop(CArray *arr) {
    // too big (> four times as is needed)? halve it
    if (--arr->length << 2 < arr->capacity && arr->capacity > MIN_CAPACITY) arr->buf = (char *)mem_realloc(arr->buf, arr->object_size * (arr->capacity = arr->capacity >> 1));
}

bool array_quick_remove(CArray *arr, unsigned int i) {
    bool ret = false;

    if (i + 1 < arr->length) {
        memcpy(arr->buf + arr->object_size * i, arr->buf + arr->object_size * (arr->length - 1), arr->object_size);
        ret = true;
    }

    array_pop(arr);
    return ret;
}

void array_sort(CArray *arr, int (*compar)(const void *, const void *)) { qsort(arr->buf, arr->length, arr->object_size, compar); }

// -------------------------------------------------------------------------

#ifdef ARRAY_TEST

typedef struct {
    int a, b;
} IntPair;

void dump(CArray *arr) {
    printf("{ (%d, %d) -- ", arr->capacity, arr->length);
    for (unsigned int i = 0; i < arr->length; ++i) {
        IntPair *p = array_get(arr, i);
        printf("(%d, %d) ", p->a, p->b);
    }
    printf("}\n");
}

int int_compare(const void *a, const void *b) {
    const int *ia = a, *ib = b;
    return *ia - *ib;
}

void test_sort() {
    int *i;
    CArray *arr = array_new(int);

    array_add_val(int, arr) = 3;
    array_add_val(int, arr) = 5;
    array_add_val(int, arr) = 1;
    array_add_val(int, arr) = 7;
    array_add_val(int, arr) = 1;
    array_add_val(int, arr) = 0;
    array_add_val(int, arr) = 499;
    array_add_val(int, arr) = 200;

    printf("before sort: ");
    array_foreach(i, arr) printf("%d ", *i);
    printf("\n");

    array_sort(arr, int_compare);

    printf("after sort: ");
    array_foreach(i, arr) printf("%d ", *i);
    printf("\n");

    array_free(arr);
}

int main() {
    CArray *arr = array_new(IntPair);

    // add some
    for (unsigned int i = 0; i < 7; ++i) {
        array_add_val(IntPair, arr) = (IntPair){i, i * i};
        dump(arr);
    }

    // remove some
    array_quick_remove(arr, 2);
    dump(arr);
    array_quick_remove(arr, 4);
    dump(arr);
    while (array_length(arr) > 0) {
        array_quick_remove(arr, 0);
        dump(arr);
    }

    array_free(arr);

    test_sort();

    return 0;
}

#endif

CVec2 vec2_zero = {0.0, 0.0};

CVec2 vec2_add(CVec2 u, CVec2 v) { return vec2(u.x + v.x, u.y + v.y); }
CVec2 vec2_sub(CVec2 u, CVec2 v) { return vec2(u.x - v.x, u.y - v.y); }
CVec2 vec2_mul(CVec2 u, CVec2 v) { return vec2(u.x * v.x, u.y * v.y); }
CVec2 vec2_div(CVec2 u, CVec2 v) { return vec2(u.x / v.x, u.y / v.y); }
CVec2 vec2_scalar_mul(CVec2 v, Scalar f) { return vec2(v.x * f, v.y * f); }
CVec2 vec2_scalar_div(CVec2 v, Scalar f) { return vec2(v.x / f, v.y / f); }
CVec2 scalar_vec2_div(Scalar f, CVec2 v) { return vec2(f / v.x, f / v.y); }
CVec2 vec2_neg(CVec2 v) { return vec2(-v.x, -v.y); }

Scalar vec2_len(CVec2 v) { return scalar_sqrt(v.x * v.x + v.y * v.y); }
CVec2 vec2_normalize(CVec2 v) {
    if (v.x == 0 && v.y == 0) return v;
    return vec2_scalar_div(v, vec2_len(v));
}
Scalar vec2_dot(CVec2 u, CVec2 v) { return u.x * v.x + u.y * v.y; }
Scalar vec2_dist(CVec2 u, CVec2 v) { return vec2_len(vec2_sub(u, v)); }

CVec2 vec2_rot(CVec2 v, Scalar rot) { return vec2(v.x * scalar_cos(rot) - v.y * scalar_sin(rot), v.x * scalar_sin(rot) + v.y * scalar_cos(rot)); }
Scalar vec2_atan2(CVec2 v) { return scalar_atan2(v.y, v.x); }

void vec2_save(CVec2 *v, const char *n, Store *s) {
    Store *t;

    if (store_child_save_compressed(&t, n, s)) {
        scalar_save(&v->x, "x", t);
        scalar_save(&v->y, "y", t);
    }
}
bool vec2_load(CVec2 *v, const char *n, CVec2 d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        scalar_load(&v->x, "x", 0, t);
        scalar_load(&v->y, "y", 0, t);
    } else
        *v = d;
    return t != NULL;
}

#undef vec2
CVec2 vec2(Scalar x, Scalar y) { return CVec2{x, y}; }

CMat3 mat3_mul(CMat3 m, CMat3 n) {
    return mat3(m.m[0][0] * n.m[0][0] + m.m[1][0] * n.m[0][1] + m.m[2][0] * n.m[0][2], m.m[0][1] * n.m[0][0] + m.m[1][1] * n.m[0][1] + m.m[2][1] * n.m[0][2],
                m.m[0][2] * n.m[0][0] + m.m[1][2] * n.m[0][1] + m.m[2][2] * n.m[0][2],

                m.m[0][0] * n.m[1][0] + m.m[1][0] * n.m[1][1] + m.m[2][0] * n.m[1][2], m.m[0][1] * n.m[1][0] + m.m[1][1] * n.m[1][1] + m.m[2][1] * n.m[1][2],
                m.m[0][2] * n.m[1][0] + m.m[1][2] * n.m[1][1] + m.m[2][2] * n.m[1][2],

                m.m[0][0] * n.m[2][0] + m.m[1][0] * n.m[2][1] + m.m[2][0] * n.m[2][2], m.m[0][1] * n.m[2][0] + m.m[1][1] * n.m[2][1] + m.m[2][1] * n.m[2][2],
                m.m[0][2] * n.m[2][0] + m.m[1][2] * n.m[2][1] + m.m[2][2] * n.m[2][2]);
}

CMat3 mat3_scaling_rotation_translation(CVec2 scale, Scalar rot, CVec2 trans) {
    return mat3(scale.x * scalar_cos(rot), scale.x * scalar_sin(rot), 0.0f, scale.y * -scalar_sin(rot), scale.y * scalar_cos(rot), 0.0f, trans.x, trans.y, 1.0f);
}

CVec2 mat3_get_translation(CMat3 m) { return vec2(m.m[2][0], m.m[2][1]); }
Scalar mat3_get_rotation(CMat3 m) { return scalar_atan2(m.m[0][1], m.m[0][0]); }
CVec2 mat3_get_scale(CMat3 m) { return vec2(scalar_sqrt(m.m[0][0] * m.m[0][0] + m.m[0][1] * m.m[0][1]), scalar_sqrt(m.m[1][0] * m.m[1][0] + m.m[1][1] * m.m[1][1])); }

CMat3 mat3_inverse(CMat3 m) {
    Scalar det;
    CMat3 inv;

    inv.m[0][0] = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];
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

CVec2 mat3_transform(CMat3 m, CVec2 v) { return vec2(m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0], m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1]); }

void mat3_save(CMat3 *m, const char *n, Store *s) {
    Store *t;
    unsigned int i, j;

    if (store_child_save_compressed(&t, n, s))
        for (i = 0; i < 3; ++i)
            for (j = 0; j < 3; ++j) scalar_save(&m->m[i][j], NULL, t);
}
bool mat3_load(CMat3 *m, const char *n, CMat3 d, Store *s) {
    Store *t;
    unsigned int i, j;

    if (store_child_load(&t, n, s))
        for (i = 0; i < 3; ++i)
            for (j = 0; j < 3; ++j) scalar_load(&m->m[i][j], NULL, 0, t);
    else
        *m = d;
    return t != NULL;
}

#undef mat3_identity
CMat3 mat3_identity() { return mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f); }

#undef mat3
CMat3 mat3(Scalar m00, Scalar m01, Scalar m02, Scalar m10, Scalar m11, Scalar m12, Scalar m20, Scalar m21, Scalar m22) { return CMat3{{{m00, m01, m02}, {m10, m11, m12}, {m20, m21, m22}}}; }

BBox bbox_merge(BBox a, BBox b) { return bbox(vec2(scalar_min(a.min.x, b.min.x), scalar_min(a.min.y, b.min.y)), vec2(scalar_max(a.max.x, b.max.x), scalar_max(a.max.y, b.max.y))); }
BBox bbox_bound(CVec2 a, CVec2 b) { return bbox(vec2(scalar_min(a.x, b.x), scalar_min(a.y, b.y)), vec2(scalar_max(a.x, b.x), scalar_max(a.y, b.y))); }
bool bbox_contains(BBox b, CVec2 p) { return b.min.x <= p.x && p.x <= b.max.x && b.min.y <= p.y && p.y <= b.max.y; }

BBox bbox(CVec2 min, CVec2 max) {
    BBox bb;
    bb.min = min;
    bb.max = max;
    return bb;
}

BBox bbox_transform(CMat3 m, BBox b) {
    CVec2 v1, v2, v3, v4;

    v1 = mat3_transform(m, vec2(b.min.x, b.min.y));
    v2 = mat3_transform(m, vec2(b.max.x, b.min.y));
    v3 = mat3_transform(m, vec2(b.max.x, b.max.y));
    v4 = mat3_transform(m, vec2(b.min.x, b.max.y));

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
Color color_opaque(Scalar r, Scalar g, Scalar b) { return color(r, g, b, 1); }

#undef color
Color color(Scalar r, Scalar g, Scalar b, Scalar a) { return Color{r, g, b, a}; }

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

static void _stream_deinit(Stream *sm) { mem_free(sm->buf); }

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
    _stream_deinit(s->sm);
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
    } else if (sm->buf[sm->pos] != '{')
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
    FILE *f;
    unsigned int n;
    char *str;

    f = fopen(filename, "r");
    error_assert(f, "file '%s' must be open for reading", filename);

    fscanf(f, "%u\n", &n);
    str = (char *)mem_alloc(n + 1);
    fread(str, 1, n, f);
    fclose(f);
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
