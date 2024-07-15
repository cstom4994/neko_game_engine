#include "neko_base.h"

#include <stdarg.h>
#include <stdio.h>

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