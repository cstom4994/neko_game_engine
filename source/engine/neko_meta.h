
#ifndef NEKO_META_H
#define NEKO_META_H

#include "neko.h"

#define NEKO_META_PROPERTY_FLAG_POINTER 0x01
#define NEKO_META_PROPERTY_FLAG_DOUBLE_POINTER 0x02

typedef struct neko_meta_property_type_info_t {
    const char* name;  // 用于显示名称
    u32 id;            // 用于查找和关联的 ID（最有可能通过枚举）
    u32 flags;         // 该字段需要取消引用的次数
    union {
        struct {
            u64 enum_id;
        } enum_info;

        struct {
            u32 key_id;
            u32 val_id;
        } container_info;
    } info;
} neko_meta_property_type_info_t;

// 默认元属性类型 ID
typedef enum neko_meta_property_type {
    NEKO_META_PROPERTY_TYPE_U8 = 0x00,
    NEKO_META_PROPERTY_TYPE_U16,
    NEKO_META_PROPERTY_TYPE_U32,
    NEKO_META_PROPERTY_TYPE_U64,
    NEKO_META_PROPERTY_TYPE_S8,
    NEKO_META_PROPERTY_TYPE_S16,
    NEKO_META_PROPERTY_TYPE_S32,
    NEKO_META_PROPERTY_TYPE_S64,
    NEKO_META_PROPERTY_TYPE_F32,
    NEKO_META_PROPERTY_TYPE_F64,
    NEKO_META_PROPERTY_TYPE_ENUM,
    NEKO_META_PROPERTY_TYPE_VEC2,
    NEKO_META_PROPERTY_TYPE_VEC3,
    NEKO_META_PROPERTY_TYPE_VEC4,
    NEKO_META_PROPERTY_TYPE_QUAT,
    NEKO_META_PROPERTY_TYPE_MAT3,
    NEKO_META_PROPERTY_TYPE_MAT4,
    NEKO_META_PROPERTY_TYPE_VQS,
    NEKO_META_PROPERTY_TYPE_UUID,
    NEKO_META_PROPERTY_TYPE_SIZE_T,  // 用于指针或 size_t 变量
    NEKO_META_PROPERTY_TYPE_STR,     // 用于 const char* / char*
    NEKO_META_PROPERTY_TYPE_COLOR,
    NEKO_META_PROPERTY_TYPE_OBJ,
    NEKO_META_PROPERTY_TYPE_COUNT
} neko_meta_property_type;

NEKO_API_PRIVATE neko_meta_property_type_info_t neko_meta_property_type_decl_impl(const char* name, u32 id);

#define neko_meta_property_type_decl(T, PROP_TYPE) neko_meta_property_type_decl_impl(NEKO_TO_STR(T), PROP_TYPE)

