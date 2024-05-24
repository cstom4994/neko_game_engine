

#include "neko_common.h"

#include "engine/neko.h"

// Base64
// https://blog.csdn.net/qq_26093511/article/details/78836087

unsigned char* neko_base64_encode(unsigned char* str) {
    long len, str_len;
    unsigned char* res;
    int i, j;
    // 定义base64编码表
    unsigned char* base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // 计算经过base64编码后的字符串长度
    str_len = neko_strlen(str);
    if (str_len % 3 == 0)
        len = str_len / 3 * 4;
    else
        len = (str_len / 3 + 1) * 4;

    res = neko_safe_malloc(sizeof(unsigned char) * len + 1);
    res[len] = '\0';

    // 以3个8位字符为一组进行编码
    for (i = 0, j = 0; i < len - 2; j += 3, i += 4) {
        res[i] = base64_table[str[j] >> 2];                                      // 取出第一个字符的前6位并找出对应的结果字符
        res[i + 1] = base64_table[(str[j] & 0x3) << 4 | (str[j + 1] >> 4)];      // 将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符
        res[i + 2] = base64_table[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)];  // 将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符
        res[i + 3] = base64_table[str[j + 2] & 0x3f];                            // 取出第三个字符的后6位并找出结果字符
    }

    switch (str_len % 3) {
        case 1:
            res[i - 2] = '=';
            res[i - 1] = '=';
            break;
        case 2:
            res[i - 1] = '=';
            break;
    }

    return res;
}

unsigned char* neko_base64_decode(unsigned char* code) {
    // 根据base64表，以字符找到对应的十进制数据
    int table[] = {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  62, 0,  0,  0,  63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                   17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  0,  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    long len, str_len;
    unsigned char* res;
    int i, j;

    // 计算解码后的字符串长度
    len = neko_strlen(code);
    // 判断编码后的字符串后是否有=
    if (strstr(code, "=="))
        str_len = len / 4 * 3 - 2;
    else if (strstr(code, "="))
        str_len = len / 4 * 3 - 1;
    else
        str_len = len / 4 * 3;

    res = neko_safe_malloc(sizeof(unsigned char) * str_len + 1);
    res[str_len] = '\0';

    // 以4个字符为一位进行解码
    for (i = 0, j = 0; i < len - 2; j += 3, i += 4) {
        // 取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合
        res[j] = ((unsigned char)table[code[i]]) << 2 | (((unsigned char)table[code[i + 1]]) >> 4);
        // 取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合
        res[j + 1] = (((unsigned char)table[code[i + 1]]) << 4) | (((unsigned char)table[code[i + 2]]) >> 2);
        // 取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合
        res[j + 2] = (((unsigned char)table[code[i + 2]]) << 6) | ((unsigned char)table[code[i + 3]]);
    }

    return res;
}

#define HASHTABLE_SIZE_T size_t

#define HASHTABLE_MEMSET(ptr, val, cnt) (memset(ptr, val, cnt))
#define HASHTABLE_MEMCPY(dst, src, cnt) (memcpy(dst, src, cnt))
#define HASHTABLE_MALLOC(ctx, size) (malloc(size))
#define HASHTABLE_FREE(ctx, ptr) (free(ptr))

static HASHTABLE_U32 hashtable_internal_pow2ceil(HASHTABLE_U32 v) {
    --v;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    ++v;
    v += (v == 0);
    return v;
}

void hashtable_init(hashtable_t* table, int item_size, int initial_capacity, void* memctx) {
    initial_capacity = (int)hashtable_internal_pow2ceil(initial_capacity >= 0 ? (HASHTABLE_U32)initial_capacity : 32U);
    table->memctx = memctx;
    table->count = 0;
    table->item_size = item_size;
    table->slot_capacity = (int)hashtable_internal_pow2ceil((HASHTABLE_U32)(initial_capacity + initial_capacity / 2));
    int slots_size = (int)(table->slot_capacity * sizeof(*table->slots));
    table->slots = (struct hashtable_internal_slot_t*)HASHTABLE_MALLOC(table->memctx, (HASHTABLE_SIZE_T)slots_size);
    neko_assert(table->slots);
    HASHTABLE_MEMSET(table->slots, 0, (HASHTABLE_SIZE_T)slots_size);
    table->item_capacity = (int)hashtable_internal_pow2ceil((HASHTABLE_U32)initial_capacity);
    table->items_key = (HASHTABLE_U64*)HASHTABLE_MALLOC(table->memctx, table->item_capacity * (sizeof(*table->items_key) + sizeof(*table->items_slot) + table->item_size) + table->item_size);
    neko_assert(table->items_key);
    table->items_slot = (int*)(table->items_key + table->item_capacity);
    table->items_data = (void*)(table->items_slot + table->item_capacity);
    table->swap_temp = (void*)(((uintptr_t)table->items_data) + table->item_size * table->item_capacity);
}

void hashtable_term(hashtable_t* table) {
    HASHTABLE_FREE(table->memctx, table->items_key);
    HASHTABLE_FREE(table->memctx, table->slots);
}

// from https://gist.github.com/badboy/6267743
static HASHTABLE_U32 hashtable_internal_calculate_hash(HASHTABLE_U64 key) {
    key = (~key) + (key << 18);
    key = key ^ (key >> 31);
    key = key * 21;
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);
    neko_assert(key);
    return (HASHTABLE_U32)key;
}

