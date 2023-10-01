
#include "neko_ecs.h"

#include "engine/neko_component.h"

typedef struct {
    u32 *data;
    u64 capacity;
    u64 top;
    b32 empty;
} neko_ecs_stack;

typedef struct {
    void *data;
    u32 count;
    u32 size;

    neko_ecs_component_destroy destroy_func;

    neko_ecs_stack *indexes;

} neko_ecs_component_pool;

typedef struct {
    neko_ecs_system_func func;
    neko_ecs_system_type type;
} neko_ecs_system;

struct neko_ecs {
    u32 max_entities;
    u32 component_count;
    u32 system_count;

    neko_ecs_stack *indexes;

    // max_index 用来优化
    u32 max_index;
    u32 *versions;

    // components 是组件的索引
    // 最大值为 (实体数 * component_count)
    // 索引通过 (index * comp_count + comp_type) 实现
    // component_masks 的工作原理相同 只是检查 mask 是否启用
    u32 *components;
    b32 *component_masks;

    neko_ecs_component_pool *pool;

    neko_ecs_system *systems;
    u32 systems_top;
};

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
        neko_println("Failed to peek, stack is full");
        return 0;
    }
    return s->data[s->top - 1];
}

void neko_ecs_stack_push(neko_ecs_stack *s, u32 val) {
    if (neko_ecs_stack_full(s)) {
        neko_println("Failed to push %u, stack is full", val);
        return;
    }

    s->empty = false;
    s->data[s->top++] = val;
}

u32 neko_ecs_stack_pop(neko_ecs_stack *s) {
    if (s->empty) {
        neko_println("Failed to pop, stack is empty");
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
    memcpy(ptr, data, pool->size);  // 初始化组件的数据是以深拷贝进行
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
        neko_println("Registered Component type %u more than once.\n", component_type);
        return;
    }

    if (count * size <= 0) {
        neko_println("Registering Component type %u (count*size) is less than 0.\n", component_type);
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
        neko_println("Component %u already exists on neko_ecs_ent %lu (Index %u)", type, e, index);
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
        neko_println("Component %u doesn't exist on neko_ecs_ent %lu (Index %u)", type, e, index);
        return;
    }

    neko_ecs_component_pool *pool = &ecs->pool[type];
    neko_ecs_component_pool_push(pool, ecs->components[index * ecs->component_count + type]);
    ecs->component_masks[index * ecs->component_count + type] = false;
}

void *neko_ecs_ent_get_component(neko_ecs *ecs, neko_ecs_ent e, neko_ecs_component_type type) {
    u32 index = __neko_ecs_ent_index(e);

    if (!neko_ecs_ent_has_component(ecs, e, type)) {
        neko_println("Trying to get non existent component %u on neko_ecs_ent %lu (Index %u)", type, e, index);
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

    printf("---- neko_ecs_ent ----\nIndex: %d\nVersion: %d\nMask: ", __neko_ecs_ent_index(e), ecs->versions[index]);

    for (u32 i = ecs->component_count; i-- > 0;) {
        printf("%u", ecs->component_masks[index * ecs->component_count + i]);
    }
    printf("\n");

    for (u32 i = 0; i < ecs->component_count; i++) {
        if (neko_ecs_ent_has_component(ecs, e, i)) {
            // printf("Component Type: %s (Index: %d)\n", neko::enum_name((ComponentType)i).data(), ecs->components[index * ecs->component_count + i]);
        }
    }

    printf("----------------\n");
}
