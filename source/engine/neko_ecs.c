
#include "neko_ecs.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "deps/lua/lauxlib.h"
#include "deps/lua/lua.h"
#include "deps/lua/lualib.h"
#include "neko.h"

static void *entitytainer__ptr_to_aligned_ptr(void *ptr, int align);
static bool entitytainer__child_in_bucket(neko_entitytainer_entity *bucket, neko_entitytainer_bucketlist *bucket_list, neko_entitytainer_entity child);

NEKO_API_DECL int neko_ent_needed_size(struct neko_entitytainer_config *config) {
    int size_needed = sizeof(neko_entitytainer);
    size_needed += config->num_entries * sizeof(neko_entitytainer_entry);            // Lookup
    size_needed += config->num_entries * sizeof(neko_entitytainer_entity);           // Reverse lookup
    size_needed += config->num_bucket_lists * sizeof(neko_entitytainer_bucketlist);  // List structs

    // Bucket lists
    for (int i = 0; i < config->num_bucket_lists; ++i) {
        size_needed += config->bucket_list_sizes[i] * config->bucket_sizes[i] * sizeof(neko_entitytainer_entity);
    }

    // Account for struct alignment, with good margins :D
    int things_to_align = 1 + config->num_bucket_lists;
    int safe_alignment = sizeof(void *) * 16;
    size_needed += things_to_align * safe_alignment;

    return size_needed;
}

NEKO_API_DECL neko_entitytainer *neko_ent_create(struct neko_entitytainer_config *config) {

    unsigned char *buffer_start = (unsigned char *)config->memory;
    memset(buffer_start, 0, config->memory_size);
    unsigned char *buffer = buffer_start;
    buffer = (unsigned char *)entitytainer__ptr_to_aligned_ptr(buffer, (int)NEKO_ENT_alignof(neko_entitytainer));

    neko_entitytainer *entitytainer = (neko_entitytainer *)buffer;
    entitytainer->num_bucket_lists = config->num_bucket_lists;
    entitytainer->remove_with_holes = config->remove_with_holes;
    entitytainer->keep_capacity_on_remove = config->keep_capacity_on_remove;
    entitytainer->entry_lookup_size = config->num_entries;

    memcpy(&entitytainer->config, config, sizeof(*config));
    // if ( entitytainer->config.name[0] == 0 ) {
    //     const char* default_name = "entitytainer";
    //     memcpy( entitytainer->config.name, default_name, 12 );
    //     entitytainer->config.name[12] = 0;
    // }

    buffer += sizeof(neko_entitytainer);
    entitytainer->entry_lookup = (neko_entitytainer_entry *)buffer;
    buffer += sizeof(neko_entitytainer_entry) * config->num_entries;
    entitytainer->entry_parent_lookup = (neko_entitytainer_entity *)buffer;
    buffer += sizeof(neko_entitytainer_entity) * config->num_entries;

    buffer = (unsigned char *)entitytainer__ptr_to_aligned_ptr(buffer, (int)NEKO_ENT_alignof(neko_entitytainer_bucketlist));
    entitytainer->bucket_lists = (neko_entitytainer_bucketlist *)buffer;

    unsigned char *bucket_list_end = buffer + sizeof(neko_entitytainer_bucketlist) * config->num_bucket_lists;
    neko_entitytainer_entity *bucket_data_start = (neko_entitytainer_entity *)bucket_list_end;
    neko_entitytainer_entity *bucket_data = bucket_data_start;
    for (int i = 0; i < config->num_bucket_lists; ++i) {
        // Just making sure that we don't go into the bucket data area
        neko_assert(buffer + sizeof(neko_entitytainer_bucketlist) <= bucket_list_end, "Passing end of bucket area");

        // We need to do this because first_free_bucket is stored as an int.
        neko_assert(config->bucket_sizes[i] * sizeof(neko_entitytainer_entity) >= sizeof(int));

        neko_entitytainer_bucketlist *list = (neko_entitytainer_bucketlist *)buffer;
        list->bucket_data = bucket_data;
        list->bucket_size = config->bucket_sizes[i];
        list->total_buckets = config->bucket_list_sizes[i];
        list->first_free_bucket = NEKO_ENT_NoFreeBucket;
        list->used_buckets = 0;

        if (i == 0) {
            // We need this in order to ensure that we can use 0 as the default "invalid" entry.
            list->used_buckets = 1;
        }

        buffer += sizeof(neko_entitytainer_bucketlist);
        bucket_data += list->bucket_size * list->total_buckets;
    }

    neko_assert(*bucket_data_start == 0);
    neko_assert((unsigned char *)bucket_data <= buffer_start + config->memory_size);
    return entitytainer;
}

NEKO_API_DECL neko_entitytainer *neko_ent_realloc(neko_entitytainer *entitytainer_old, void *memory, int memory_size, float growth) {
    neko_assert(false);  // Not yet implemented
    (void)memory_size;

    int num_entries = entitytainer_old->entry_lookup_size;  // * growth;
    int size_needed = sizeof(neko_entitytainer);
    size_needed += (int)(num_entries * sizeof(neko_entitytainer_entry));
    size_needed += (int)(num_entries * sizeof(neko_entitytainer_entity));
    size_needed += entitytainer_old->num_bucket_lists * sizeof(neko_entitytainer_bucketlist);

    for (int i = 0; i < entitytainer_old->num_bucket_lists; ++i) {
        neko_entitytainer_bucketlist *bucket_list = &entitytainer_old->bucket_lists[i];
        int old_bucket_size = bucket_list->total_buckets * bucket_list->bucket_size * sizeof(neko_entitytainer_entity);
        size_needed += (int)(old_bucket_size * growth);
    }

    char *buffer = (char *)memory;

    neko_entitytainer *entitytainer = (neko_entitytainer *)buffer;
    *entitytainer = *entitytainer_old;
    entitytainer->entry_lookup_size = num_entries;
    buffer += sizeof(neko_entitytainer);
    entitytainer->entry_lookup = (neko_entitytainer_entry *)buffer;
    buffer += sizeof(neko_entitytainer_entry) * num_entries;
    entitytainer->entry_parent_lookup = (neko_entitytainer_entity *)buffer;
    buffer += sizeof(neko_entitytainer_entity) * num_entries;

    // char* bucket_data = buffer + sizeof( TheEntitytainerBucketList ) * entitytainer_old->num_bucket_lists;
    // for ( int i = 0; i < entitytainer_old->num_bucket_lists; ++i ) {
    //     // neko_assert( bucket_data - buffer > bucket_sizes[i] * bucket_list_sizes[i] ); // >= ?
    //     TheEntitytainerBucketList* list = (TheEntitytainerBucketList*)buffer;
    //     list->bucket_data                   = bucket_data;
    //     list->bucket_size               = entitytainer_old->bucket_lists[i].bucket_size;
    //     list->total_buckets             = entitytainer_old->bucket_lists[i].total_buckets;
    //     list->first_free_bucket         = entitytainer_old->bucket_lists[i].first_free_bucket;
    //     list->used_buckets              = entitytainer_old->bucket_lists[i].used_buckets;

    //     int old_buffer_size = entitytainer_old->bucket_lists[i].total_buckets * sizeof( neko_entitytainer_entry );
    //     memcpy( list->bucket_data, entitytainer_old->bucket_lists[i].bucket_data, old_buffer_size );
    //     buffer += sizeof( TheEntitytainerBucketList );
    //     bucket_data += list->bucket_size * list->total_buckets;
    // }

    // neko_assert( bucket_data == buffer + buffer_size );
    return entitytainer;
}

NEKO_API_DECL bool neko_ent_needs_realloc(neko_entitytainer *entitytainer, float percent_free, int num_free_buckets) {
    for (int i = 0; i < entitytainer->num_bucket_lists; ++i) {
        // neko_assert( bucket_data - buffer > bucket_sizes[i] * bucket_list_sizes[i] ); // >= ?
        neko_entitytainer_bucketlist *list = &entitytainer->bucket_lists[i];
        if (percent_free >= 0) {
            num_free_buckets = (int)(list->total_buckets * percent_free);
        }

        if (list->total_buckets - list->used_buckets <= num_free_buckets) {
            return true;
        }
    }

    return false;
}

NEKO_API_DECL void neko_ent_add_entity(neko_entitytainer *entitytainer, neko_entitytainer_entity entity) {
    neko_assert(entitytainer->entry_lookup[entity] == 0, "Entitytainer[%s] Tried to add entity " NEKO_ENT_EntityFormat " but it was already added.", "", entity);

    neko_entitytainer_bucketlist *bucket_list = &entitytainer->bucket_lists[0];
    int bucket_index = bucket_list->used_buckets;
    if (bucket_list->first_free_bucket != NEKO_ENT_NoFreeBucket) {
        // There's a freed bucket available
        bucket_index = bucket_list->first_free_bucket;
        int bucket_offset = bucket_index * bucket_list->bucket_size;
        bucket_list->first_free_bucket = bucket_list->bucket_data[bucket_offset];
    }

    // TODO: Move to larger bucket list if this one is full
    neko_assert(bucket_list->used_buckets < bucket_list->total_buckets);
    ++bucket_list->used_buckets;

    neko_entitytainer_entry *lookup = &entitytainer->entry_lookup[entity];
    neko_assert(*lookup == 0);
    *lookup = (neko_entitytainer_entry)bucket_index;  // bucket list index is 0

    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = bucket_list->bucket_data + bucket_offset;
    memset(bucket, 0, bucket_list->bucket_size * sizeof(neko_entitytainer_entity));
}