static int hashtable_internal_find_slot(hashtable_t const* table, HASHTABLE_U64 key) {
    int const slot_mask = table->slot_capacity - 1;
    HASHTABLE_U32 const hash = hashtable_internal_calculate_hash(key);

    int const base_slot = (int)(hash & (HASHTABLE_U32)slot_mask);
    int base_count = table->slots[base_slot].base_count;
    int slot = base_slot;

    while (base_count > 0) {
        HASHTABLE_U32 slot_hash = table->slots[slot].key_hash;
        if (slot_hash) {
            int slot_base = (int)(slot_hash & (HASHTABLE_U32)slot_mask);
            if (slot_base == base_slot) {
                neko_assert(base_count > 0);
                --base_count;
                if (slot_hash == hash && table->items_key[table->slots[slot].item_index] == key) return slot;
            }
        }
        slot = (slot + 1) & slot_mask;
    }

    return -1;
}

static void hashtable_internal_expand_slots(hashtable_t* table) {
    int const old_capacity = table->slot_capacity;
    struct hashtable_internal_slot_t* old_slots = table->slots;

    table->slot_capacity *= 2;
    int const slot_mask = table->slot_capacity - 1;

    int const size = (int)(table->slot_capacity * sizeof(*table->slots));
    table->slots = (struct hashtable_internal_slot_t*)HASHTABLE_MALLOC(table->memctx, (HASHTABLE_SIZE_T)size);
    neko_assert(table->slots);
    HASHTABLE_MEMSET(table->slots, 0, (HASHTABLE_SIZE_T)size);

    for (int i = 0; i < old_capacity; ++i) {
        HASHTABLE_U32 const hash = old_slots[i].key_hash;
        if (hash) {
            int const base_slot = (int)(hash & (HASHTABLE_U32)slot_mask);
            int slot = base_slot;
            while (table->slots[slot].key_hash) slot = (slot + 1) & slot_mask;
            table->slots[slot].key_hash = hash;
            int item_index = old_slots[i].item_index;
            table->slots[slot].item_index = item_index;
            table->items_slot[item_index] = slot;
            ++table->slots[base_slot].base_count;
        }
    }

    HASHTABLE_FREE(table->memctx, old_slots);
}

