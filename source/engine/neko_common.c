

#include "neko_common.h"

#include "engine/neko.h"

// Base64
// https://blog.csdn.net/qq_26093511/article/details/78836087

const char* neko_base64_encode(const char* str) {
    long len, str_len;
    char* res;
    int i, j;
    // 定义base64编码表
    unsigned char* base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // 计算经过base64编码后的字符串长度
    str_len = neko_strlen(str);
    if (str_len % 3 == 0)
        len = str_len / 3 * 4;
    else
        len = (str_len / 3 + 1) * 4;

    res = neko_safe_malloc(sizeof(char) * len + 1);
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

const char* neko_base64_decode(const char* code) {
    // 根据base64表，以字符找到对应的十进制数据
    int table[] = {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  62, 0,  0,  0,  63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                   17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  0,  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    long len, str_len;
    char* res;
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

    res = neko_safe_malloc(sizeof(char) * str_len + 1);
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

#define HASHTABLE_MEMSET(ptr, val, cnt) (memset(ptr, val, cnt))
#define HASHTABLE_MEMCPY(dst, src, cnt) (memcpy(dst, src, cnt))
#define HASHTABLE_MALLOC(ctx, size) (neko_safe_malloc(size))
#define HASHTABLE_FREE(ctx, ptr) (neko_safe_free(ptr))

static u32 hashtable_internal_pow2ceil(u32 v) {
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
    initial_capacity = (int)hashtable_internal_pow2ceil(initial_capacity >= 0 ? (u32)initial_capacity : 32U);
    table->memctx = memctx;
    table->count = 0;
    table->item_size = item_size;
    table->slot_capacity = (int)hashtable_internal_pow2ceil((u32)(initial_capacity + initial_capacity / 2));
    int slots_size = (int)(table->slot_capacity * sizeof(*table->slots));
    table->slots = (struct hashtable_internal_slot_t*)HASHTABLE_MALLOC(table->memctx, (size_t)slots_size);
    NEKO_ASSERT(table->slots);
    HASHTABLE_MEMSET(table->slots, 0, (size_t)slots_size);
    table->item_capacity = (int)hashtable_internal_pow2ceil((u32)initial_capacity);
    table->items_key = (u64*)HASHTABLE_MALLOC(table->memctx, table->item_capacity * (sizeof(*table->items_key) + sizeof(*table->items_slot) + table->item_size) + table->item_size);
    NEKO_ASSERT(table->items_key);
    table->items_slot = (int*)(table->items_key + table->item_capacity);
    table->items_data = (void*)(table->items_slot + table->item_capacity);
    table->swap_temp = (void*)(((uintptr_t)table->items_data) + table->item_size * table->item_capacity);
}

void hashtable_term(hashtable_t* table) {
    HASHTABLE_FREE(table->memctx, table->items_key);
    HASHTABLE_FREE(table->memctx, table->slots);
}

// from https://gist.github.com/badboy/6267743
static u32 hashtable_internal_calculate_hash(u64 key) {
    key = (~key) + (key << 18);
    key = key ^ (key >> 31);
    key = key * 21;
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);
    NEKO_ASSERT(key);
    return (u32)key;
}

static int hashtable_internal_find_slot(hashtable_t const* table, u64 key) {
    int const slot_mask = table->slot_capacity - 1;
    u32 const hash = hashtable_internal_calculate_hash(key);

    int const base_slot = (int)(hash & (u32)slot_mask);
    int base_count = table->slots[base_slot].base_count;
    int slot = base_slot;

    while (base_count > 0) {
        u32 slot_hash = table->slots[slot].key_hash;
        if (slot_hash) {
            int slot_base = (int)(slot_hash & (u32)slot_mask);
            if (slot_base == base_slot) {
                NEKO_ASSERT(base_count > 0);
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
    table->slots = (struct hashtable_internal_slot_t*)HASHTABLE_MALLOC(table->memctx, (size_t)size);
    NEKO_ASSERT(table->slots);
    HASHTABLE_MEMSET(table->slots, 0, (size_t)size);

    for (int i = 0; i < old_capacity; ++i) {
        u32 const hash = old_slots[i].key_hash;
        if (hash) {
            int const base_slot = (int)(hash & (u32)slot_mask);
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
    u64* const new_items_key =
            (u64*)HASHTABLE_MALLOC(table->memctx, table->item_capacity * (sizeof(*table->items_key) + sizeof(*table->items_slot) + table->item_size) + table->item_size);
    NEKO_ASSERT(new_items_key);

    int* const new_items_slot = (int*)(new_items_key + table->item_capacity);
    void* const new_items_data = (void*)(new_items_slot + table->item_capacity);
    void* const new_swap_temp = (void*)(((uintptr_t)new_items_data) + table->item_size * table->item_capacity);

    HASHTABLE_MEMCPY(new_items_key, table->items_key, table->count * sizeof(*table->items_key));
    HASHTABLE_MEMCPY(new_items_slot, table->items_slot, table->count * sizeof(*table->items_key));
    HASHTABLE_MEMCPY(new_items_data, table->items_data, (size_t)table->count * table->item_size);

    HASHTABLE_FREE(table->memctx, table->items_key);

    table->items_key = new_items_key;
    table->items_slot = new_items_slot;
    table->items_data = new_items_data;
    table->swap_temp = new_swap_temp;
}

void* hashtable_insert(hashtable_t* table, u64 key, void const* item) {
    NEKO_ASSERT(hashtable_internal_find_slot(table, key) < 0);

    if (table->count >= (table->slot_capacity - table->slot_capacity / 3)) hashtable_internal_expand_slots(table);

    int const slot_mask = table->slot_capacity - 1;
    u32 const hash = hashtable_internal_calculate_hash(key);

    int const base_slot = (int)(hash & (u32)slot_mask);
    int base_count = table->slots[base_slot].base_count;
    int slot = base_slot;
    int first_free = slot;
    while (base_count) {
        u32 const slot_hash = table->slots[slot].key_hash;
        if (slot_hash == 0 && table->slots[first_free].key_hash != 0) first_free = slot;
        int slot_base = (int)(slot_hash & (u32)slot_mask);
        if (slot_base == base_slot) --base_count;
        slot = (slot + 1) & slot_mask;
    }

    slot = first_free;
    while (table->slots[slot].key_hash) slot = (slot + 1) & slot_mask;

    if (table->count >= table->item_capacity) hashtable_internal_expand_items(table);

    NEKO_ASSERT(!table->slots[slot].key_hash && (hash & (u32)slot_mask) == (u32)base_slot);
    NEKO_ASSERT(hash);
    table->slots[slot].key_hash = hash;
    table->slots[slot].item_index = table->count;
    ++table->slots[base_slot].base_count;

    void* dest_item = (void*)(((uintptr_t)table->items_data) + table->count * table->item_size);
    memcpy(dest_item, item, (size_t)table->item_size);
    table->items_key[table->count] = key;
    table->items_slot[table->count] = slot;
    ++table->count;

    return dest_item;
}

void hashtable_remove(hashtable_t* table, u64 key) {
    int const slot = hashtable_internal_find_slot(table, key);
    NEKO_ASSERT(slot >= 0);

    int const slot_mask = table->slot_capacity - 1;
    u32 const hash = table->slots[slot].key_hash;
    int const base_slot = (int)(hash & (u32)slot_mask);
    NEKO_ASSERT(hash);
    --table->slots[base_slot].base_count;
    table->slots[slot].key_hash = 0;

    int index = table->slots[slot].item_index;
    int last_index = table->count - 1;
    if (index != last_index) {
        table->items_key[index] = table->items_key[last_index];
        table->items_slot[index] = table->items_slot[last_index];
        void* dst_item = (void*)(((uintptr_t)table->items_data) + index * table->item_size);
        void* src_item = (void*)(((uintptr_t)table->items_data) + last_index * table->item_size);
        HASHTABLE_MEMCPY(dst_item, src_item, (size_t)table->item_size);
        table->slots[table->items_slot[last_index]].item_index = index;
    }
    --table->count;
}

void hashtable_clear(hashtable_t* table) {
    table->count = 0;
    HASHTABLE_MEMSET(table->slots, 0, table->slot_capacity * sizeof(*table->slots));
}

void* hashtable_find(hashtable_t const* table, u64 key) {
    int const slot = hashtable_internal_find_slot(table, key);
    if (slot < 0) return 0;

    int const index = table->slots[slot].item_index;
    void* const item = (void*)(((uintptr_t)table->items_data) + index * table->item_size);
    return item;
}

int hashtable_count(hashtable_t const* table) { return table->count; }

void* hashtable_items(hashtable_t const* table) { return table->items_data; }

u64 const* hashtable_keys(hashtable_t const* table) { return table->items_key; }

void hashtable_swap(hashtable_t* table, int index_a, int index_b) {
    if (index_a < 0 || index_a >= table->count || index_b < 0 || index_b >= table->count) return;

    int slot_a = table->items_slot[index_a];
    int slot_b = table->items_slot[index_b];

    table->items_slot[index_a] = slot_b;
    table->items_slot[index_b] = slot_a;

    u64 temp_key = table->items_key[index_a];
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
        hashtable_insert( &table, (u64)(uintptr_t)key_a, &value_a );
        hashtable_insert( &table, (u64)(uintptr_t)key_b, &value_b );
        }

        // find the values by key
        value_t* value_a = (value_t*)hashtable_find( &table, (u64)(uintptr_t)key_a );
        printf( "First item: %s\n", value_a->id );
        value_t* value_b = (value_t*)hashtable_find( &table, (u64)(uintptr_t)key_b );
        printf( "Second item: %s\n", value_b->id );

        // remove one of the items
        hashtable_remove( &table, (u64)(uintptr_t)key_a );

        // it is possible to enumerate keys and values
        int count = hashtable_count( &table );
        u64 const* keys = hashtable_keys( &table );
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

/*========================
// NEKO_LEXER
========================*/

//==== [ Token ] ============================================================//

NEKO_API_DECL neko_token_t neko_token_invalid_token() {
    neko_token_t t = NEKO_DEFAULT_VAL();
    t.text = "";
    t.type = NEKO_TOKEN_UNKNOWN;
    t.len = 0;
    return t;
}

NEKO_API_DECL bool neko_token_compare_type(const neko_token_t* t, neko_token_type type) { return (t->type == type); }

NEKO_API_DECL bool neko_token_compare_text(const neko_token_t* t, const char* match) { return (neko_string_compare_equal_n(t->text, match, t->len)); }

NEKO_API_DECL void neko_token_print_text(const neko_token_t* t) { neko_println("%.*s\n", t->len, t->text); }

NEKO_API_DECL void neko_token_debug_print(const neko_token_t* t) { neko_println("%s: %.*s", neko_token_type_to_str(t->type), t->len, t->text); }

NEKO_API_DECL const char* neko_token_type_to_str(neko_token_type type) {
    switch (type) {
        default:
        case NEKO_TOKEN_UNKNOWN:
            return NEKO_TO_STR(NEKO_TOKEN_UNKNOWN);
            break;
        case NEKO_TOKEN_LPAREN:
            return NEKO_TO_STR(NEKO_TOKEN_LPAREN);
            break;
        case NEKO_TOKEN_RPAREN:
            return NEKO_TO_STR(NEKO_TOKEN_RPAREN);
            break;
        case NEKO_TOKEN_LTHAN:
            return NEKO_TO_STR(NEKO_TOKEN_LTHAN);
            break;
        case NEKO_TOKEN_GTHAN:
            return NEKO_TO_STR(NEKO_TOKEN_GTHAN);
            break;
        case NEKO_TOKEN_SEMICOLON:
            return NEKO_TO_STR(NEKO_TOKEN_SEMICOLON);
            break;
        case NEKO_TOKEN_COLON:
            return NEKO_TO_STR(NEKO_TOKEN_COLON);
            break;
        case NEKO_TOKEN_COMMA:
            return NEKO_TO_STR(NEKO_TOKEN_COMMA);
            break;
        case NEKO_TOKEN_EQUAL:
            return NEKO_TO_STR(NEKO_TOKEN_EQUAL);
            break;
        case NEKO_TOKEN_NOT:
            return NEKO_TO_STR(NEKO_TOKEN_NOT);
            break;
        case NEKO_TOKEN_HASH:
            return NEKO_TO_STR(NEKO_TOKEN_HASH);
            break;
        case NEKO_TOKEN_PIPE:
            return NEKO_TO_STR(NEKO_TOKEN_PIPE);
            break;
        case NEKO_TOKEN_AMPERSAND:
            return NEKO_TO_STR(NEKO_TOKEN_AMPERSAND);
            break;
        case NEKO_TOKEN_LBRACE:
            return NEKO_TO_STR(NEKO_TOKEN_LBRACE);
            break;
        case NEKO_TOKEN_RBRACE:
            return NEKO_TO_STR(NEKO_TOKEN_RBRACE);
            break;
        case NEKO_TOKEN_LBRACKET:
            return NEKO_TO_STR(NEKO_TOKEN_LBRACKET);
            break;
        case NEKO_TOKEN_RBRACKET:
            return NEKO_TO_STR(NEKO_TOKEN_RBRACKET);
            break;
        case NEKO_TOKEN_MINUS:
            return NEKO_TO_STR(NEKO_TOKEN_MINUS);
            break;
        case NEKO_TOKEN_PLUS:
            return NEKO_TO_STR(NEKO_TOKEN_PLUS);
            break;
        case NEKO_TOKEN_ASTERISK:
            return NEKO_TO_STR(NEKO_TOKEN_ASTERISK);
            break;
        case NEKO_TOKEN_BSLASH:
            return NEKO_TO_STR(NEKO_TOKEN_BLASH);
            break;
        case NEKO_TOKEN_FSLASH:
            return NEKO_TO_STR(NEKO_TOKEN_FLASH);
            break;
        case NEKO_TOKEN_QMARK:
            return NEKO_TO_STR(NEKO_TOKEN_QMARK);
            break;
        case NEKO_TOKEN_DOLLAR:
            return NEKO_TO_STR(NEKO_TOKEN_DOLLAR);
            break;
        case NEKO_TOKEN_SPACE:
            return NEKO_TO_STR(NEKO_TOKEN_SPACE);
            break;
        case NEKO_TOKEN_NEWLINE:
            return NEKO_TO_STR(NEKO_TOKEN_NEWLINE);
            break;
        case NEKO_TOKEN_TAB:
            return NEKO_TO_STR(NEKO_TOKEN_TAB);
            break;
        case NEKO_TOKEN_SINGLE_LINE_COMMENT:
            return NEKO_TO_STR(NEKO_TOKEN_SINGLE_LINE_COMMENT);
            break;
        case NEKO_TOKEN_MULTI_LINE_COMMENT:
            return NEKO_TO_STR(NEKO_TOKEN_MULTI_LINE_COMMENT);
            break;
        case NEKO_TOKEN_IDENTIFIER:
            return NEKO_TO_STR(NEKO_TOKEN_IDENTIFIER);
            break;
        case NEKO_TOKEN_NUMBER:
            return NEKO_TO_STR(NEKO_TOKEN_NUMBER);
            break;
    }
}

NEKO_API_DECL bool neko_char_is_null_term(char c) { return (c == '\0'); }

NEKO_API_DECL bool neko_char_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }

NEKO_API_DECL bool neko_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_char_is_end_of_line(c)); }

NEKO_API_DECL bool neko_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }

NEKO_API_DECL bool neko_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

//==== [ Lexer ] ============================================================//

NEKO_API_DECL void neko_lexer_set_contents(neko_lexer_t* lex, const char* contents) {
    lex->at = contents;
    lex->current_token = neko_token_invalid_token();
}

NEKO_API_DECL bool neko_lexer_c_can_lex(neko_lexer_t* lex) {
    bool size_pass = lex->contents_size ? lex->size < lex->contents_size : true;
    return (size_pass && lex->at && !neko_char_is_null_term(*(lex->at)));
}

NEKO_API_DECL void neko_lexer_set_token(neko_lexer_t* lex, neko_token_t token) {
    lex->at = token.text;
    lex->current_token = token;
}

NEKO_API_DECL void neko_lexer_c_eat_white_space(neko_lexer_t* lex) {
    for (;;) {
        if (neko_char_is_white_space(*lex->at)) {
            lex->at++;
        }

        // Single line comment
        else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '/')) {
            lex->at += 2;
            while (*lex->at && !neko_char_is_end_of_line(*lex->at)) {
                lex->at++;
            }
        }

        // Multi-line comment
        else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '*')) {
            lex->at += 2;
            while (lex->at[0] && lex->at[1] && !(lex->at[0] == '*' && lex->at[1] == '/')) {
                lex->at++;
            }
            if (lex->at[0] == '*') {
                lex->at++;
            }
        }

        else {
            break;
        }
    }
}