NEKO_API_DECL void entitytainer_remove_entity(neko_entitytainer *entitytainer, neko_entitytainer_entity entity) {
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[entity];

    if (entitytainer->entry_parent_lookup[entity] != NEKO_ENT_InvalidEntity) {
        if (entitytainer->remove_with_holes) {
            neko_ent_remove_child_with_holes(entitytainer, entitytainer->entry_parent_lookup[entity], entity);
        } else {
            neko_ent_remove_child_no_holes(entitytainer, entitytainer->entry_parent_lookup[entity], entity);
        }

        lookup = entitytainer->entry_lookup[entity];
    }

    if (lookup == 0) {
        // lookup is 0 for entities that don't have children (or haven't been added by _add_entity)
        return;
    }

    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = bucket_list->bucket_data + bucket_offset;
    neko_assert(bucket[0] == 0, "Entitytainer[%s] Tried to remove " NEKO_ENT_EntityFormat " but it still had children. First child=" NEKO_ENT_EntityFormat, "", entity, bucket[1]);
    *bucket = (neko_entitytainer_entity)bucket_list->first_free_bucket;
    bucket_list->first_free_bucket = bucket_index;

    entitytainer->entry_lookup[entity] = 0;
    --bucket_list->used_buckets;
}

NEKO_API_DECL void neko_ent_reserve(neko_entitytainer *entitytainer, neko_entitytainer_entity parent, int capacity) {
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[parent];
    neko_assert(lookup != 0);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = bucket_list->bucket_data + bucket_offset;
    if (bucket_list->bucket_size > capacity) {
        return;
    }

    neko_entitytainer_bucketlist *bucket_list_new = NULL;
    int bucket_list_index_new = -1;
    for (int i_bl = 0; i_bl < entitytainer->num_bucket_lists; ++i_bl) {
        bucket_list_new = &entitytainer->bucket_lists[i_bl];
        if (bucket_list_new->bucket_size > capacity + 1) {
            bucket_list_index_new = i_bl;
            break;
        }
    }

    neko_assert(bucket_list_index_new != -1);

    int bucket_index_new = bucket_list_new->used_buckets;
    if (bucket_list_new->first_free_bucket != NEKO_ENT_NoFreeBucket) {
        // There's a freed bucket available
        bucket_index_new = bucket_list_new->first_free_bucket;
        int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
    }

    int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
    neko_entitytainer_entity *bucket_new = bucket_list_new->bucket_data + bucket_offset_new;
    memset(bucket_new, 0, bucket_list_new->bucket_size * sizeof(neko_entitytainer_entity));
    memcpy(bucket_new, bucket, bucket_list->bucket_size * sizeof(neko_entitytainer_entity));

    *bucket = (neko_entitytainer_entity)bucket_list->first_free_bucket;
    bucket_list->first_free_bucket = bucket_index;
    bucket = bucket_new;

    bucket_list_new->used_buckets++;
    bucket_list->used_buckets--;

    // Update lookup
    neko_entitytainer_entry lookup_new = (neko_entitytainer_entry)(bucket_list_index_new << NEKO_ENT_BucketListOffset);
    lookup_new = lookup_new | (neko_entitytainer_entry)bucket_index_new;
    entitytainer->entry_lookup[parent] = lookup_new;
}

NEKO_API_DECL void neko_ent_add_child(neko_entitytainer *entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child) {
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[parent];
    neko_assert(lookup != 0, "Entitytainer[%s] Tried to add " NEKO_ENT_EntityFormat " as child to " NEKO_ENT_EntityFormat " who was not added.", "", child, parent);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = bucket_list->bucket_data + bucket_offset;

#if NEKO_ENT_DEFENSIVE_ASSERTS
    neko_assert(!entitytainer__child_in_bucket(bucket, bucket_list, child),
                "Entitytainer[%s] Tried to add " NEKO_ENT_EntityFormat " as child to " NEKO_ENT_EntityFormat " but it was already its child.", "", child, parent);
#endif

#if NEKO_ENT_DEFENSIVE_CHECKS
    if (entitytainer__child_in_bucket(bucket, bucket_list, child)) {
        neko_assert(entitytainer_get_parent(entitytainer, child) == parent);
        return;
    }
#endif

    if (bucket[0] + 1 == bucket_list->bucket_size) {
        neko_assert(bucket_list_index != 3);
        neko_entitytainer_bucketlist *bucket_list_new = bucket_list + 1;
        int bucket_index_new = bucket_list_new->used_buckets;
        if (bucket_list_new->first_free_bucket != NEKO_ENT_NoFreeBucket) {
            // There's a freed bucket available
            bucket_index_new = bucket_list_new->first_free_bucket;
            int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
            bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
        }

        int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        neko_entitytainer_entity *bucket_new = bucket_list_new->bucket_data + bucket_offset_new;
        memset(bucket_new, 0, bucket_list_new->bucket_size * sizeof(neko_entitytainer_entity));
        memcpy(bucket_new, bucket, bucket_list->bucket_size * sizeof(neko_entitytainer_entity));

        *bucket = (neko_entitytainer_entity)bucket_list->first_free_bucket;
        bucket_list->first_free_bucket = bucket_index;
        bucket = bucket_new;

        bucket_list_new->used_buckets++;
        bucket_list->used_buckets--;

        // Update lookup
        int bucket_list_index_new = (bucket_list_index + 1) << NEKO_ENT_BucketListOffset;
        neko_entitytainer_entry lookup_new = (neko_entitytainer_entry)bucket_list_index_new;
        lookup_new = lookup_new | (neko_entitytainer_entry)bucket_index_new;
        entitytainer->entry_lookup[parent] = lookup_new;
    }

#if NEKO_ENT_DEFENSIVE_ASSERTS
    neko_assert(!entitytainer__child_in_bucket(bucket, bucket_list, child));
#endif

    // Update count and insert child into bucket
    neko_entitytainer_entity count = bucket[0] + (neko_entitytainer_entity)1;
    bucket[0] = count;
    if (entitytainer->remove_with_holes) {
        int i = 1;
        for (; i < count; ++i) {
            if (bucket[i] == NEKO_ENT_InvalidEntity) {
                bucket[i] = child;
                break;
            }
        }
        if (i == count) {
            // Didn't find a "holed" slot, add child to the end.
            bucket[i] = child;
        }
    } else {
        bucket[count] = child;
    }

    neko_assert(entitytainer->entry_parent_lookup[child] == NEKO_ENT_InvalidEntity);
    neko_assert(entitytainer->entry_parent_lookup[child] == NEKO_ENT_InvalidEntity,
                "Entitytainer[%s] Tried to add " NEKO_ENT_EntityFormat " as child to " NEKO_ENT_EntityFormat " but it was already parented to " NEKO_ENT_EntityFormat, "", child, parent,
                entitytainer->entry_parent_lookup[child]);
    entitytainer->entry_parent_lookup[child] = parent;
}

NEKO_API_DECL void neko_ent_add_child_at_index(neko_entitytainer *entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child, int index) {
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[parent];
    neko_assert(lookup != 0);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = bucket_list->bucket_data + bucket_offset;

#if NEKO_ENT_DEFENSIVE_ASSERTS
    neko_assert(!entitytainer__child_in_bucket(bucket, bucket_list, child));
#endif

#if NEKO_ENT_DEFENSIVE_CHECKS
    if (entitytainer__child_in_bucket(bucket, bucket_list, child)) {
        neko_assert(entitytainer_get_parent(entitytainer, child) == parent);
        return;
    }
#endif

    while (index + 1 >= bucket_list->bucket_size) {
        neko_assert(bucket_list_index != 3);  // No bucket lists with buckets of this size
        neko_entitytainer_bucketlist *bucket_list_new = bucket_list + 1;
        int bucket_index_new = bucket_list_new->used_buckets;
        if (index + 1 >= bucket_list_new->bucket_size) {
            bucket_list_index += 1;
            bucket_list = bucket_list_new;
            continue;
        }

        if (bucket_list_new->first_free_bucket != NEKO_ENT_NoFreeBucket) {
            // There's a freed bucket available
            bucket_index_new = bucket_list_new->first_free_bucket;
            int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
            bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
        }

        neko_assert(bucket_index_new < bucket_list_new->total_buckets);  // No free buckets at all
        int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        neko_entitytainer_entity *bucket_new = bucket_list_new->bucket_data + bucket_offset_new;
        memset(bucket_new, 0, bucket_list_new->bucket_size * sizeof(neko_entitytainer_entity));
        memcpy(bucket_new, bucket, bucket_list->bucket_size * sizeof(neko_entitytainer_entity));

        *bucket = (neko_entitytainer_entity)bucket_list->first_free_bucket;
        bucket_list->first_free_bucket = bucket_index;

        bucket_list_new->used_buckets++;
        bucket_list->used_buckets--;

        bucket = bucket_new;
        bucket_list = bucket_list_new;
        bucket_index = bucket_index_new;
        bucket_list_index += 1;

        // Update lookup
        int bucket_list_index_new = (bucket_list_index) << NEKO_ENT_BucketListOffset;
        neko_entitytainer_entry lookup_new = (neko_entitytainer_entry)bucket_list_index_new;
        lookup_new = lookup_new | (neko_entitytainer_entry)bucket_index_new;
        entitytainer->entry_lookup[parent] = lookup_new;
    }

#if NEKO_ENT_DEFENSIVE_ASSERTS
    neko_assert(!entitytainer__child_in_bucket(bucket, bucket_list, child));
#endif

    // Update count and insert child into bucket
    neko_assert(bucket[index + 1] == NEKO_ENT_InvalidEntity);
    neko_entitytainer_entity count = bucket[0] + (neko_entitytainer_entity)1;
    bucket[0] = count;
    bucket[index + 1] = child;

    neko_assert(entitytainer->entry_parent_lookup[child] == NEKO_ENT_InvalidEntity,
                "Entitytainer[%s] Tried to add " NEKO_ENT_EntityFormat " as child to " NEKO_ENT_EntityFormat " but it was already parented to " NEKO_ENT_EntityFormat, "", child, parent,
                entitytainer->entry_parent_lookup[child]);
    entitytainer->entry_parent_lookup[child] = parent;
}