static void hashtable_internal_expand_items(hashtable_t* table) {
    table->item_capacity *= 2;
    HASHTABLE_U64* const new_items_key =
            (HASHTABLE_U64*)HASHTABLE_MALLOC(table->memctx, table->item_capacity * (sizeof(*table->items_key) + sizeof(*table->items_slot) + table->item_size) + table->item_size);
    neko_assert(new_items_key);

    int* const new_items_slot = (int*)(new_items_key + table->item_capacity);
    void* const new_items_data = (void*)(new_items_slot + table->item_capacity);
    void* const new_swap_temp = (void*)(((uintptr_t)new_items_data) + table->item_size * table->item_capacity);

    HASHTABLE_MEMCPY(new_items_key, table->items_key, table->count * sizeof(*table->items_key));
    HASHTABLE_MEMCPY(new_items_slot, table->items_slot, table->count * sizeof(*table->items_key));
    HASHTABLE_MEMCPY(new_items_data, table->items_data, (HASHTABLE_SIZE_T)table->count * table->item_size);

    HASHTABLE_FREE(table->memctx, table->items_key);

    table->items_key = new_items_key;
    table->items_slot = new_items_slot;
    table->items_data = new_items_data;
    table->swap_temp = new_swap_temp;
}

void* hashtable_insert(hashtable_t* table, HASHTABLE_U64 key, void const* item) {
    neko_assert(hashtable_internal_find_slot(table, key) < 0);

    if (table->count >= (table->slot_capacity - table->slot_capacity / 3)) hashtable_internal_expand_slots(table);

    int const slot_mask = table->slot_capacity - 1;
    HASHTABLE_U32 const hash = hashtable_internal_calculate_hash(key);

    int const base_slot = (int)(hash & (HASHTABLE_U32)slot_mask);
    int base_count = table->slots[base_slot].base_count;
    int slot = base_slot;
    int first_free = slot;
    while (base_count) {
        HASHTABLE_U32 const slot_hash = table->slots[slot].key_hash;
        if (slot_hash == 0 && table->slots[first_free].key_hash != 0) first_free = slot;
        int slot_base = (int)(slot_hash & (HASHTABLE_U32)slot_mask);
        if (slot_base == base_slot) --base_count;
        slot = (slot + 1) & slot_mask;
    }

    slot = first_free;
    while (table->slots[slot].key_hash) slot = (slot + 1) & slot_mask;

    if (table->count >= table->item_capacity) hashtable_internal_expand_items(table);

    neko_assert(!table->slots[slot].key_hash && (hash & (HASHTABLE_U32)slot_mask) == (HASHTABLE_U32)base_slot);
    neko_assert(hash);
    table->slots[slot].key_hash = hash;
    table->slots[slot].item_index = table->count;
    ++table->slots[base_slot].base_count;

    void* dest_item = (void*)(((uintptr_t)table->items_data) + table->count * table->item_size);
    memcpy(dest_item, item, (HASHTABLE_SIZE_T)table->item_size);
    table->items_key[table->count] = key;
    table->items_slot[table->count] = slot;
    ++table->count;

    return dest_item;
}

void hashtable_remove(hashtable_t* table, HASHTABLE_U64 key) {
    int const slot = hashtable_internal_find_slot(table, key);
    neko_assert(slot >= 0);

    int const slot_mask = table->slot_capacity - 1;
    HASHTABLE_U32 const hash = table->slots[slot].key_hash;
    int const base_slot = (int)(hash & (HASHTABLE_U32)slot_mask);
    neko_assert(hash);
    --table->slots[base_slot].base_count;
    table->slots[slot].key_hash = 0;

    int index = table->slots[slot].item_index;
    int last_index = table->count - 1;
    if (index != last_index) {
        table->items_key[index] = table->items_key[last_index];
        table->items_slot[index] = table->items_slot[last_index];
        void* dst_item = (void*)(((uintptr_t)table->items_data) + index * table->item_size);
        void* src_item = (void*)(((uintptr_t)table->items_data) + last_index * table->item_size);
        HASHTABLE_MEMCPY(dst_item, src_item, (HASHTABLE_SIZE_T)table->item_size);
        table->slots[table->items_slot[last_index]].item_index = index;
    }
    --table->count;
}

