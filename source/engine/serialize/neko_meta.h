#ifndef NEKO_META_H
#define NEKO_META_H

#include "engine/common/neko_hash.h"
#include "engine/common/neko_types.h"

typedef enum neko_meta_property_type {
    META_PROPERTY_TYPE_U8 = 0x00,
    META_PROPERTY_TYPE_U16,
    META_PROPERTY_TYPE_U32,
    META_PROPERTY_TYPE_U64,
    META_PROPERTY_TYPE_S8,
    META_PROPERTY_TYPE_S16,
    META_PROPERTY_TYPE_S32,
    META_PROPERTY_TYPE_S64,
    META_PROPERTY_TYPE_F32,
    META_PROPERTY_TYPE_F64,
    META_PROPERTY_TYPE_SIZE_T,
    META_PROPERTY_TYPE_STR,
    META_PROPERTY_TYPE_COUNT
} neko_meta_property_type;

typedef struct neko_meta_property_typeinfo_t {
    const_str name;  // 显示名称
    u32 id;          // 匹配属性类型 用于查找和 switch 语句
} neko_meta_property_typeinfo_t;

extern neko_meta_property_typeinfo_t __neko_meta_property_typeinfo_decl_impl(const_str name, u32 id);

#define neko_meta_property_typeinfo_decl(TY, PROP_TYPE) __neko_meta_property_typeinfo_decl_impl(neko_to_str(TY), PROP_TYPE)

#define neko_meta_property_typeinfo_U8 neko_meta_property_typeinfo_decl(u8, META_PROPERTY_TYPE_U8)
#define neko_meta_property_typeinfo_S8 neko_meta_property_typeinfo_decl(s8, META_PROPERTY_TYPE_S8)
#define neko_meta_property_typeinfo_U16 neko_meta_property_typeinfo_decl(u16, META_PROPERTY_TYPE_U16)
#define neko_meta_property_typeinfo_S16 neko_meta_property_typeinfo_decl(s16, META_PROPERTY_TYPE_S16)
#define neko_meta_property_typeinfo_U32 neko_meta_property_typeinfo_decl(u32, META_PROPERTY_TYPE_U32)
#define neko_meta_property_typeinfo_S32 neko_meta_property_typeinfo_decl(s32, META_PROPERTY_TYPE_S32)
#define neko_meta_property_typeinfo_U64 neko_meta_property_typeinfo_decl(u64, META_PROPERTY_TYPE_U64)
#define neko_meta_property_typeinfo_S64 neko_meta_property_typeinfo_decl(s64, META_PROPERTY_TYPE_S64)
#define neko_meta_property_typeinfo_F32 neko_meta_property_typeinfo_decl(f32, META_PROPERTY_TYPE_F32)
#define neko_meta_property_typeinfo_F64 neko_meta_property_typeinfo_decl(f64, META_PROPERTY_TYPE_F64)
#define neko_meta_property_typeinfo_SIZE_T neko_meta_property_typeinfo_decl(size_t, META_PROPERTY_TYPE_SIZE_T)
#define neko_meta_property_typeinfo_STR neko_meta_property_typeinfo_decl(char*, META_PROPERTY_TYPE_STR)

typedef struct neko_meta_property_t {
    const_str name;                      // 字段的显示名称
    size_t offset;                       // 结构体的字节偏移量
    neko_meta_property_typeinfo_t type;  // 类型信息
} neko_meta_property_t;

extern neko_meta_property_t __neko_meta_property_impl(const_str name, size_t offset, neko_meta_property_typeinfo_t type);

#define neko_meta_property(CLS, FIELD, TYPE) __neko_meta_property_impl(neko_to_str(FIELD), neko_offset(CLS, FIELD), (TYPE))

typedef struct neko_meta_class_t {
    const_str name;                    // 显示名称
    u32 property_count;                // 列表中所有属性的计数
    neko_meta_property_t* properties;  // 所有属性列表
} neko_meta_class_t;

typedef struct neko_meta_registry_t {
    neko_hashmap<neko_meta_class_t> classes_;
} neko_meta_registry_t;

typedef struct neko_meta_class_decl_t {
    neko_meta_property_t* properties;  // 属性数组
    size_t size;                       // 数组大小 以字节为单位
} neko_meta_class_decl_t;

// Functions
extern neko_meta_registry_t neko_meta_registry_new();
extern void neko_meta_registry_free(neko_meta_registry_t* meta);
extern u64 __neko_meta_registry_register_class_impl(neko_meta_registry_t* meta, const_str name, const neko_meta_class_decl_t* decl);
extern neko_meta_class_t* __neko_meta_class_getp_impl(neko_meta_registry_t* meta, const_str name);

#define neko_meta_registry_decl_class(T) neko_meta_property_t __gen_##T##_property[]
#define neko_meta_registry_register_class(META, T)                                                                                                         \
    do {                                                                                                                                                   \
        const u8 __gen_##T##_property_size = neko_array_size(__gen_##T##_property);                                                                        \
        neko_meta_class_decl_t __gen_##T##_class = {.properties = __gen_##T##_property, .size = __gen_##T##_property_size * sizeof(neko_meta_property_t)}; \
        __neko_meta_registry_register_class_impl((META), neko_to_str(T), (&__gen_##T##_class));                                                            \
    } while (0)

#define neko_meta_registry_class_get(META, T) __neko_meta_class_getp_impl(META, neko_to_str(T))

#define neko_meta_registry_getvp(OBJ, T, PROP) ((T*)((u8*)(OBJ) + (PROP)->offset))

#define neko_meta_registry_getv(OBJ, T, PROP) (*((T*)((u8*)(OBJ) + (PROP)->offset)))

#endif