NEKO_API_DECL void neko_ent_remove_child_no_holes(neko_entitytainer *entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child) {
    neko_assert(!entitytainer->config.remove_with_holes);
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[parent];
    neko_assert(lookup != 0);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = (neko_entitytainer_entity *)(bucket_list->bucket_data + bucket_offset);

    // Remove child from bucket, move children after forward one step.
    int num_children = bucket[0];
    neko_entitytainer_entity *child_to_move = &bucket[1];
    int count = 0;
    while (*child_to_move != child && count < num_children) {
        ++count;
        ++child_to_move;
    }

    neko_assert(count < num_children);

    for (; count < num_children - 1; ++count) {
        *child_to_move = *(child_to_move + 1);
        ++child_to_move;
    }

    // Lower child count, clear entry
    bucket[0]--;
    entitytainer->entry_parent_lookup[child] = 0;

#if NEKO_ENT_DEFENSIVE_ASSERTS
    neko_assert(!entitytainer__child_in_bucket(bucket, bucket_list, child));
#endif

#if NEKO_ENT_DEFENSIVE_CHECKS
    if (entitytainer__child_in_bucket(bucket, bucket_list, child)) {
        entitytainer_remove_child_no_holes(entitytainer, parent, child);
    }
#endif

    if (entitytainer->keep_capacity_on_remove) {
        return;
    }

    neko_entitytainer_bucketlist *bucket_list_prev = bucket_list_index > 0 ? (entitytainer->bucket_lists + bucket_list_index - 1) : NULL;
    if (bucket_list_prev != NULL && bucket[0] + 1 == bucket_list_prev->bucket_size) {
        neko_entitytainer_bucketlist *bucket_list_new = bucket_list_prev;
        int bucket_index_new = bucket_list_new->used_buckets;
        if (bucket_list_new->first_free_bucket != NEKO_ENT_NoFreeBucket) {
            // There's a freed bucket available
            bucket_index_new = bucket_list_new->first_free_bucket;
            int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
            bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
        }

        int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        neko_entitytainer_entity *bucket_new = bucket_list_new->bucket_data + bucket_offset_new;
        memcpy(bucket_new, bucket, bucket_list_new->bucket_size * sizeof(neko_entitytainer_entity));

        bucket_list_new->used_buckets++;
        bucket_list->used_buckets--;

        // Update lookup
        int bucket_list_index_new = (bucket_list_index - 1) << NEKO_ENT_BucketListOffset;
        neko_entitytainer_entry lookup_new = (neko_entitytainer_entry)bucket_list_index_new;
        lookup_new = lookup_new | (neko_entitytainer_entry)bucket_index_new;
        entitytainer->entry_lookup[parent] = lookup_new;

#if NEKO_ENT_DEFENSIVE_ASSERTS
        neko_assert(!entitytainer__child_in_bucket(bucket, bucket_list, child));
#endif

#if NEKO_ENT_DEFENSIVE_CHECKS
        if (entitytainer__child_in_bucket(bucket, bucket_list, child)) {
            entitytainer_remove_child_no_holes(entitytainer, parent, child);
        }
#endif
    }
}

NEKO_API_DECL void neko_ent_remove_child_with_holes(neko_entitytainer *entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child) {
    neko_assert(entitytainer->remove_with_holes);
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[parent];
    neko_assert(lookup != 0);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = (neko_entitytainer_entity *)(bucket_list->bucket_data + bucket_offset);

    // Remove child from bucket, move children after forward one step.
    int capacity = bucket_list->bucket_size;
    int last_child_index = 0;
    int child_to_move_index = 0;
    for (int i = 1; i < capacity; i++) {
        if (bucket[i] == child) {
            child_to_move_index = i;
        } else if (bucket[i] != NEKO_ENT_InvalidEntity) {
            last_child_index = i;
        }
    }

    neko_assert(child_to_move_index != 0);
    bucket[child_to_move_index] = NEKO_ENT_InvalidEntity;

    // Lower child count, clear entry
    bucket[0]--;
    entitytainer->entry_parent_lookup[child] = 0;

#if NEKO_ENT_DEFENSIVE_ASSERTS
    neko_assert(!entitytainer__child_in_bucket(bucket, bucket_list, child),
                "Entitytainer[%s] Removed child " NEKO_ENT_EntityFormat " from parent " NEKO_ENT_EntityFormat ", but it was still its child.", "", child, parent);
#endif

#if NEKO_ENT_DEFENSIVE_CHECKS
    if (entitytainer__child_in_bucket(bucket, bucket_list, child)) {
        entitytainer_remove_child_with_holes(entitytainer, parent, child);
    }
#endif

    if (entitytainer->keep_capacity_on_remove) {
        return;
    }

    neko_entitytainer_bucketlist *bucket_list_prev = bucket_list_index > 0 ? (entitytainer->bucket_lists + bucket_list_index - 1) : NULL;
    if (bucket_list_prev != NULL && last_child_index + NEKO_ENT_ShrinkMargin < bucket_list_prev->bucket_size) {
        // We've shrunk enough to fit in the previous bucket, move.
        neko_entitytainer_bucketlist *bucket_list_new = bucket_list_prev;
        int bucket_index_new = bucket_list_new->used_buckets;
        int bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        if (bucket_list_new->first_free_bucket != NEKO_ENT_NoFreeBucket) {
            // There's a freed bucket available
            bucket_index_new = bucket_list_new->first_free_bucket;
            bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
            bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
        }

        neko_entitytainer_entity *bucket_new = bucket_list_new->bucket_data + bucket_offset_new;
        memcpy(bucket_new, bucket, bucket_list_new->bucket_size * sizeof(neko_entitytainer_entity));

        bucket_list_new->used_buckets++;
        bucket_list->used_buckets--;

        // Update lookup
        int bucket_list_index_new = (bucket_list_index - 1) << NEKO_ENT_BucketListOffset;
        neko_entitytainer_entry lookup_new = (neko_entitytainer_entry)bucket_list_index_new;
        lookup_new = lookup_new | (neko_entitytainer_entry)bucket_index_new;
        entitytainer->entry_lookup[parent] = lookup_new;

#if NEKO_ENT_DEFENSIVE_ASSERTS
        neko_assert(!entitytainer__child_in_bucket(bucket, bucket_list, child));
#endif

#if NEKO_ENT_DEFENSIVE_CHECKS
        if (entitytainer__child_in_bucket(bucket, bucket_list, child)) {
            entitytainer_remove_child_with_holes(entitytainer, parent, child);
        }
#endif
    }
}

NEKO_API_DECL void neko_ent_get_children(neko_entitytainer *entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity **children, int *num_children, int *capacity) {

    neko_entitytainer_entry lookup = entitytainer->entry_lookup[parent];
    neko_assert(lookup != 0);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = (neko_entitytainer_entity *)(bucket_list->bucket_data + bucket_offset);
    *num_children = (int)bucket[0];
    *children = bucket + 1;
    *capacity = bucket_list->bucket_size - 1;
}

NEKO_API_DECL int entitytainer_num_children(neko_entitytainer *entitytainer, neko_entitytainer_entity parent) {
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[parent];
    neko_assert(lookup != 0);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = (neko_entitytainer_entity *)(bucket_list->bucket_data + bucket_offset);
    return (int)bucket[0];
}

NEKO_API_DECL int neko_ent_get_child_index(neko_entitytainer *entitytainer, neko_entitytainer_entity parent, neko_entitytainer_entity child) {
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[parent];
    neko_assert(lookup != 0);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = (neko_entitytainer_entity *)(bucket_list->bucket_data + bucket_offset);
    int num_children = (int)bucket[0];
    for (int i = 0; i < num_children; ++i) {
        if (bucket[1 + i] == child) {
            return i;
        }
    }

    return -1;
}

NEKO_API_DECL neko_entitytainer_entity neko_ent_get_parent(neko_entitytainer *entitytainer, neko_entitytainer_entity child) {
    neko_entitytainer_entity parent = entitytainer->entry_parent_lookup[child];
    return parent;
}

NEKO_API_DECL bool neko_ent_is_added(neko_entitytainer *entitytainer, neko_entitytainer_entity entity) {
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[entity];
    return lookup != 0;
}