void hashtable_clear(hashtable_t* table) {
    table->count = 0;
    HASHTABLE_MEMSET(table->slots, 0, table->slot_capacity * sizeof(*table->slots));
}

void* hashtable_find(hashtable_t const* table, HASHTABLE_U64 key) {
    int const slot = hashtable_internal_find_slot(table, key);
    if (slot < 0) return 0;

    int const index = table->slots[slot].item_index;
    void* const item = (void*)(((uintptr_t)table->items_data) + index * table->item_size);
    return item;
}

int hashtable_count(hashtable_t const* table) { return table->count; }

void* hashtable_items(hashtable_t const* table) { return table->items_data; }

HASHTABLE_U64 const* hashtable_keys(hashtable_t const* table) { return table->items_key; }

void hashtable_swap(hashtable_t* table, int index_a, int index_b) {
    if (index_a < 0 || index_a >= table->count || index_b < 0 || index_b >= table->count) return;

    int slot_a = table->items_slot[index_a];
    int slot_b = table->items_slot[index_b];

    table->items_slot[index_a] = slot_b;
    table->items_slot[index_b] = slot_a;

    HASHTABLE_U64 temp_key = table->items_key[index_a];
    table->items_key[index_a] = table->items_key[index_b];
    table->items_key[index_b] = temp_key;

    void* item_a = (void*)(((uintptr_t)table->items_data) + index_a * table->item_size);
    void* item_b = (void*)(((uintptr_t)table->items_data) + index_b * table->item_size);
    HASHTABLE_MEMCPY(table->swap_temp, item_a, table->item_size);
    HASHTABLE_MEMCPY(item_a, item_b, table->item_size);
    HASHTABLE_MEMCPY(item_b, table->swap_temp, table->item_size);

    table->slots[slot_a].item_index = index_b;
    table->slots[slot_b].item_index = index_a;
}

/*

    int main( int argc, char** argv ) {
        // define some example key and value types
        typedef struct key_t { int a, b, c; } key_t;
        typedef struct value_t {
            char id[ 64 ];
            float x, y, z;
            int n[ 250 ];
        } value_t;

        // create a couple of sample keys
        // (don't bother to fill in the fields for this sample)
        key_t* key_a = (key_t*)malloc( sizeof( key_t ) );
        key_t* key_b = (key_t*)malloc( sizeof( key_t ) );

        hashtable_t table;
        hashtable_init( &table, sizeof( value_t ), 256, 0 );

        {
        // values are copied into the table, not stored by pointer
        // (don't bother to fill in all the fields for this sample)
        value_t value_a = { "Item A" };
        value_t value_b = { "Item B" };
        hashtable_insert( &table, (HASHTABLE_U64)(uintptr_t)key_a, &value_a );
        hashtable_insert( &table, (HASHTABLE_U64)(uintptr_t)key_b, &value_b );
        }

        // find the values by key
        value_t* value_a = (value_t*)hashtable_find( &table, (HASHTABLE_U64)(uintptr_t)key_a );
        printf( "First item: %s\n", value_a->id );
        value_t* value_b = (value_t*)hashtable_find( &table, (HASHTABLE_U64)(uintptr_t)key_b );
        printf( "Second item: %s\n", value_b->id );

        // remove one of the items
        hashtable_remove( &table, (HASHTABLE_U64)(uintptr_t)key_a );

        // it is possible to enumerate keys and values
        int count = hashtable_count( &table );
        HASHTABLE_U64 const* keys = hashtable_keys( &table );
        value_t* items = (value_t*)hashtable_items( &table );
        printf( "\nEnumeration:\n" );
        for( int i = 0; i < count; ++i ) {
            printf( "  0x%X : %s\n", (int) keys[ i ], items[ i ].id );
        }

        // cleanup
        hashtable_term( &table );
        free( key_b );
        free( key_a );
        return 0;
    }

*/

