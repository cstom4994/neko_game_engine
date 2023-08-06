#ifndef NEKO_META_CLASS_H
#define NEKO_META_CLASS_H

#include "engine/common/neko_containers.h"
#include "engine/common/neko_types.h"

#define restrict

// Forward Decl.
struct neko_object;
struct neko_byte_buffer;

typedef enum {
    neko_meta_property_type_b8,
    neko_meta_property_type_u8,
    neko_meta_property_type_s8,
    neko_meta_property_type_u16,
    neko_meta_property_type_s16,
    neko_meta_property_type_u32,
    neko_meta_property_type_s32,
    neko_meta_property_type_s64,
    neko_meta_property_type_u64,
    neko_meta_property_type_f32,
    neko_meta_property_type_f64,
    neko_meta_property_type_vec2,
    neko_meta_property_type_vec3,
    neko_meta_property_type_vec4,
    neko_meta_property_type_quat,
    neko_meta_property_type_vqs,
    neko_meta_property_type_mat4,
    neko_meta_property_type_entity,
    neko_meta_property_type_const_str,
    neko_meta_property_type_uuid,
    neko_meta_property_type_enum,
    neko_meta_property_type_object,
    neko_meta_property_type_count,
} neko_meta_property_type;

// Type alias of unsigned 16 bit integer as a meta class id
typedef u16 neko_meta_class_id;

typedef struct {
    neko_meta_property_type type;
    u16 offset;
    const char* label;
} neko_meta_property;

#define neko_meta_property_set_value(prop, object, type, val) *(type*)((u8*)object + prop->offset) = val

#define neko_meta_property_get_value(prop, object, type) *((type*)((u8*)object + prop->offset))

#define neko_type_to_meta_property_type(cls) __neko_type_to_meta_property_type((#cls))

#define neko_type_to_meta_property_str(cls) __neko_type_to_meta_property_str((#cls))

neko_static_inline neko_meta_property neko_meta_property_ctor(const char* _label, neko_meta_property_type _type, u16 _offset) {
    neko_meta_property prop;
    prop.label = _label;
    prop.type = _type;
    prop.offset = _offset;
    return prop;
}

neko_static_inline const char* neko_meta_property_to_str(neko_meta_property_type type) {
    switch (type) {
        case neko_meta_property_type_u8:
            return "u8";
            break;
        case neko_meta_property_type_s8:
            return "s8";
            break;
        case neko_meta_property_type_u16:
            return "u16";
            break;
        case neko_meta_property_type_s16:
            return "s16";
            break;
        case neko_meta_property_type_u32:
            return "u32";
            break;
        case neko_meta_property_type_s32:
            return "s32";
            break;
        case neko_meta_property_type_u64:
            return "u64";
            break;
        case neko_meta_property_type_s64:
            return "s64";
            break;
        case neko_meta_property_type_f32:
            return "f32";
            break;
        case neko_meta_property_type_f64:
            return "f64";
            break;
        case neko_meta_property_type_vec2:
            return "neko_vec2";
            break;
        case neko_meta_property_type_vec3:
            return "neko_vec3";
            break;
        case neko_meta_property_type_vec4:
            return "neko_vec4";
            break;
        case neko_meta_property_type_mat4:
            return "neko_mat4";
            break;
        case neko_meta_property_type_quat:
            return "neko_quat";
            break;
        case neko_meta_property_type_vqs:
            return "neko_vqs";
            break;
        case neko_meta_property_type_const_str:
            return "const_str";
            break;
        case neko_meta_property_type_entity:
            return "neko_entity";
            break;
        case neko_meta_property_type_object:
            return "neko_object";
            break;
        case neko_meta_property_type_uuid:
            return "neko_uuid";
            break;
        case neko_meta_property_type_enum:
            return "enum";
            break;
        default:
            return "unknown";
            break;
    };
}

typedef neko_meta_property* neko_meta_property_ptr;
// Hash table := key: u32, val: neko_meta_property_ptr
neko_hash_table_decl(u32, neko_meta_property_ptr, neko_hash_u32, neko_hash_key_comp_std_type);

typedef struct neko_meta_class {
    neko_meta_class_id id;
    neko_meta_property* properties;
    u16 property_count;
    const char* label;
    neko_hash_table(u32, neko_meta_property_ptr) property_name_to_index_table;
    neko_result (*serialize_func)(struct neko_object*, struct neko_byte_buffer*);
    neko_result (*deserialize_func)(struct neko_object*, struct neko_byte_buffer*);
} neko_meta_class;

/*============================================================
// Meta Class Registry
============================================================*/

typedef struct {
    // Has a list of registry information that can be index by ID
    neko_meta_class* classes;
    u32 count;
    neko_hash_table(u32, u32) class_idx_ht;
} neko_meta_class_registry;

const neko_meta_class* neko_meta_class_registry_get_class_by_label(neko_meta_class_registry* restrict registry, const char* label);

/*============================================================
// Function Decls
============================================================*/

const neko_meta_property* neko_meta_class_get_property_by_name(struct neko_object* obj, const char* name);

const char* neko_meta_class_get_label(neko_meta_class* restrict cls);

void neko_meta_class_registry_init_meta_properties(neko_meta_class_registry* restrict registry);

neko_meta_class* neko_meta_class_get(void* _obj);

neko_result neko_meta_class_registry_init(neko_meta_class_registry* restrict registry);

void neko_meta_property_value_to_str(neko_meta_property* restrict prop, struct neko_object* restrict obj, u8* restrict buffer, usize buffer_size);

const neko_meta_class* __neko_meta_class_impl(struct neko_object* restrict obj);

const char* __neko_type_name_cls(u32 id);

neko_meta_property_type __neko_type_to_meta_property_type(const char* type);

const char* __neko_type_to_meta_property_str(const char* type);

neko_result __neko_object_serialization_base(struct neko_object* obj, struct neko_byte_buffer* buffer);
neko_result __neko_object_deserialization_base(struct neko_object* obj, struct neko_byte_buffer* buffer);

#endif  // NEKO_META_CLASS_H

/*
    Meta class registry should have mechanism for registering and unregistering meta class information based on hashed key of type name
    Will need slot_map to map from hashed name to slot index in array of meta class information

    typedef struct neko_meta_class_registry {
        neko_slot_map(neko_meta_class_ptr) classes;
    } neko_meta_class_registry;

    const neko_meta_class* neko_register_meta_class(u32 hash, neko_meta_class_desc cls_desc)
    {

    }


*/