NEKO_API_DECL void neko_ent_remove_holes(neko_entitytainer *entitytainer, neko_entitytainer_entity entity) {
    // TODO
    neko_assert(false);
    neko_entitytainer_entry lookup = entitytainer->entry_lookup[entity];
    neko_assert(lookup != 0);
    int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
    neko_entitytainer_bucketlist *bucket_list = entitytainer->bucket_lists + bucket_list_index;
    int bucket_index = lookup & NEKO_ENT_BucketMask;
    int bucket_offset = bucket_index * bucket_list->bucket_size;
    neko_entitytainer_entity *bucket = (neko_entitytainer_entity *)(bucket_list->bucket_data + bucket_offset);
    int first_free_index = 1;
    for (int i = 1; i < bucket_list->bucket_size; ++i) {
        neko_entitytainer_entity child = bucket[i];
        if (child != NEKO_ENT_InvalidEntity) {
            for (int i_free = first_free_index; i_free < i; ++i_free) {
                if (bucket[i_free] == NEKO_ENT_InvalidEntity) {
                    first_free_index = i_free + 1;
                    bucket[i_free] = child;
                    bucket[i] = NEKO_ENT_InvalidEntity;
                    break;
                }
            }
        }
    }
}

NEKO_API_DECL int neko_ent_save(neko_entitytainer *entitytainer, unsigned char *buffer, int buffer_size) {

    neko_entitytainer_bucketlist *last = &entitytainer->bucket_lists[entitytainer->num_bucket_lists - 1];
    neko_entitytainer_entity *entity_end = last->bucket_data + last->bucket_size * last->total_buckets;
    unsigned char *begin = (unsigned char *)entitytainer;
    unsigned char *end = (unsigned char *)entity_end;
    int size = (int)(end - begin);
    if (size > buffer_size) {
        return size;
    }

    memcpy(buffer, entitytainer, size);
    return size;
}

NEKO_API_DECL neko_entitytainer *neko_ent_load(unsigned char *buffer, int buffer_size) {
    neko_assert(entitytainer__ptr_to_aligned_ptr(buffer, (int)NEKO_ENT_alignof(neko_entitytainer)) == buffer);

    // Fix pointers
    neko_entitytainer *entitytainer = (neko_entitytainer *)buffer;
    buffer += sizeof(neko_entitytainer);
    entitytainer->entry_lookup = (neko_entitytainer_entry *)buffer;
    buffer += sizeof(neko_entitytainer_entry) * entitytainer->entry_lookup_size;
    entitytainer->entry_parent_lookup = (neko_entitytainer_entity *)buffer;
    buffer += sizeof(neko_entitytainer_entity) * entitytainer->entry_lookup_size;

    buffer = (unsigned char *)entitytainer__ptr_to_aligned_ptr(buffer, (int)NEKO_ENT_alignof(neko_entitytainer_bucketlist));
    entitytainer->bucket_lists = (neko_entitytainer_bucketlist *)buffer;

    unsigned char *bucket_list_end = buffer + sizeof(neko_entitytainer_bucketlist) * entitytainer->num_bucket_lists;
    neko_entitytainer_entity *bucket_data_start = (neko_entitytainer_entity *)bucket_list_end;
    neko_entitytainer_entity *bucket_data = bucket_data_start;
    for (int i = 0; i < entitytainer->num_bucket_lists; ++i) {
        neko_entitytainer_bucketlist *list = (neko_entitytainer_bucketlist *)buffer;
        list->bucket_data = bucket_data;
        buffer += sizeof(neko_entitytainer_bucketlist);
        bucket_data += list->bucket_size * list->total_buckets;
    }

    (void)buffer_size;
    neko_assert((unsigned char *)bucket_data <= buffer + buffer_size);
    return entitytainer;
}

NEKO_API_DECL void neko_ent_load_into(neko_entitytainer *entitytainer_dst, const neko_entitytainer *entitytainer_src) {
    // if ( NEKO_ENT_memcmp( &entitytainer_dst->config, &entitytainer_src->config, sizeof( TheEntitytainerConfig
    // ) )
    // ==
    //      0 ) {
    //     memcpy( entitytainer_dst, entitytainer_src, sizeof( TheEntitytainerConfig ) );
    //     return;
    // }

    // Only allow grow for now
    neko_assert(entitytainer_src->config.num_bucket_lists == entitytainer_dst->config.num_bucket_lists);
    for (int i_bl = 0; i_bl < entitytainer_src->config.num_bucket_lists; ++i_bl) {
        neko_assert(entitytainer_src->config.bucket_sizes[i_bl] <= entitytainer_dst->config.bucket_sizes[i_bl]);
        neko_assert(entitytainer_src->config.bucket_list_sizes[i_bl] <= entitytainer_dst->config.bucket_list_sizes[i_bl]);

        if (entitytainer_src->config.bucket_sizes[i_bl] == entitytainer_dst->config.bucket_sizes[i_bl]) {
            int bucket_list_size = sizeof(neko_entitytainer_entity) * entitytainer_src->config.bucket_list_sizes[i_bl] * entitytainer_src->config.bucket_sizes[i_bl];
            memcpy(entitytainer_dst->bucket_lists[i_bl].bucket_data, entitytainer_src->bucket_lists[i_bl].bucket_data, bucket_list_size);
        } else {
            neko_entitytainer_bucketlist *bucket_list_src = &entitytainer_src->bucket_lists[i_bl];
            int bucket_size_src = entitytainer_src->config.bucket_sizes[i_bl];
            neko_entitytainer_bucketlist *bucket_list_dst = &entitytainer_dst->bucket_lists[i_bl];
            int bucket_size_dst = entitytainer_dst->config.bucket_sizes[i_bl];
            for (int i_bucket = 0; i_bucket < entitytainer_src->config.bucket_list_sizes[i_bl]; ++i_bucket) {
                int bucket_offset_src = i_bucket * bucket_size_src;
                neko_entitytainer_entity *bucket_src = bucket_list_src->bucket_data + bucket_offset_src;
                int bucket_offset_dst = i_bucket * bucket_size_dst;
                neko_entitytainer_entity *bucket_dst = bucket_list_dst->bucket_data + bucket_offset_dst;
                memcpy(bucket_dst, bucket_src, bucket_size_src * sizeof(neko_entitytainer_entity));
            }
        }
        entitytainer_dst->bucket_lists[i_bl].first_free_bucket = entitytainer_src->bucket_lists[i_bl].first_free_bucket;
        entitytainer_dst->bucket_lists[i_bl].used_buckets = entitytainer_src->bucket_lists[i_bl].used_buckets;
    }

    memcpy(entitytainer_dst->entry_lookup, entitytainer_src->entry_lookup, sizeof(neko_entitytainer_entry) * entitytainer_src->entry_lookup_size);
    memcpy(entitytainer_dst->entry_parent_lookup, entitytainer_src->entry_parent_lookup, sizeof(neko_entitytainer_entity) * entitytainer_src->entry_lookup_size);

#if NEKO_ENT_DEFENSIVE_CHECKS
    for (TheEntitytainerEntity entity = 0; entity < entitytainer_dst->config.num_entries; ++entity) {
        neko_entitytainer_entry lookup = entitytainer_dst->entry_lookup[entity];
        if (lookup == 0) {
            continue;
        }

        int bucket_list_index = lookup >> NEKO_ENT_BucketListOffset;
        TheEntitytainerBucketList *bucket_list = entitytainer_dst->bucket_lists + bucket_list_index;
        int bucket_index = lookup & NEKO_ENT_BucketMask;
        int bucket_offset = bucket_index * bucket_list->bucket_size;
        TheEntitytainerEntity *bucket = bucket_list->bucket_data + bucket_offset;

        TheEntitytainerEntity count = bucket[0];
        TheEntitytainerEntity found_count = 0;
        for (int i_child1 = 1; i_child1 < bucket_list->bucket_size; ++i_child1) {
            if (found_count == count) {
                break;
            }

            if (bucket[i_child1] == NEKO_ENT_InvalidEntity) {
                continue;
            }

            ++found_count;
            for (int i_child2 = i_child1 + 1; i_child2 < bucket_list->bucket_size; ++i_child2) {
                if (bucket[i_child1] == bucket[i_child2]) {
                    if (entitytainer_dst->remove_with_holes) {
                        neko_assert(entitytainer_dst->keep_capacity_on_remove, "untested");
                        bucket[0]--;
                        bucket[i_child2] = 0;
                        // entitytainer_remove_child_with_holes( entitytainer_dst, entity, bucket[i_child1] );
                    } else {
                        neko_assert(false, "untested");
                    }
                }
            }
        }
    }
#endif
}

static void *entitytainer__ptr_to_aligned_ptr(void *ptr, int align) {
    if (align == 0) {
        return ptr;
    }

    long long int ptr_address = (long long int)ptr;
    int offset = (~(int)ptr_address + 1) & (align - 1);
    void *aligned_ptr = (char *)ptr + offset;
    return aligned_ptr;
}

static bool entitytainer__child_in_bucket(neko_entitytainer_entity *bucket, neko_entitytainer_bucketlist *bucket_list, neko_entitytainer_entity child) {

    neko_entitytainer_entity count = bucket[0];
    int found = 0;
    for (int i = 1; i < bucket_list->bucket_size; ++i) {
        if (found == count) {
            break;
        }

        if (bucket[i] == NEKO_ENT_InvalidEntity) {
            continue;
        }

        if (bucket[i] == child) {
            return true;
        }

        ++found;
    }

    return false;
}

