#ifndef NEKO_OBJECT_H
#define NEKO_OBJECT_H

#include "engine/common/neko_types.h"
#include "engine/common/neko_util.h"

// Intropection / Reflection keywords that should "compile away" but can be used for code generation
#define _introspect neko_empty_instruction(_introspect)
#define _non_serializable neko_empty_instruction(_non_serializable)
#define _immutable neko_empty_instruction(_immutable)
#define _read_only neko_empty_instruction(_immutable)
#define _ignore neko_empty_instruction(_ignore)
#define _default(...) neko_empty_instruction(_default)
#define _attributes(...) neko_empty_instruction(_attributes)
#define _ctor(...) neko_empty_instruction(_ctor)
#define _defaults(...) neko_empty_instruction(_defaults)
#define _serializes(...) neko_empty_instruction(_serialize)
#define _deserialize(...) neko_empty_instruction(_deserialize)
#define _components(...) neko_empty_instruction(_components)
#define _struct_default(type) type##_default

// Helper macro for typedefing a struture definition
#define neko_struct_def(name, ...) \
    typedef struct {               \
        __VA_ARGS__                \
    } name

// Definition for derived struct (based another parent struct)
#define neko_derive_def(name, parent, ...) neko_struct_def(name, parent _base; __VA_ARGS__)

#define neko_engine_check(statement) (statement) ? true : false

#define _base(base_type) base_type _base

/*============================================================
// Object Definition: neko_object
============================================================*/

// This could, instead, be a way to grab the meta class from the given reflected object
typedef struct neko_object {
    // Function pointer for finding the id for a particular object instance
    u32 (*type_id)();
} neko_object;

// Helper macro for specifically deriving some structure from an object struct
#define neko_object_def(name, ...) neko_derive_def(name, object, __VA_ARGS__)

#define neko_construct(type) *(type*)__neko_default_object_##type()

#define neko_construct_heap(type) (type*)__neko_default_object_##type##_heap()

neko_static_inline u32 __neko_type_id_impl(neko_object* obj) {
    neko_assert(obj->type_id != NULL);
    return (obj->type_id());
}

const char* neko_type_name_obj(neko_object* obj);

#define neko_type_name(obj) neko_type_name_obj(neko_cast(neko_object, obj))

#define neko_type_id(obj) __neko_type_id_impl(neko_cast(neko_object, obj))

#define neko_meta_class(obj) __neko_meta_class_impl(neko_cast(neko_object, obj))

#define neko_type_id_cls(cls) (u32) neko_meta_class_id_##cls

// #define neko_type_name_cls(cls)\
//  __neko_type_name_cls(neko_type_id_cls(cls))

#define neko_type_name_cls(cls) __neko_type_name_cls(cls)

void __neko_object_print_impl(neko_object* obj);

#define neko_object_print(obj) (__neko_object_print_impl(neko_cast(neko_object, (obj))))

#define neko_object_serialize(obj, buffer) neko_meta_class(neko_cast(neko_object, obj))->serialize_func(neko_cast(neko_object, obj), buffer)

#define neko_object_deserialize(obj, buffer) neko_meta_class(neko_cast(neko_object, obj))->deserialize_func(neko_cast(neko_object, obj), buffer)

#endif  // NEKO_OBJECT_H