NEKO_API_DECL neko_token_t neko_lexer_c_next_token(neko_lexer_t* lex) {
    if (lex->skip_white_space) {
        lex->eat_white_space(lex);
    }

    neko_token_t t = neko_token_invalid_token();
    t.text = lex->at;
    t.len = 1;

    if (lex->can_lex(lex)) {
        char c = *lex->at;
        switch (c) {
            case '(': {
                t.type = NEKO_TOKEN_LPAREN;
                lex->at++;
            } break;
            case ')': {
                t.type = NEKO_TOKEN_RPAREN;
                lex->at++;
            } break;
            case '<': {
                t.type = NEKO_TOKEN_LTHAN;
                lex->at++;
            } break;
            case '>': {
                t.type = NEKO_TOKEN_GTHAN;
                lex->at++;
            } break;
            case ';': {
                t.type = NEKO_TOKEN_SEMICOLON;
                lex->at++;
            } break;
            case ':': {
                t.type = NEKO_TOKEN_COLON;
                lex->at++;
            } break;
            case ',': {
                t.type = NEKO_TOKEN_COMMA;
                lex->at++;
            } break;
            case '=': {
                t.type = NEKO_TOKEN_EQUAL;
                lex->at++;
            } break;
            case '!': {
                t.type = NEKO_TOKEN_NOT;
                lex->at++;
            } break;
            case '#': {
                t.type = NEKO_TOKEN_HASH;
                lex->at++;
            } break;
            case '|': {
                t.type = NEKO_TOKEN_PIPE;
                lex->at++;
            } break;
            case '&': {
                t.type = NEKO_TOKEN_AMPERSAND;
                lex->at++;
            } break;
            case '{': {
                t.type = NEKO_TOKEN_LBRACE;
                lex->at++;
            } break;
            case '}': {
                t.type = NEKO_TOKEN_RBRACE;
                lex->at++;
            } break;
            case '[': {
                t.type = NEKO_TOKEN_LBRACKET;
                lex->at++;
            } break;
            case ']': {
                t.type = NEKO_TOKEN_RBRACKET;
                lex->at++;
            } break;
            case '+': {
                t.type = NEKO_TOKEN_PLUS;
                lex->at++;
            } break;
            case '*': {
                t.type = NEKO_TOKEN_ASTERISK;
                lex->at++;
            } break;
            case '\\': {
                t.type = NEKO_TOKEN_BSLASH;
                lex->at++;
            } break;
            case '?': {
                t.type = NEKO_TOKEN_QMARK;
                lex->at++;
            } break;
            case '%': {
                t.type = NEKO_TOKEN_PERCENT;
                lex->at++;
            } break;
            case '$': {
                t.type = NEKO_TOKEN_DOLLAR;
                lex->at++;
            } break;
            case ' ': {
                t.type = NEKO_TOKEN_SPACE;
                lex->at++;
            } break;
            case '\n': {
                t.type = NEKO_TOKEN_NEWLINE;
                lex->at++;
            } break;
            case '\r': {
                t.type = NEKO_TOKEN_NEWLINE;
                lex->at++;
            } break;
            case '\t': {
                t.type = NEKO_TOKEN_TAB;
                lex->at++;
            } break;
            case '.': {
                t.type = NEKO_TOKEN_PERIOD;
                lex->at++;
            } break;

            case '-': {
                if (lex->at[1] && !neko_char_is_numeric(lex->at[1])) {
                    t.type = NEKO_TOKEN_MINUS;
                    lex->at++;
                } else {
                    lex->at++;
                    u32 num_decimals = 0;
                    while (lex->at[0] && (neko_char_is_numeric(lex->at[0]) || (lex->at[0] == '.' && num_decimals == 0) || lex->at[0] == 'f')) {
                        // Grab decimal
                        num_decimals = lex->at[0] == '.' ? num_decimals++ : num_decimals;

                        // Increment
                        lex->at++;
                    }

                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_NUMBER;
                }

            } break;

            case '/': {
                // Single line comment
                if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '/')) {
                    lex->at += 2;
                    while (lex->at[0] && !neko_char_is_end_of_line(lex->at[0])) {
                        lex->at++;
                    }
                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_SINGLE_LINE_COMMENT;
                }

                // Multi line comment
                else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '*')) {
                    lex->at += 2;
                    while (lex->can_lex(lex)) {
                        if (lex->at[0] == '*' && lex->at[1] == '/') {
                            lex->at += 2;
                            break;
                        }
                        lex->at++;
                    }
                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_MULTI_LINE_COMMENT;
                }
                // it's just a forward slash
                else {
                    t.type = NEKO_TOKEN_FSLASH;
                    lex->at++;
                }
            } break;

            case '"': {
                // Move forward after finding first quotation
                lex->at++;

                while (lex->at && *lex->at != '"') {
                    if (lex->at[0] == '\\' && lex->at[1]) {
                        lex->at++;
                    }
                    lex->at++;
                }

                // Move past quotation
                lex->at++;
                t.len = lex->at - t.text;
                t.type = NEKO_TOKEN_STRING;
            } break;

            // Alpha/Numeric/Identifier
            default: {
                if ((neko_char_is_alpha(c) || c == '_') && c != '-') {
                    while (neko_char_is_alpha(lex->at[0]) || neko_char_is_numeric(lex->at[0]) || lex->at[0] == '_') {
                        lex->at++;
                    }

                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_IDENTIFIER;
                } else if (neko_char_is_numeric(c) && c != '-') {
                    u32 num_decimals = 0;
                    while (neko_char_is_numeric(lex->at[0]) || (lex->at[0] == '.' && num_decimals == 0) || lex->at[0] == 'f') {
                        // Grab decimal
                        num_decimals = lex->at[0] == '.' ? num_decimals++ : num_decimals;

                        // Increment
                        lex->at++;
                    }

                    t.len = lex->at - t.text;
                    t.type = NEKO_TOKEN_NUMBER;
                } else {
                    t.type = NEKO_TOKEN_UNKNOWN;
                    lex->at++;
                }

            } break;
        }
    }

    // Set current token for lex
    lex->current_token = t;

    // Record size
    lex->size += t.len;

    return t;
}