#define TYPE_MIN_ID 1
#define TYPE_MAX_ID 255
#define TYPE_COUNT 256

#define ECS_WORLD (1)
#define ECS_WORLD_UDATA_NAME "__NEKO_ECS_WORLD"

#define WORLD_PROTO_ID 1
#define WORLD_PROTO_DEFINE 2
#define WORLD_COMPONENTS 3
#define WORLD_MATCH_CTX 4
#define WORLD_KEY_EID 5
#define WORLD_KEY_TID 6
#define WORLD_UPVAL_N 6

#define LINK_NIL (-1)
#define LINK_NONE (-2)

#define ENTITY_MAX_COMPONENTS (64)  // 最大组件数量

enum match_mode {
    MATCH_ALL = 0,
    MATCH_DIRTY = 1,
    MATCH_DEAD = 2,
};

struct component {
    int eid;
    int dirty_next;
    int dead_next;
};

struct component_pool {
    int cap;
    int free;
    int dirty_head;
    int dirty_tail;
    int dead_head;
    int dead_tail;
    struct component *buf;
};

// 每个Component类型都有一个数字id称为tid
// 每个Component实例都有一个数字id称为cid
// 根据tid和cid来找到某一个具体的Component实例
struct component_ptr {
    int tid;
    int cid;
};

struct entity {
    int next;
    short cn;
    unsigned char components[32];
    int index[ENTITY_MAX_COMPONENTS];
};

struct match_ctx {
    struct neko_ecs_world_t *w;
    int i;
    int kn;
    int keys[ENTITY_MAX_COMPONENTS];
};

struct neko_ecs_world_t {
    int entity_cap;
    int entity_free;  // TODO:将ENTITY_FREE设置为FIFO 这可以在以后回收EID以避免某些错误
    int entity_dead;
    int type_idx;
    struct entity *entity_buf;
    struct component_pool component_pool[TYPE_COUNT];
};

static inline int component_has(struct entity *e, int tid) { return e->components[tid] < ENTITY_MAX_COMPONENTS; }

static inline void component_clr(struct entity *e, int tid) {
    unsigned char idx = e->components[tid];
    if (idx < ENTITY_MAX_COMPONENTS) {
        e->index[idx] = -1;
        e->components[tid] = ENTITY_MAX_COMPONENTS;
        --e->cn;
    }
}

static inline int component_add(lua_State *L, struct neko_ecs_world_t *w, struct entity *e, int tid) {
    int cid, i;
    struct component *c;
    struct component_pool *cp;
    struct component_ptr *ptr;
    if (e->components[tid] < ENTITY_MAX_COMPONENTS) {
        return luaL_error(L, "Entity(%d) already exist component(%d)", e - w->entity_buf, tid);
    }
    if (e->cn >= ENTITY_MAX_COMPONENTS) {
        return luaL_error(L, "Entity(%d) add to many components", e - w->entity_buf);
    }
    // new component
    cp = &w->component_pool[tid];
    if (cp->free >= cp->cap) {
        cp->cap *= 2;
        cp->buf = neko_safe_realloc(cp->buf, cp->cap * sizeof(cp->buf[0]));
    }
    c = &cp->buf[cp->free++];
    c->eid = e - w->entity_buf;
    c->dirty_next = LINK_NONE;
    c->dead_next = LINK_NONE;
    cid = c - cp->buf;
    // add component into entity
    for (i = 0; i < ENTITY_MAX_COMPONENTS; i++) {
        if (e->index[i] < 0) break;
    }
    assert(i < ENTITY_MAX_COMPONENTS);
    e->components[tid] = i;
    e->index[i] = cid;
    return cid;
}

static inline void component_dead(struct neko_ecs_world_t *w, int tid, int cid) {
    struct component *c;
    struct component_pool *cp;
    cp = &w->component_pool[tid];
    c = &cp->buf[cid];
    if (c->dead_next != LINK_NONE)  // already dead?
        return;
    c->dead_next = LINK_NIL;
    if (cp->dead_tail == LINK_NIL) {
        cp->dead_tail = cid;
        cp->dead_head = cid;
    } else {
        cp->buf[cp->dead_tail].dead_next = cid;
        cp->dead_tail = cid;
    }
    return;
}

static inline void component_dirty(struct neko_ecs_world_t *w, int tid, int cid) {
    struct component *c;
    struct component_pool *cp;
    cp = &w->component_pool[tid];
    c = &cp->buf[cid];
    if (c->dirty_next != LINK_NONE)  // already diryt
        return;
    c->dirty_next = LINK_NIL;
    if (cp->dirty_tail == LINK_NIL) {
        cp->dirty_tail = cid;
        cp->dirty_head = cid;
    } else {
        cp->buf[cp->dirty_tail].dirty_next = cid;
        cp->dirty_tail = cid;
    }
    return;
}

static inline struct entity *entity_alloc(struct neko_ecs_world_t *w) {
    struct entity *e;
    int eid = w->entity_free;
    if (eid < 0) {
        int i = 0;
        int oldcap = w->entity_cap;
        int newcap = oldcap * 2;
        w->entity_cap = newcap;
        w->entity_buf = neko_safe_realloc(w->entity_buf, newcap * sizeof(w->entity_buf[0]));
        w->entity_free = oldcap + 1;
        e = &w->entity_buf[oldcap];
        for (i = w->entity_free; i < newcap - 1; i++) {
            w->entity_buf[i].cn = -1;
            w->entity_buf[i].next = i + 1;
        }
        w->entity_buf[newcap - 1].cn = -1;
        w->entity_buf[newcap - 1].next = LINK_NIL;
    } else {
        e = &w->entity_buf[eid];
        w->entity_free = e->next;
    }
    e->cn = 0;
    memset(e->components, ENTITY_MAX_COMPONENTS, sizeof(e->components));
    memset(e->index, -1, sizeof(e->index));
    e->next = LINK_NONE;
    return e;
}

static inline void entity_dead(struct neko_ecs_world_t *w, struct entity *e) {
    int t;
    if (e->cn < 0) {
        assert(e->next != LINK_NONE);
        return;
    }
    assert(e->next == LINK_NONE);
    for (t = TYPE_MIN_ID; t <= w->type_idx; t++) {
        int i = e->components[t];
        if (i < ENTITY_MAX_COMPONENTS) component_dead(w, t, e->index[i]);
    }
    e->next = w->entity_dead;
    w->entity_dead = e - w->entity_buf;
}

static inline void entity_free(struct neko_ecs_world_t *w, struct entity *e) {
    assert(e->cn == -1);
    e->next = w->entity_free;
    w->entity_free = e - w->entity_buf;
}

static inline int get_typeid(lua_State *L, int stk, int proto_id) {
    int id;
    stk = lua_absindex(L, stk);
    lua_pushvalue(L, stk);
    lua_gettable(L, proto_id);
    id = lua_tointeger(L, -1);
    lua_pop(L, 1);
    luaL_argcheck(L, id >= TYPE_MIN_ID, stk, "invalid type");
    return id;
}

static inline struct entity *get_entity(lua_State *L, struct neko_ecs_world_t *w, int stk) {
    struct entity *e;
    int eid = luaL_checkinteger(L, stk);
    luaL_argcheck(L, eid < w->entity_cap, 2, "eid is invalid");
    e = &w->entity_buf[eid];
    luaL_argcheck(L, e->cn >= 0, 2, "entity is dead");
    return e;
}

static inline int get_cid_in_entity(struct entity *e, int tid) {
    int i = e->components[tid];
    if (i >= ENTITY_MAX_COMPONENTS) return -1;
    return e->index[i];
}

static inline void update_cid_in_entity(struct entity *e, int tid, int cid) {
    int i = e->components[tid];
    assert(i < ENTITY_MAX_COMPONENTS);
    e->index[i] = cid;
}

static int __neko_ecs_world_end(lua_State *L) {
    struct neko_ecs_world_t *w;
    int type, tid;
    struct component_pool *cp;
    w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    tid = w->type_idx;  // 总数-1(索引)
    for (int i = 0; i <= tid; i++) {
        cp = &w->component_pool[i];
        neko_safe_free(cp->buf);
    }
    neko_safe_free(w->entity_buf);
    return 0;
}

static int __neko_ecs_world_register(lua_State *L) {
    struct neko_ecs_world_t *w;
    int type, tid;
    struct component_pool *cp;
    w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    // check if 'componentA' has been declared
    lua_pushvalue(L, 2);
    type = lua_gettable(L, -2);
    luaL_argcheck(L, type == LUA_TNIL, 1, "duplicated component type");
    lua_pop(L, 1);
    // new component type
    tid = ++w->type_idx;
    luaL_argcheck(L, tid >= TYPE_MIN_ID && tid <= TYPE_MAX_ID, 1, "component type is too may");
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    lua_createtable(L, 128, 0);
    lua_seti(L, -2, tid);
    lua_pop(L, 1);
    cp = &w->component_pool[tid];
    cp->cap = 64;
    cp->free = 0;
    cp->dirty_head = LINK_NIL;
    cp->dirty_tail = LINK_NIL;
    cp->dead_head = LINK_NIL;
    cp->dead_tail = LINK_NIL;
    cp->buf = neko_safe_malloc(cp->cap * sizeof(cp->buf[0]));
    // set proto id
    lua_pushvalue(L, 2);
    lua_pushinteger(L, tid);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    ////////
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_DEFINE);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    return 0;
}