s32 neko_sexpr_node_fits_format(neko_snode_t* node, neko_snode_t* fmt) {
    if (!node) return 0;

    if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        for (s32 i = node->node_len && node->node->type == NEKO_SEXPR_TYPE_STRING; i < node->node_len; i++) {
            if (node->node[i].type == NEKO_SEXPR_TYPE_ARRAY && node->node[i].node_len && node->node[i].node->type == NEKO_SEXPR_TYPE_STRING) {
                if (!neko_sexpr_node_get_tagged(fmt, node->node[i].node->str)) return 0;
            } else {
                neko_snode_t* res = neko_sexpr_node_get_index(fmt, i);
                if (!res) return 0;
                if (node->node[i].type != NEKO_SEXPR_TYPE_ARRAY && res->type == NEKO_SEXPR_TYPE_ARRAY && res->node_len == 2) {
                    if (res->node[1].type != node->node[i].type) return 0;
                } else if (node->node[i].type != res->type) {
                    return 0;
                }
            }
        }
        return 1;
    }
    return node->type == fmt->type;
}

neko_snode_t* neko_sexpr_node_get_value(neko_snode_t* node, s32* len) {
    if (!node) return NULL;
    if (len) *len = 1;
    if (node->type != NEKO_SEXPR_TYPE_ARRAY) return node;

    if (!node->node || node->node_len < 2 || node->node->type != NEKO_SEXPR_TYPE_STRING) return node->node;
    if (len) *len = node->node_len - 1;
    return node->node + 1;
}

neko_snode_t* neko_sexpr_node_get_tagged(neko_snode_t* node, const_str tag) {
    if (!node || node->type != NEKO_SEXPR_TYPE_ARRAY || !node->node) return NULL;

    for (s32 i = 0; i < node->node_len; i++)
        if (node->node[i].type == NEKO_SEXPR_TYPE_ARRAY && node->node[i].node_len && node->node[i].node->type == NEKO_SEXPR_TYPE_STRING) {
            if (strcmp(tag, node->node[i].node->str) == 0) return node->node + i;
        }
    return NULL;
}

neko_snode_t* neko_sexpr_node_get_index(neko_snode_t* node, s32 index) {
    if (!node || node->type != NEKO_SEXPR_TYPE_ARRAY || !node->node || index < 0 || index >= node->node_len) return NULL;
    return node->node + index;
}

static void neko_sexpr_write_nde(FILE* fd, const neko_snode_t* node, s32 root, s32 indents, s32 new_line_start, s32 first) {
    if (!node) {
        neko_log_warning("neko S-expression error: node was NULL");
        return;
    }

    if ((new_line_start && (!first || root)) || node->new_line_at_start) {
        fprintf(fd, "\n");
        for (s32 i = 0; i < indents; i++) fprintf(fd, "    ");
    } else if (!root && !first) {
        fprintf(fd, " ");
    }

    if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        fprintf(fd, "(");
    } else if (root) {
        neko_log_warning("neko S-expression error: a root node was not an array");
        return;
    }

    if (node->type == NEKO_SEXPR_TYPE_NONE) {
        neko_log_warning("neko S-expression error: skipping writing an uninitialized member");
    } else if (node->type == NEKO_SEXPR_TYPE_INT) {
        fprintf(fd, "%d", node->i);
    } else if (node->type == NEKO_SEXPR_TYPE_FLOAT) {
        fprintf(fd, "%f", node->f);
    } else if (node->type == NEKO_SEXPR_TYPE_STRING) {
        s32 res = 0;
        for (s32 i = 0; i < strlen(node->str); i++) {
            if (i == 0 && memchr("1234567890.-", node->str[i], sizeof("1234567890.-"))) {
                res = 1;
                break;
            }
            if (memchr("\n '\t\v()\r", node->str[i], sizeof("\n '\t\v()\r"))) {
                res = 1;
                break;
            }
        }
        if (strchr(node->str, '`')) {
            neko_log_warning("neko S-expression error: string '%s' contains a `, sexpr does not support this", node->str);
        } else {
            if (res)
                fprintf(fd, "`%s`", node->str);
            else
                fprintf(fd, "%s", node->str);
        }
    } else if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        for (s32 i = 0; i < node->node_len; i++) neko_sexpr_write_nde(fd, node->node + i, 0, indents + 1, node->new_line_at_end_of_subsequent_elements, i == 0);
    }

    if (node->new_line_at_end_of_subsequent_elements) {
        fprintf(fd, "\n");
        for (s32 i = 0; i < indents; i++) fprintf(fd, "    ");
    }
    if (node->type == NEKO_SEXPR_TYPE_ARRAY) fprintf(fd, ")");
}

