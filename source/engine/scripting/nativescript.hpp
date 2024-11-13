#pragma once

#include "base/common/base.hpp"
#include "engine/ecs/entity.h"

struct App;

typedef u32 (*native_script_get_instance_size_f)();
typedef void (*native_script_init_f)(App *scene, NativeEntity entity, void *);
typedef void (*native_script_update_f)(App *scene, NativeEntity entity, void *, double);
typedef void (*native_script_physics_update_f)(App *scene, NativeEntity entity, void *, double);
typedef void (*native_script_free_f)(App *scene, NativeEntity entity, void *);

typedef struct native_script_t {
    void *instance;

    char *get_instance_size_name;
    char *on_init_name;
    char *on_update_name;
    char *on_physics_update_name;
    char *on_free_name;

    native_script_init_f on_init;
    native_script_update_f on_update;
    native_script_physics_update_f on_physics_update;
    native_script_free_f on_free;

    NativeEntity entity;
} native_script_t;

typedef struct native_script_context_t {
    native_script_t *scripts;
    u32 script_count;
    u32 script_capacity;

    App *scene;

    void *handle;
} native_script_context_t;

native_script_context_t *native_new_script_context(App *scene, const char *assembly_path);
void native_init_scripts(native_script_context_t *context);
native_script_t *native_new_script(native_script_context_t *context, NativeEntity entity, const char *get_instance_size_name, const char *on_init_name, const char *on_update_name,
                                   const char *on_physics_update_name, const char *on_free_name, bool init_on_create);
void native_free_scripts(native_script_context_t *context);