NEKO_API_DECL neko_token_t neko_lexer_next_token(neko_lexer_t* lex) { return lex->next_token(lex); }

NEKO_API_DECL bool neko_lexer_can_lex(neko_lexer_t* lex) { return lex->can_lex(lex); }

NEKO_API_DECL neko_token_t neko_lexer_current_token(const neko_lexer_t* lex) { return lex->current_token; }

NEKO_API_DECL bool neko_lexer_current_token_compare_type(const neko_lexer_t* lex, neko_token_type type) { return (lex->current_token.type == type); }

NEKO_API_DECL neko_token_t neko_lexer_peek(neko_lexer_t* lex) {
    // Store current at and current token
    const char* at = lex->at;
    neko_token_t cur_t = neko_lexer_current_token(lex);

    // Get next token
    neko_token_t next_t = lex->next_token(lex);

    // Reset
    lex->current_token = cur_t;
    lex->at = at;

    // Return
    return next_t;
}

// Check to see if token type of next valid token matches 'match'. Restores place in lex if not.
NEKO_API_DECL bool neko_lexer_require_token_text(neko_lexer_t* lex, const char* match) {
    // Store current position and token
    const char* at = lex->at;
    neko_token_t cur_t = lex->current_token;

    // Get next token
    neko_token_t next_t = lex->next_token(lex);

    // Compare token text
    if (neko_token_compare_text(&next_t, match)) {
        return true;
    }

    // Error
    neko_log_warning("neko_lexer_require_token_text::%.*s, expected: %s", cur_t.len, cur_t.text, match);

    // Reset
    lex->at = at;
    lex->current_token = cur_t;

    return false;
}