s32 neko_sexpr_write_to_file(char* filename, const neko_snode_t node) {
    if (node.type != NEKO_SEXPR_TYPE_ARRAY) return 0;

    FILE* fd = fopen(filename, "wb");
    if (!fd) {
        neko_log_warning("neko S-expression error: unable to open file");
        return 0;
    }

    for (s32 i = 0; i < node.node_len; i++) neko_sexpr_write_nde(fd, node.node + i, 1, 0, i > 0, 1);

    fclose(fd);
    return 1;
}

neko_snode_t neko_sexpr_parse_file(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        neko_log_warning("neko S-expression error: unable to open file");
        return (neko_snode_t){0};
    }

    fseek(file, 0L, SEEK_END);
    s32 readsize = ftell(file);
    rewind(file);

    char* buffer = neko_safe_malloc(readsize);
    if (!buffer) return (neko_snode_t){0};

    fread(buffer, 1, readsize, file);
    fclose(file);

    return neko_sexpr_parse_memory(buffer, readsize);
}

static s32 neko_sexpr_seek_whitespace(char* mem, s32 index, s32 sz) {
    while (index < sz && !memchr("\n '\t()\v\r", mem[index], sizeof("\n '\t()\v\r"))) index++;
    return index;
}

static s32 neko_sexpr_parse_memory_array(char* mem, s32 sz, neko_snode_t* node) {
    s32 index = 0;
    if (index >= sz) return index;
    if (mem[index] == '(') index++;
    if (index >= sz) return index;

    node->type = NEKO_SEXPR_TYPE_ARRAY;
    node->is_node_allocated = 1;
    s32 new_line_on_all_elements = 1;

    while (index < sz) {
        s32 last_index = index;

        while (index < sz && memchr("\n \t\v\r", mem[index], sizeof("\n \t\v\r"))) index++;

        if (mem[index] == ')') {
            if (node->node_len <= 1) new_line_on_all_elements = 0;
            break;
        }

        s32 i = ++node->node_len - 1;
        node->node = neko_safe_realloc(node->node, sizeof(*node->node) * node->node_len);
        node->node[i] = (neko_snode_t){0};

        if (index != last_index && memchr(mem + last_index, '\n', index - last_index)) node->node[i].new_line_at_start = 1;

        if (memchr("1234567890.-", mem[index], sizeof("1234567890.-"))) {
            s32 number_end = neko_sexpr_seek_whitespace(mem, index, sz);

            char tmp = mem[number_end];
            mem[number_end] = 0;
            if (memchr(mem + index, '.', number_end - index)) {  // 浮点型
                node->node[i].f = atof(mem + index);
                node->node[i].type = NEKO_SEXPR_TYPE_FLOAT;
            } else /* s32 */ {
                node->node[i].i = atoi(mem + index);
                node->node[i].type = NEKO_SEXPR_TYPE_INT;
            }
            mem[number_end] = tmp;
            index = number_end;
        } else if (mem[index] == '(') {  // 数组
            index += neko_sexpr_parse_memory_array(mem + index, sz - index, node->node + i);
        } else if (mem[index] == '`') {  // 字符串
            char* end;
            s32 times = 0;
            do {
                times++;
                end = (sz - index - 1 < 0) ? NULL : memchr(mem + index + times, '`', sz - index - times);
            } while (end && end[-1] == '\\');
            if (!end) {
                neko_log_warning("neko S-expression error: unable to find closing quote");
                return sz;
            }
            size_t strsz = (end) - (mem + index + 1);

            node->node[i].str = neko_safe_malloc(strsz + 1);
            node->node[i].is_str_allocated = 1;

            memcpy(node->node[i].str, mem + index + 1, strsz);
            node->node[i].str[strsz] = 0;

            index += strsz + 2;
            node->node[i].type = NEKO_SEXPR_TYPE_STRING;
        } else {  // 处理不带 " " 的字符串
            s32 end = neko_sexpr_seek_whitespace(mem, index, sz);

            size_t strsz = (end - index);

            node->node[i].str = neko_safe_malloc(strsz + 1);
            node->node[i].is_str_allocated = 1;

            memcpy(node->node[i].str, mem + index, strsz);
            node->node[i].str[strsz] = 0;

            index += strsz;
            node->node[i].type = NEKO_SEXPR_TYPE_STRING;
        }

        if (mem[index] == '\n' && node->node[i].type == NEKO_SEXPR_TYPE_ARRAY && !node->node[i].new_line_at_start) node->node[i].new_line_at_end = 1;

        if ((!node->node[i].new_line_at_start || node->node[i].new_line_at_end) && i != 0) new_line_on_all_elements = 0;
    }
    node->new_line_at_end_of_subsequent_elements = new_line_on_all_elements;

    if (index >= sz) {
        neko_log_warning("neko S-expression error: unable to find closing parenthesis");
        return sz;
    }
    return index + 1;
}