// 默认元属性类型信息定义
#define NEKO_META_PROPERTY_TYPE_INFO_U8 neko_meta_property_type_decl(uint8_t, NEKO_META_PROPERTY_TYPE_U8)
#define NEKO_META_PROPERTY_TYPE_INFO_S8 neko_meta_property_type_decl(int8_t, NEKO_META_PROPERTY_TYPE_S8)
#define NEKO_META_PROPERTY_TYPE_INFO_U16 neko_meta_property_type_decl(uint16_t, NEKO_META_PROPERTY_TYPE_U16)
#define NEKO_META_PROPERTY_TYPE_INFO_S16 neko_meta_property_type_decl(int16_t, NEKO_META_PROPERTY_TYPE_S16)
#define NEKO_META_PROPERTY_TYPE_INFO_U32 neko_meta_property_type_decl(uint32_t, NEKO_META_PROPERTY_TYPE_U32)
#define NEKO_META_PROPERTY_TYPE_INFO_S32 neko_meta_property_type_decl(int32_t, NEKO_META_PROPERTY_TYPE_S32)
#define NEKO_META_PROPERTY_TYPE_INFO_U64 neko_meta_property_type_decl(uint64_t, NEKO_META_PROPERTY_TYPE_U64)
#define NEKO_META_PROPERTY_TYPE_INFO_S64 neko_meta_property_type_decl(int64_t, NEKO_META_PROPERTY_TYPE_S64)
#define NEKO_META_PROPERTY_TYPE_INFO_F32 neko_meta_property_type_decl(float, NEKO_META_PROPERTY_TYPE_F32)
#define NEKO_META_PROPERTY_TYPE_INFO_F64 neko_meta_property_type_decl(double, NEKO_META_PROPERTY_TYPE_F64)
#define NEKO_META_PROPERTY_TYPE_INFO_ENUM neko_meta_property_type_decl(enum, NEKO_META_PROPERTY_TYPE_ENUM)
#define NEKO_META_PROPERTY_TYPE_INFO_VEC2 neko_meta_property_type_decl(neko_vec2, NEKO_META_PROPERTY_TYPE_VEC2)
#define NEKO_META_PROPERTY_TYPE_INFO_VEC3 neko_meta_property_type_decl(neko_vec3, NEKO_META_PROPERTY_TYPE_VEC3)
#define NEKO_META_PROPERTY_TYPE_INFO_VEC4 neko_meta_property_type_decl(neko_vec4, NEKO_META_PROPERTY_TYPE_VEC4)
#define NEKO_META_PROPERTY_TYPE_INFO_QUAT neko_meta_property_type_decl(neko_quat, NEKO_META_PROPERTY_TYPE_QUAT)
#define NEKO_META_PROPERTY_TYPE_INFO_MAT3 neko_meta_property_type_decl(neko_mat3, NEKO_META_PROPERTY_TYPE_MAT3)
#define NEKO_META_PROPERTY_TYPE_INFO_MAT4 neko_meta_property_type_decl(neko_mat4, NEKO_META_PROPERTY_TYPE_MAT4)
#define NEKO_META_PROPERTY_TYPE_INFO_VQS neko_meta_property_type_decl(neko_vqs, NEKO_META_PROPERTY_TYPE_VQS)
#define NEKO_META_PROPERTY_TYPE_INFO_UUID neko_meta_property_type_decl(neko_uuid_t, NEKO_META_PROPERTY_TYPE_UUID)
#define NEKO_META_PROPERTY_TYPE_INFO_SIZE_T neko_meta_property_type_decl(size_t, NEKO_META_PROPERTY_TYPE_SIZE_T)
#define NEKO_META_PROPERTY_TYPE_INFO_STR neko_meta_property_type_decl(char*, NEKO_META_PROPERTY_TYPE_STR)
#define NEKO_META_PROPERTY_TYPE_INFO_COLOR neko_meta_property_type_decl(neko_color_t, NEKO_META_PROPERTY_TYPE_COLOR)
#define NEKO_META_PROPERTY_TYPE_INFO_OBJ neko_meta_property_type_decl(object, NEKO_META_PROPERTY_TYPE_OBJ)

typedef struct neko_meta_property_t {
    const char* name;
    u32 offset;
    const char* type_name;
    size_t size;
    neko_meta_property_type_info_t type;
} neko_meta_property_t;

typedef struct neko_meta_enum_value_t {
    const char* name;
} neko_meta_enum_value_t;

NEKO_API_PRIVATE neko_meta_property_t neko_meta_property_impl(const char* field_type_name, const char* field, size_t sz, uint32_t offset, neko_meta_property_type_info_t type);

#define neko_meta_property(CLS, FIELD_TYPE, FIELD, TYPE) neko_meta_property_impl(NEKO_TO_STR(FIELD_TYPE), NEKO_TO_STR(FIELD), sizeof(FIELD_TYPE), NEKO_OFFSET(CLS, FIELD), TYPE)

typedef struct neko_meta_vtable_t {
    neko_hash_table(u64, void*) funcs;  // 哈希函数名称到函数指针
} neko_meta_vtable_t;

