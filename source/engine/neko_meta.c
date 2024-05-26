
#include "neko_meta.h"

NEKO_API_DECL neko_meta_registry_t neko_meta_registry_new() {
    neko_meta_registry_t meta = neko_default_val();
    return meta;
}

NEKO_API_DECL void neko_meta_registry_free(neko_meta_registry_t* meta) {
    // 释放类
    for (neko_hash_table_iter it = neko_hash_table_iter_new(meta->classes); neko_hash_table_iter_valid(meta->classes, it); neko_hash_table_iter_advance(meta->classes, it)) {
        neko_meta_class_t* cls = neko_hash_table_iter_getp(meta->classes, it);
        neko_safe_free(cls->properties);
    }
    neko_hash_table_free(meta->classes);
    // 释放枚举
    for (neko_hash_table_iter it = neko_hash_table_iter_new(meta->enums); neko_hash_table_iter_valid(meta->enums, it); neko_hash_table_iter_advance(meta->enums, it)) {
        neko_meta_enum_t* enm = neko_hash_table_iter_getp(meta->enums, it);
        neko_safe_free(enm->values);
    }
    neko_hash_table_free(meta->enums);
}

neko_meta_property_t neko_meta_property_impl(const char* field_type_name, const char* field, size_t size, uint32_t offset, neko_meta_property_type_info_t type) {
    neko_meta_property_t mp = neko_default_val();
    mp.name = field;
    mp.type_name = field_type_name;
    mp.offset = offset;
    mp.size = size;
    mp.type = type;
    return mp;
}

u64 neko_meta_class_register(neko_meta_registry_t* meta, const neko_meta_class_decl_t decl) {
    uint32_t ct = decl.size / sizeof(neko_meta_property_t);
    neko_meta_class_t cls = neko_default_val();
    cls.properties = (neko_meta_property_t*)neko_safe_malloc(decl.size);
    cls.property_count = ct;
    memcpy(cls.properties, decl.properties, decl.size);
    for (uint32_t i = 0; i < cls.property_count; ++i) {
        neko_meta_property_t* prop = &cls.properties[i];
        neko_hash_table_insert(cls.property_map, neko_hash_str64(prop->name), prop);
    }
    cls.name = decl.name;
    cls.base = decl.base ? neko_hash_str64(decl.base) : neko_hash_str64("NULL");
    u64 id = neko_hash_str64(decl.name);
    cls.id = id;
    cls.vtable = decl.vtable ? *decl.vtable : cls.vtable;
    cls.size = decl.cls_size;
    neko_hash_table_insert(meta->classes, id, cls);
    return id;
}

NEKO_API_DECL void neko_meta_class_unregister(neko_meta_registry_t* meta, u64 id) {
    neko_meta_class_t* cls = neko_hash_table_getp(meta->classes, id);
    if (cls->properties) neko_safe_free(cls->properties);
    if (cls->property_map) neko_hash_table_free(cls->property_map);
    neko_hash_table_erase(meta->classes, id);
}

NEKO_API_DECL u64 neko_meta_enum_register(neko_meta_registry_t* meta, const neko_meta_enum_decl_t* decl) {
    uint32_t ct = decl->size / sizeof(neko_meta_enum_value_t);
    neko_meta_enum_t enm = neko_default_val();
    enm.values = (neko_meta_enum_value_t*)neko_safe_malloc(decl->size);
    enm.value_count = ct;
    enm.name = decl->name;
    memcpy(enm.values, decl->values, decl->size);
    u64 id = neko_hash_str64(decl->name);
    enm.id = id;
    neko_hash_table_insert(meta->enums, id, enm);
    return id;
}

NEKO_API_PRIVATE neko_meta_property_type_info_t neko_meta_property_type_decl_impl(const char* name, uint32_t id) {
    neko_meta_property_type_info_t info = neko_default_val();
    info.name = name;
    info.id = id;
    return info;
}

NEKO_API_DECL b32 neko_meta_has_base_class(const neko_meta_registry_t* meta, const neko_meta_class_t* cls) { return (neko_hash_table_key_exists(meta->classes, cls->base)); }

NEKO_API_DECL void* neko_meta_func_get_internal(const neko_meta_class_t* cls, const char* func_name) {
    u64 hash = neko_hash_str64(func_name);
    if (neko_hash_table_exists(cls->vtable.funcs, hash)) {
        return neko_hash_table_get(cls->vtable.funcs, hash);
    }
    return NULL;
}

NEKO_API_DECL void* neko_meta_func_get_internal_w_id(const neko_meta_registry_t* meta, u64 id, const char* func_name) {
    const neko_meta_class_t* cls = neko_hash_table_getp(meta->classes, id);
    u64 hash = neko_hash_str64(func_name);
    if (neko_hash_table_exists(cls->vtable.funcs, hash)) {
        return neko_hash_table_get(cls->vtable.funcs, hash);
    }
    return NULL;
}