neko_snode_t neko_sexpr_parse_memory(char* mem, s32 sz) {
    s32 index = 0;
    neko_snode_t node = {.type = NEKO_SEXPR_TYPE_ARRAY};
    node.node_len = 0;
    while (index < sz) {
        char* new_pos = memchr(mem + index, '(', sz - index);
        if (!new_pos) return node;

        index += (s32)(new_pos - (mem + index));

        node.node_len += 1;
        node.node = neko_safe_realloc(node.node, sizeof(*node.node) * node.node_len);
        node.node[node.node_len - 1] = (neko_snode_t){0};

        index += neko_sexpr_parse_memory_array(mem + index, sz - index, node.node + (node.node_len - 1));
    }
    if (node.node_len) node.is_node_allocated = 1;
    return node;
}

void neko_sexpr_free_node(neko_snode_t* node) {

    if (node->type == NEKO_SEXPR_TYPE_STRING && node->is_str_allocated) {
        neko_safe_free(node->str);
    } else if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        for (s32 i = 0; i < node->node_len; i++) neko_sexpr_free_node(node->node + i);
        if (node->is_node_allocated) neko_safe_free(node->node);
    }
}

neko_snode_t neko_sexpr_dup_node(neko_snode_t* node, s32 free_old_node) {
    if (node->type == NEKO_SEXPR_TYPE_STRING) {
        char* str = strdup(node->str);
        if (free_old_node && node->is_str_allocated) neko_safe_free(node->str);
        node->is_str_allocated = 1;
        node->str = str;
    } else if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        neko_snode_t* n = neko_safe_malloc(node->node_len * sizeof(*n));
        memcpy(n, node->node, node->node_len * sizeof(*n));
        if (free_old_node && node->is_node_allocated) neko_safe_free(node->node);
        node->node = n;
        node->is_node_allocated = 1;
        for (s32 i = 0; i < node->node_len; i++) neko_sexpr_dup_node(node->node + i, free_old_node);
    }
    return *node;
}
