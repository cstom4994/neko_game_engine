

#include "neko_common.h"

#include "engine/neko.hpp"

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

void hashtable_init(neko_hashtable_t* table, int item_size, int initial_capacity, void* memctx) {
    initial_capacity = (int)hashtable_internal_pow2ceil(initial_capacity >= 0 ? (u32)initial_capacity : 32U);
    table->memctx = memctx;
    table->count = 0;
    table->item_size = item_size;
    table->slot_capacity = (int)hashtable_internal_pow2ceil((u32)(initial_capacity + initial_capacity / 2));
    int slots_size = (int)(table->slot_capacity * sizeof(*table->slots));
    table->slots = (neko_hashtable_t::slot_t*)HASHTABLE_MALLOC(table->memctx, (size_t)slots_size);
    NEKO_ASSERT(table->slots);
    memset(table->slots, 0, (size_t)slots_size);
    table->item_capacity = (int)hashtable_internal_pow2ceil((u32)initial_capacity);
    table->items_key = (u64*)HASHTABLE_MALLOC(table->memctx, table->item_capacity * (sizeof(*table->items_key) + sizeof(*table->items_slot) + table->item_size) + table->item_size);
    NEKO_ASSERT(table->items_key);
    table->items_slot = (int*)(table->items_key + table->item_capacity);
    table->items_data = (void*)(table->items_slot + table->item_capacity);
    table->swap_temp = (void*)(((uintptr_t)table->items_data) + table->item_size * table->item_capacity);
}

void hashtable_term(neko_hashtable_t* table) {
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

static int hashtable_internal_find_slot(neko_hashtable_t const* table, u64 key) {
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

static void hashtable_internal_expand_slots(neko_hashtable_t* table) {
    int const old_capacity = table->slot_capacity;
    neko_hashtable_t::slot_t* old_slots = table->slots;

    table->slot_capacity *= 2;
    int const slot_mask = table->slot_capacity - 1;

    int const size = (int)(table->slot_capacity * sizeof(*table->slots));
    table->slots = (neko_hashtable_t::slot_t*)HASHTABLE_MALLOC(table->memctx, (size_t)size);
    NEKO_ASSERT(table->slots);
    memset(table->slots, 0, (size_t)size);

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

static void hashtable_internal_expand_items(neko_hashtable_t* table) {
    table->item_capacity *= 2;
    u64* const new_items_key = (u64*)HASHTABLE_MALLOC(table->memctx, table->item_capacity * (sizeof(*table->items_key) + sizeof(*table->items_slot) + table->item_size) + table->item_size);
    NEKO_ASSERT(new_items_key);

    int* const new_items_slot = (int*)(new_items_key + table->item_capacity);
    void* const new_items_data = (void*)(new_items_slot + table->item_capacity);
    void* const new_swap_temp = (void*)(((uintptr_t)new_items_data) + table->item_size * table->item_capacity);

    memcpy(new_items_key, table->items_key, table->count * sizeof(*table->items_key));
    memcpy(new_items_slot, table->items_slot, table->count * sizeof(*table->items_key));
    memcpy(new_items_data, table->items_data, (size_t)table->count * table->item_size);

    HASHTABLE_FREE(table->memctx, table->items_key);

    table->items_key = new_items_key;
    table->items_slot = new_items_slot;
    table->items_data = new_items_data;
    table->swap_temp = new_swap_temp;
}

void* hashtable_insert(neko_hashtable_t* table, u64 key, void const* item) {
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

void hashtable_remove(neko_hashtable_t* table, u64 key) {
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
        memcpy(dst_item, src_item, (size_t)table->item_size);
        table->slots[table->items_slot[last_index]].item_index = index;
    }
    --table->count;
}

void hashtable_clear(neko_hashtable_t* table) {
    table->count = 0;
    memset(table->slots, 0, table->slot_capacity * sizeof(*table->slots));
}

void* hashtable_find(neko_hashtable_t const* table, u64 key) {
    int const slot = hashtable_internal_find_slot(table, key);
    if (slot < 0) return 0;

    int const index = table->slots[slot].item_index;
    void* const item = (void*)(((uintptr_t)table->items_data) + index * table->item_size);
    return item;
}

int hashtable_count(neko_hashtable_t const* table) { return table->count; }

void* hashtable_items(neko_hashtable_t const* table) { return table->items_data; }

u64 const* hashtable_keys(neko_hashtable_t const* table) { return table->items_key; }

void hashtable_swap(neko_hashtable_t* table, int index_a, int index_b) {
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
    memcpy(table->swap_temp, item_a, table->item_size);
    memcpy(item_a, item_b, table->item_size);
    memcpy(item_b, table->swap_temp, table->item_size);

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

        neko_hashtable_t table;
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