static int __neko_ecs_world_new_entity(lua_State *L) {
    int i, components, proto_id;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = entity_alloc(w);
    int eid = e - w->entity_buf;
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    components = lua_gettop(L);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    proto_id = components + 1;
    lua_pushnil(L); /* first key */
    while (lua_next(L, 2) != 0) {
        int cid;
        int tid = get_typeid(L, -2, proto_id);
        lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_EID);
        lua_pushinteger(L, eid);
        lua_settable(L, -3);
        lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_TID);
        lua_pushinteger(L, tid);
        lua_settable(L, -3);
        cid = component_add(L, w, e, tid);
        luaL_argcheck(L, cid >= 0, 2, "entity has duplicated component");
        lua_geti(L, components, tid);
        lua_pushvalue(L, -2);
        lua_seti(L, -2, cid);
        lua_pop(L, 2);
    }
    lua_pop(L, 1);
    lua_pushinteger(L, eid);
    return 1;
}

static int __neko_ecs_world_del_entity(lua_State *L) {
    int i;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
    entity_dead(w, e);
    return 0;
}

static int __neko_ecs_world_get_component(lua_State *L) {
    int i, top, proto_id, components;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
    top = lua_gettop(L);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    proto_id = top + 1;
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    components = top + 2;
    for (i = 3; i <= top; i++) {
        int tid = get_typeid(L, i, proto_id);
        int cid = get_cid_in_entity(e, tid);
        if (cid >= 0) {
            lua_geti(L, components, tid);
            lua_geti(L, -1, cid);
            lua_replace(L, -2);
        } else {
            lua_pushnil(L);
        }
    }
    return (top - 3 + 1);
}

static int __neko_ecs_world_add_component(lua_State *L) {
    int tid, cid;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    tid = get_typeid(L, 3, lua_gettop(L));
    cid = component_add(L, w, e, tid);
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    lua_geti(L, -1, tid);
    lua_pushvalue(L, 4);
    lua_seti(L, -2, cid);
    lua_pop(L, 3);
    return 0;
}

static int __neko_ecs_world_remove_component(lua_State *L) {
    int i, proto_id;
    int tid, cid;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *e = get_entity(L, w, 2);
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    proto_id = lua_gettop(L);
    for (i = 3; i <= (proto_id - 1); i++) {
        tid = get_typeid(L, i, proto_id);
        cid = get_cid_in_entity(e, tid);
        component_dead(w, tid, cid);
    }
    lua_pop(L, 1);
    return 0;
}

static int __neko_ecs_world_touch_component(lua_State *L) {
    int eid, tid, cid;
    struct entity *e;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_EID);
    lua_gettable(L, 2);
    eid = luaL_checkinteger(L, -1);
    luaL_argcheck(L, eid > 0 && eid < w->entity_cap, 2, "invalid component");
    lua_getiuservalue(L, ECS_WORLD, WORLD_KEY_TID);
    lua_gettable(L, 2);
    tid = luaL_checkinteger(L, -1);
    luaL_argcheck(L, tid >= TYPE_MIN_ID && tid <= TYPE_MAX_ID, 2, "invalid component");
    e = &w->entity_buf[eid];
    cid = get_cid_in_entity(e, tid);
    component_dirty(w, tid, cid);
    return 0;
}

static void push_result(lua_State *L, int *keys, int kn, struct entity *e) {
    int i;
    if (e != NULL) {  // match one
        int components;
        lua_getiuservalue(L, 1, 1);
        lua_getiuservalue(L, -1, WORLD_COMPONENTS);
        lua_replace(L, -2);
        components = lua_gettop(L);
        for (i = 0; i < kn; i++) {
            int tid = keys[i];
            int cid = get_cid_in_entity(e, tid);
            lua_geti(L, components, tid);
            lua_geti(L, -1, cid);
            lua_replace(L, -2);
        }
    } else {
        for (i = 0; i < kn; i++) lua_pushnil(L);
    }
}

static inline struct entity *restrict_component(struct entity *ebuf, struct component *c, int *keys, int kn) {
    int i;
    struct entity *e;
    e = &ebuf[c->eid];
    for (i = 1; i < kn; i++) {
        if (!component_has(e, keys[i])) return NULL;
    }
    return e;
}

// match_all(match_ctx *mctx, nil)
static int match_all(lua_State *L) {
    int i;
    int mi, free;
    int kn, *keys;
    struct entity *e = NULL;
    struct component_pool *cp;
    struct match_ctx *mctx = lua_touserdata(L, 1);
    struct neko_ecs_world_t *w = mctx->w;
    struct entity *entity_buf = w->entity_buf;
    kn = mctx->kn;
    keys = mctx->keys;
    cp = &w->component_pool[mctx->keys[0]];
    mi = mctx->i;
    free = cp->free;
    while (mi < free) {
        struct component *c = &cp->buf[mi++];
        if (c->dead_next == LINK_NONE) {
            e = restrict_component(entity_buf, c, keys, kn);
            if (e != NULL) break;
        }
    }
    mctx->i = mi;
    push_result(L, keys, kn, e);
    return kn;
}

static int match_dirty(lua_State *L) {
    int next;
    int kn, *keys;
    struct entity *e = NULL;
    struct component_pool *cp;
    struct match_ctx *mctx = lua_touserdata(L, 1);
    struct neko_ecs_world_t *w = mctx->w;
    struct entity *entity_buf = w->entity_buf;
    keys = mctx->keys;
    kn = mctx->kn;
    next = mctx->i;
    cp = &w->component_pool[keys[0]];
    while (next != LINK_NIL) {
        struct component *c = &cp->buf[next];
        next = c->dirty_next;
        if (c->dead_next == LINK_NONE) {
            e = restrict_component(entity_buf, c, keys, kn);
            if (e != NULL) break;
        }
    }
    mctx->i = next;
    push_result(L, keys, kn, e);
    return kn;
}

static int match_dead(lua_State *L) {
    int next;
    int kn, *keys;
    struct entity *e = NULL;
    struct component_pool *cp;
    struct match_ctx *mctx = lua_touserdata(L, 1);
    struct neko_ecs_world_t *w = mctx->w;
    struct entity *entity_buf = w->entity_buf;
    keys = mctx->keys;
    kn = mctx->kn;
    next = mctx->i;
    cp = &w->component_pool[keys[0]];
    while (next != LINK_NIL) {
        struct component *c = &cp->buf[next];
        next = c->dead_next;
        e = restrict_component(entity_buf, c, keys, kn);
        if (e != NULL) break;
    }
    mctx->i = next;
    push_result(L, keys, kn, e);
    return kn;
}

static int __neko_ecs_world_match_component(lua_State *L) {
    int i;
    size_t sz;
    struct neko_ecs_world_t *w;
    enum match_mode mode;
    int (*iter)(lua_State *L) = NULL;
    struct match_ctx *mctx = NULL;
    const char *m = luaL_checklstring(L, 2, &sz);
    int top = lua_gettop(L);
    w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    switch (sz) {
        case 3:
            if (m[0] == 'a' && m[1] == 'l' && m[2] == 'l') {
                iter = match_all;
                mode = MATCH_ALL;
            }
            break;
        case 5:
            if (memcmp(m, "dirty", 5) == 0) {
                iter = match_dirty;
                mode = MATCH_DIRTY;
            }
            break;
        case 4:
            if (memcmp(m, "dead", 4) == 0) {
                iter = match_dead;
                mode = MATCH_DEAD;
            }
            break;
    }
    if (iter == NULL) return luaL_argerror(L, 2, "mode can only be[all,dirty,dead]");
    luaL_argcheck(L, top >= 3, 3, "lost the component name");
    luaL_argcheck(L, top < neko_arr_size(mctx->keys), top, "too many component");
    lua_getiuservalue(L, ECS_WORLD, WORLD_PROTO_ID);
    lua_pushcfunction(L, iter);
    lua_getiuservalue(L, ECS_WORLD, WORLD_MATCH_CTX);
    mctx = lua_touserdata(L, -1);
    mctx->w = w;
    mctx->kn = 0;
    for (i = 3; i <= top; i++) mctx->keys[mctx->kn++] = get_typeid(L, i, top + 1);
    switch (mode) {
        case MATCH_ALL:
            mctx->i = 0;
            break;
        case MATCH_DIRTY:
            mctx->i = w->component_pool[mctx->keys[0]].dirty_head;
            break;
        case MATCH_DEAD:
            mctx->i = w->component_pool[mctx->keys[0]].dead_head;
            break;
    }
    lua_pushnil(L);
    return 3;
}

