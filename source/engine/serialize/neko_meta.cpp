
#include "engine/serialize/neko_meta.h"

#include "engine/common/neko_util.h"

neko_meta_registry_t neko_meta_registry_new() {
    neko_meta_registry_t meta = {0};
    return meta;
}

void neko_meta_registry_free(neko_meta_registry_t* meta) {
    // TODO
}

u64 __neko_meta_registry_register_class_impl(neko_meta_registry_t* meta, const_str name, const neko_meta_class_decl_t* decl) {
    neko_meta_class_t cls = {0};

    u32 ct = decl->size / sizeof(neko_meta_property_t);
    cls.name = name;
    cls.property_count = ct;
    cls.properties = (neko_meta_property_t*)neko::neko_gc_alloc(&neko::g_gc, decl->size);
    memcpy(cls.properties, decl->properties, decl->size);

    u64 id = neko::hash(name);
    meta->classes_[id] = cls;
    return id;
}

neko_meta_property_t __neko_meta_property_impl(const_str name, size_t offset, neko_meta_property_typeinfo_t type) {
    neko_meta_property_t mp = {0};
    mp.name = name;
    mp.offset = offset;
    mp.type = type;
    return mp;
}

neko_meta_property_typeinfo_t __neko_meta_property_typeinfo_decl_impl(const_str name, u32 id) {
    neko_meta_property_typeinfo_t info = {0};
    info.name = name;
    info.id = id;
    return info;
}

neko_meta_class_t* __neko_meta_class_getp_impl(neko_meta_registry_t* meta, const_str name) {
    u64 id = neko::hash(name);
    return &meta->classes_[id];
}