typedef struct neko_meta_class_t {
    neko_meta_property_t* properties;                          // Prop
    u32 property_count;                                        // 列表中的属性数量
    neko_hash_table(u64, neko_meta_property_t*) property_map;  // 哈希属性名称到指针的映射
    const char* name;                                          // 类名称
    u64 id;                                                    // 类 ID
    u64 base;                                                  // 父类 ID
    neko_meta_vtable_t vtable;                                 // 类的 VTable
    size_t size;                                               // 类的大小用于堆分配
} neko_meta_class_t;

typedef struct neko_meta_enum_t {
    neko_meta_enum_value_t* values;  // V
    u32 value_count;                 // 枚举值的计数
    const char* name;                // 枚举名称
    u64 id;                          // 枚举编号
} neko_meta_enum_t;

typedef struct neko_meta_registry_t {
    neko_hash_table(u64, neko_meta_class_t) classes;
    neko_hash_table(u64, neko_meta_enum_t) enums;
    void* user_data;
} neko_meta_registry_t;

typedef struct neko_meta_class_decl_t {
    const char* name;  // 类名称
    const char* base;  // 基父类名 将用于哈希 ID
    neko_meta_property_t* properties;
    size_t size;
    neko_meta_vtable_t* vtable;  // VTable
    size_t cls_size;             // 类的大小
} neko_meta_class_decl_t;

typedef struct neko_meta_enum_decl_t {
    neko_meta_enum_value_t* values;
    size_t size;
    const char* name;  // 类的显示名称
} neko_meta_enum_decl_t;

NEKO_API_DECL neko_meta_registry_t neko_meta_registry_new();
NEKO_API_DECL void neko_meta_registry_free(neko_meta_registry_t* meta);
NEKO_API_DECL const char* neko_meta_typestr(neko_meta_property_type type);
NEKO_API_DECL b32 neko_meta_has_base_class(const neko_meta_registry_t* meta, const neko_meta_class_t* cls);
NEKO_API_DECL u64 neko_meta_class_register(neko_meta_registry_t* meta, const neko_meta_class_decl_t decl);
NEKO_API_DECL void neko_meta_class_unregister(neko_meta_registry_t* meta, u64 id);
NEKO_API_DECL u64 neko_meta_enum_register(neko_meta_registry_t* meta, const neko_meta_enum_decl_t* decl);

#define neko_meta_class_get(META, T) (neko_hash_table_getp((META)->classes, neko_hash_str64(NEKO_TO_STR(T))))

#define neko_meta_class_get_w_name(META, NAME) (neko_hash_table_getp((META)->classes, neko_hash_str64(NAME)))

#define neko_meta_class_get_w_id(META, ID) (neko_hash_table_getp((META)->classes, (ID)))

#define neko_meta_class_exists(META, ID) (neko_hash_table_exists((META)->classes, ID))

#define neko_meta_getv(OBJ, T, PROP) (*((T*)((uint8_t*)(OBJ) + (PROP)->offset)))

#define neko_meta_getvp(OBJ, T, PROP) (((T*)((uint8_t*)(OBJ) + (PROP)->offset)))

#define neko_meta_setv(OBJ, T, PROP, VAL) (*((T*)((uint8_t*)(OBJ) + (PROP)->offset)) = VAL)

#define neko_meta_func_get(CLS, NAME) (neko_meta_func_get_internal(CLS, NEKO_TO_STR(NAME)));

#define neko_meta_func_get_w_id(META, ID, NAME) (neko_meta_func_get_internal_w_id(META, ID, NEKO_TO_STR(NAME)));

NEKO_API_DECL void* neko_meta_func_get_internal(const neko_meta_class_t* cls, const char* func_name);
NEKO_API_DECL void* neko_meta_func_get_internal_w_id(const neko_meta_registry_t* meta, u64 id, const char* func_name);

#endif