static int __neko_ecs_world_update(lua_State *L) {
    int next;
    int t, components;
    struct component_pool *pool;
    struct neko_ecs_world_t *w = luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    struct entity *entity_buf = w->entity_buf;
    // clear dead entity
    next = w->entity_dead;
    while (next != LINK_NIL) {
        struct entity *e;
        e = &entity_buf[next];
        e->cn = -1;
        next = e->next;
        entity_free(w, e);
    }
    pool = w->component_pool;
    // clear dead component
    lua_getiuservalue(L, ECS_WORLD, WORLD_COMPONENTS);
    for (t = 0; t <= w->type_idx; t++) {
        int w = 0, r = 0, free;
        struct component *buf;
        struct component_pool *cp;
        cp = &pool[t];
        cp->dirty_head = LINK_NIL;
        cp->dirty_tail = LINK_NIL;
        cp->dead_head = LINK_NIL;
        cp->dead_tail = LINK_NIL;
        lua_geti(L, -1, t);
        buf = cp->buf;
        free = cp->free;
        for (r = 0; r < free; r++) {
            struct component *c;
            c = &buf[r];
            c->dirty_next = LINK_NONE;
            if (c->dead_next == LINK_NONE) {  // alive
                if (w != r) {
                    struct entity *e;
                    e = &entity_buf[c->eid];
                    buf[w] = *c;
                    lua_geti(L, -1, r);
                    lua_seti(L, -2, w);
                    update_cid_in_entity(e, t, w);
                }
                w++;
            } else {  // dead component
                struct entity *e;
                e = &entity_buf[c->eid];
                if (e->next == LINK_NONE) component_clr(e, t);
            }
        }
        cp->free = w;
        while (w < free) {
            lua_pushnil(L);
            lua_seti(L, -2, w);
            w++;
        }
        lua_pop(L, 1);
    }
    return 0;
}

static void print_value(lua_State *L, int stk, int tab) {
    switch (lua_type(L, stk)) {
        case LUA_TTABLE:
            printf("%*p=>", tab * 4, lua_topointer(L, stk));
            break;
        case LUA_TSTRING:
            printf("%*s=>", tab * 4, lua_tostring(L, stk));
            break;
        default:
            printf("%*lld=>", tab * 4, (long long)lua_tointeger(L, stk));
            break;
    }
}

static void dump_table(lua_State *L, int stk, int tab) {
    lua_pushnil(L); /* first key */
    while (lua_next(L, stk) != 0) {
        // print key
        print_value(L, -2, tab);
        switch (lua_type(L, -1)) {
            case LUA_TTABLE:
                printf("{\n");
                dump_table(L, lua_absindex(L, -1), tab + 1);
                printf("}\n");
                break;
            case LUA_TSTRING:
                printf("%*s\n", tab * 4, lua_tostring(L, -1));
                break;
            default:
                printf("%*lld\n", tab * 4, (long long)lua_tointeger(L, -1));
                break;
        }
        lua_pop(L, 1);
    }
}

static void dump_upval(lua_State *L, int upval) {
    const char *name[] = {
            "PROTO_ID",
            "PROTO_DEFINE",
            "COMPONENTS",
    };
    printf("==dump== %s\n", name[upval - 1]);
    if (lua_getiuservalue(L, ECS_WORLD, upval) == LUA_TTABLE) dump_table(L, lua_gettop(L), 0);
    lua_pop(L, 1);
}

static int __neko_ecs_world_dump(lua_State *L) {
    int i;
    for (i = 1; i <= WORLD_COMPONENTS; i++) dump_upval(L, i);
    return 0;
}

int __neko_ecs_create_world(lua_State *L) {
    int i;
    struct neko_ecs_world_t *w;
    struct match_ctx *mctx;
    w = (struct neko_ecs_world_t *)lua_newuserdatauv(L, sizeof(*w), WORLD_UPVAL_N);
    memset(w, 0, sizeof(*w));
    w->entity_cap = 128;
    w->entity_free = 0;
    w->entity_dead = LINK_NIL;
    w->type_idx = 0;
    w->entity_buf = neko_safe_malloc(w->entity_cap * sizeof(w->entity_buf[0]));
    for (i = 0; i < w->entity_cap - 1; i++) {
        w->entity_buf[i].cn = -1;
        w->entity_buf[i].next = i + 1;
    }
    w->entity_buf[w->entity_cap - 1].cn = -1;
    w->entity_buf[w->entity_cap - 1].next = LINK_NIL;
    if (luaL_getmetatable(L, ECS_WORLD_UDATA_NAME) == LUA_TNIL) {
        luaL_Reg world_mt[] = {
                {"__index", NULL},
                {"__name", NULL},
                {"__gc", __neko_ecs_world_end},
                {"register", __neko_ecs_world_register},
                {"new", __neko_ecs_world_new_entity},
                {"del", __neko_ecs_world_del_entity},
                {"get", __neko_ecs_world_get_component},
                {"add", __neko_ecs_world_add_component},
                {"remove", __neko_ecs_world_remove_component},
                {"touch", __neko_ecs_world_touch_component},
                {"match", __neko_ecs_world_match_component},
                {"update", __neko_ecs_world_update},
                {"dump", __neko_ecs_world_dump},
                {NULL, NULL},
        };
        lua_pop(L, 1);
        luaL_newlibtable(L, world_mt);
        luaL_setfuncs(L, world_mt, 0);
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        lua_pushliteral(L, ECS_WORLD_UDATA_NAME);
        lua_setfield(L, -2, "__name");
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, ECS_WORLD_UDATA_NAME);
    }
    lua_setmetatable(L, -2);

    lua_createtable(L, 0, 128);
    lua_setiuservalue(L, 1, WORLD_PROTO_ID);

    lua_createtable(L, 0, 128);
    lua_setiuservalue(L, 1, WORLD_PROTO_DEFINE);

    lua_createtable(L, TYPE_MAX_ID, 0);
    lua_setiuservalue(L, 1, WORLD_COMPONENTS);

    mctx = lua_newuserdatauv(L, sizeof(*mctx), 1);
    lua_pushvalue(L, 1);
    lua_setiuservalue(L, -2, 1);
    lua_setiuservalue(L, 1, WORLD_MATCH_CTX);

    lua_pushliteral(L, "__eid");
    lua_setiuservalue(L, 1, WORLD_KEY_EID);

    lua_pushliteral(L, "__tid");
    lua_setiuservalue(L, 1, WORLD_KEY_TID);

    return 1;
}

/*=============================
// ECS
=============================*/

neko_ecs_stack *neko_ecs_stack_make(u64 capacity) {
    neko_ecs_stack *s = (neko_ecs_stack *)neko_malloc(sizeof(neko_ecs_stack));
    s->data = (u32 *)neko_malloc(sizeof(*s->data) * capacity);
    s->capacity = capacity;
    s->top = 0;
    s->empty = true;

    return s;
}

void neko_ecs_stack_destroy(neko_ecs_stack *s) {
    neko_free(s->data);
    neko_free(s);
}

b32 neko_ecs_stack_empty(neko_ecs_stack *s) { return s->empty; }

b32 neko_ecs_stack_full(neko_ecs_stack *s) { return s->top == s->capacity; }

u64 neko_ecs_stack_capacity(neko_ecs_stack *s) { return s->capacity; }

u64 neko_ecs_stack_top(neko_ecs_stack *s) { return s->top; }

u32 neko_ecs_stack_peek(neko_ecs_stack *s) {
    if (s->empty) {
        neko_log_warning("[ECS] Failed to peek, stack is full");
        return 0;
    }
    return s->data[s->top - 1];
}

void neko_ecs_stack_push(neko_ecs_stack *s, u32 val) {
    if (neko_ecs_stack_full(s)) {
        neko_log_warning("[ECS] Failed to push %u, stack is full", val);
        return;
    }

    s->empty = false;
    s->data[s->top++] = val;
}

u32 neko_ecs_stack_pop(neko_ecs_stack *s) {
    if (s->empty) {
        neko_log_warning("[ECS] Failed to pop, stack is empty");
        return 0;
    }

    if (s->top == 1) s->empty = true;
    return s->data[--s->top];
}

neko_ecs_component_pool neko_ecs_component_pool_make(u32 count, u32 size, neko_ecs_component_destroy destroy_func) {
    neko_ecs_component_pool pool;
    pool.data = neko_malloc(count * size);
    pool.count = count;
    pool.size = size;
    pool.destroy_func = destroy_func;
    pool.indexes = neko_ecs_stack_make(count);

    for (u32 i = count; i-- > 0;) {
        neko_ecs_stack_push(pool.indexes, i);
    }

    return pool;
}

void neko_ecs_component_pool_destroy(neko_ecs_component_pool *pool) {
    // for (u32 i = 0; i < pool->count; i++) {
    //     u8* ptr = (u8*)((u8*)pool->data + (i * pool->size));
    //     if (pool->destroy_func) pool->destroy_func(ptr);
    // }
    neko_free(pool->data);
    neko_ecs_stack_destroy(pool->indexes);
}

void neko_ecs_component_pool_push(neko_ecs_component_pool *pool, u32 index) {
    u8 *ptr = (u8 *)((u8 *)pool->data + (index * pool->size));
    if (pool->destroy_func) pool->destroy_func(ptr);
    neko_ecs_stack_push(pool->indexes, index);
}

u32 neko_ecs_component_pool_pop(neko_ecs_component_pool *pool, void *data) {
    u32 index = neko_ecs_stack_pop(pool->indexes);
    u8 *ptr = (u8 *)((u8 *)pool->data + (index * pool->size));
    if (NULL != data)
        memcpy(ptr, data, pool->size);  // 初始化组件数据是以深拷贝进行
    else
        memset(ptr, 0, pool->size);  // 默认组件数据为NULL 一般发生在组件数据需要特殊初始化
    return index;
}