NEKO_API_DECL bool neko_lexer_require_token_type(neko_lexer_t* lex, neko_token_type type) {
    // Store current position and token
    const char* at = lex->at;
    neko_token_t cur_t = lex->current_token;

    // Get next token
    neko_token_t next_t = lex->next_token(lex);

    // Compare token type
    if (neko_token_compare_type(&next_t, type)) {
        return true;
    }

    // Error
    // neko_println("error::neko_lexer_require_token_type::%s, expected: %s", neko_token_type_to_str(next_t.type), neko_token_type_to_str(type));

    // Reset
    lex->at = at;
    lex->current_token = cur_t;

    return false;
}

// Advances until next token of given type is found
NEKO_API_DECL bool neko_lexer_find_next_token_type(neko_lexer_t* lex, neko_token_type type) {
    neko_token_t t = lex->next_token(lex);
    while (lex->can_lex(lex)) {
        if (neko_token_compare_type(&t, type)) {
            return true;
        }
        t = lex->next_token(lex);
    }
    return false;
}

NEKO_API_DECL neko_token_t neko_lexer_advance_before_next_token_type(neko_lexer_t* lex, neko_token_type type) {
    neko_token_t t = lex->current_token;
    neko_token_t peek_t = neko_lexer_peek(lex);

    // 继续直到所需的token类型
    while (!neko_token_compare_type(&peek_t, type)) {
        t = lex->next_token(lex);
        peek_t = neko_lexer_peek(lex);
    }

    return t;
}

NEKO_API_DECL neko_lexer_t neko_lexer_c_ctor(const char* contents) {
    neko_lexer_t lex = NEKO_DEFAULT_VAL();
    lex.contents = contents;
    lex.at = contents;
    lex.can_lex = neko_lexer_c_can_lex;
    lex.eat_white_space = neko_lexer_c_eat_white_space;
    lex.next_token = neko_lexer_c_next_token;
    lex.skip_white_space = true;
    return lex;
}