neko_ecs *neko_ecs_make(u32 max_entities, u32 component_count, u32 system_count) {
    neko_ecs *ecs = (neko_ecs *)neko_malloc(sizeof(*ecs));
    ecs->max_entities = max_entities;
    ecs->component_count = component_count;
    ecs->system_count = system_count;
    ecs->indexes = neko_ecs_stack_make(max_entities);
    ecs->max_index = 0;
    ecs->versions = (u32 *)neko_malloc(max_entities * sizeof(u32));
    ecs->components = (u32 *)neko_malloc(max_entities * component_count * sizeof(u32));
    ecs->component_masks = (b32 *)neko_malloc(max_entities * component_count * sizeof(b32));
    ecs->pool = (neko_ecs_component_pool *)neko_malloc(component_count * sizeof(*ecs->pool));
    ecs->systems = (neko_ecs_system *)neko_malloc(system_count * sizeof(*ecs->systems));
    ecs->systems_top = 0;

    for (u32 i = max_entities; i-- > 0;) {
        neko_ecs_stack_push(ecs->indexes, i);

        ecs->versions[i] = 0;
        for (u32 j = 0; j < component_count; j++) {
            ecs->components[i * component_count + j] = 0;
            ecs->component_masks[i * component_count + j] = 0;
        }
    }

    for (u32 i = 0; i < system_count; i++) {
        ecs->systems[i].func = NULL;
    }

    for (u32 i = 0; i < ecs->component_count; i++) {
        ecs->pool[i].data = NULL;
    }

    return ecs;
}

void neko_ecs_destroy(neko_ecs *ecs) {
    for (u32 i = 0; i < ecs->component_count; i++) {
        neko_ecs_component_pool_destroy(&ecs->pool[i]);
    }

    neko_ecs_stack_destroy(ecs->indexes);

    neko_free(ecs->versions);
    neko_free(ecs->components);
    neko_free(ecs->component_masks);
    neko_free(ecs->pool);
    neko_free(ecs->systems);

    neko_free(ecs);
}

void neko_ecs_register_component(neko_ecs *ecs, neko_ecs_component_type component_type, u32 count, u32 size, neko_ecs_component_destroy destroy_func) {
    if (ecs->pool[component_type].data != NULL) {
        neko_log_warning("[ECS] Registered Component type %u more than once.", component_type);
        return;
    }

    if (count * size <= 0) {
        neko_log_warning("[ECS] Registering Component type %u (count*size) is less than 0.", component_type);
        return;
    }

    ecs->pool[component_type] = neko_ecs_component_pool_make(count, size, destroy_func);
}

void neko_ecs_register_system(neko_ecs *ecs, neko_ecs_system_func func, neko_ecs_system_type type) {
    neko_ecs_system *sys = &ecs->systems[ecs->systems_top++];
    sys->func = func;
    sys->type = type;
}

void neko_ecs_run_systems(neko_ecs *ecs, neko_ecs_system_type type) {
    for (u32 i = 0; i < ecs->systems_top; i++) {
        neko_ecs_system *sys = &ecs->systems[i];
        if (sys->type == type) sys->func(ecs);
    }
}

void neko_ecs_run_system(neko_ecs *ecs, u32 system_index) { ecs->systems[system_index].func(ecs); }

u32 neko_ecs_for_count(neko_ecs *ecs) { return ecs->max_index + 1; }

neko_ecs_ent neko_ecs_get_ent(neko_ecs *ecs, u32 index) { return __neko_ecs_ent_id(index, ecs->versions[index]); }

neko_ecs_ent neko_ecs_ent_make(neko_ecs *ecs) {
    u32 index = neko_ecs_stack_pop(ecs->indexes);
    u32 ver = ecs->versions[index];

    if (index > ecs->max_index) ecs->max_index = index;

    return __neko_ecs_ent_id(index, ver);
}

void neko_ecs_ent_destroy(neko_ecs *ecs, neko_ecs_ent e) {
    u32 index = __neko_ecs_ent_index(e);

    ecs->versions[index]++;
    for (u32 i = 0; i < ecs->component_count; i++) {
        neko_ecs_ent_remove_component(ecs, e, i);
    }

    neko_ecs_stack_push(ecs->indexes, index);
}

void neko_ecs_ent_add_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type, void *component_data) {
    u32 index = __neko_ecs_ent_index(e);

    if (neko_ecs_ent_has_component(ecs, e, type)) {
        neko_log_warning("[ECS] Component %u already exists on neko_ecs_ent %lu (Index %u)", type, e, index);
        return;
    }

    neko_ecs_component_pool *pool = &ecs->pool[type];
    u32 c_index = neko_ecs_component_pool_pop(pool, component_data);
    ecs->components[index * ecs->component_count + type] = c_index;
    ecs->component_masks[index * ecs->component_count + type] = true;
}

void neko_ecs_ent_remove_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type) {
    u32 index = __neko_ecs_ent_index(e);

    if (!neko_ecs_ent_has_component(ecs, e, type)) {
        neko_log_warning("[ECS] Component %u doesn't exist on neko_ecs_ent %lu (Index %u)", type, e, index);
        return;
    }

    neko_ecs_component_pool *pool = &ecs->pool[type];
    neko_ecs_component_pool_push(pool, ecs->components[index * ecs->component_count + type]);
    ecs->component_masks[index * ecs->component_count + type] = false;
}

void *neko_ecs_ent_get_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type) {
    u32 index = __neko_ecs_ent_index(e);

    if (!neko_ecs_ent_has_component(ecs, e, type)) {
        neko_log_warning("[ECS] Trying to get non existent component %u on neko_ecs_ent %lu (Index %u)", type, e, index);
        return NULL;
    }

    u32 c_index = ecs->components[index * ecs->component_count + type];
    u8 *ptr = (u8 *)((u8 *)ecs->pool[type].data + (c_index * ecs->pool[type].size));
    return ptr;
}

b32 neko_ecs_ent_is_valid(neko_ecs *ecs, neko_ecs_ent e) { return ecs->versions[__neko_ecs_ent_index(e)] == __neko_ecs_ent_ver(e); }

b32 neko_ecs_ent_has_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type component_type) { return ecs->component_masks[__neko_ecs_ent_index(e) * ecs->component_count + component_type]; }

b32 neko_ecs_ent_has_mask(neko_ecs *ecs, neko_ecs_ent e, u32 component_type_count, neko_ecs_component_type component_types[]) {
    for (u32 i = 0; i < component_type_count; i++) {
        if (!neko_ecs_ent_has_component(ecs, e, component_types[i])) return false;
    }

    return true;
}

u32 neko_ecs_ent_get_version(neko_ecs *ecs, neko_ecs_ent e) { return ecs->versions[__neko_ecs_ent_index(e)]; }

void neko_ecs_ent_print(neko_ecs *ecs, neko_ecs_ent e) {
    u32 index = __neko_ecs_ent_index(e);

    neko_printf("---- neko_ecs_ent ----\nIndex: %d\nVersion: %d\nMask: ", index, ecs->versions[index]);

    for (u32 i = ecs->component_count; i-- > 0;) {
        neko_printf("%u", ecs->component_masks[index * ecs->component_count + i]);
    }
    neko_printf("\n");

    for (u32 i = 0; i < ecs->component_count; i++) {
        if (neko_ecs_ent_has_component(ecs, e, i)) {
            // neko_println("Component Type: %s (Index: %d)", std::string(neko::enum_name((ComponentType)i)).c_str(), ecs->components[index * ecs->component_count + i]);
        }
    }

    neko_println("----------------");
}

neko_ecs_ent_view neko_ecs_ent_view_single(neko_ecs *ecs, neko_ecs_component_type component_type) {

    neko_ecs_ent_view view = neko_default_val();

    view.view_type = component_type;
    view.ecs = ecs;

    for (view.i = 0; view.i < neko_ecs_for_count(view.ecs); view.i++) {
        neko_ecs_ent e = neko_ecs_get_ent(view.ecs, view.i);
        if (neko_ecs_ent_has_component(view.ecs, e, component_type)) {
            view.ent = e;
            view.valid = true;
            return view;
        }
    }

    view.valid = false;
    return view;
}

b32 neko_ecs_ent_view_valid(neko_ecs_ent_view *view) {
    neko_assert(view);
    return (view->valid && view->i != neko_ecs_for_count(view->ecs));
}

void neko_ecs_ent_view_next(neko_ecs_ent_view *view) {
    neko_assert(view->ecs);
    neko_assert(view);

    view->valid = false;  // 重置有效状态
    view->i++;            // next

    while (view->i < neko_ecs_for_count(view->ecs)) {
        neko_ecs_ent e = neko_ecs_get_ent(view->ecs, view->i);
        if (neko_ecs_ent_has_component(view->ecs, e, view->view_type)) {
            view->ent = e;
            view->valid = true;
            break;
        } else {
            view->i++;
        }
    }
}

#if 0

ECS_COMPONENT_DECLARE(position_t);
ECS_COMPONENT_DECLARE(velocity_t);
ECS_COMPONENT_DECLARE(bounds_t);
ECS_COMPONENT_DECLARE(color_t);

void neko_ecs_com_init(ecs_world_t *world) {
    // Register component with world
    ECS_COMPONENT_DEFINE(world, position_t);
    ECS_COMPONENT_DEFINE(world, velocity_t);
    ECS_COMPONENT_DEFINE(world, bounds_t);
    ECS_COMPONENT_DEFINE(world, color_t);
}

#endif