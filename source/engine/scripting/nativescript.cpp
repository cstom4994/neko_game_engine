#include "engine/scripting/nativescript.hpp"

#ifdef NEKO_IS_WIN32
#include <windows.h>

static void native_init_script_context_library(native_script_context_t *context, const char *assembly_path) {
    assert(context);

    context->handle = LoadLibraryA(assembly_path);
    if (!context->handle) {
        console_log("Failed to load script assembly: `%s'", assembly_path);
    }
}

static void *native_get_script_proc(native_script_context_t *context, const char *name) {
    assert(context);

    void *function = GetProcAddress((HMODULE)context->handle, name);
    if (!function) {
        console_log("Failed to locate function `%s'", name);
    }

    return function;
}

static void native_deinit_script_context_library(native_script_context_t *context) {
    assert(context);

    FreeLibrary((HMODULE)context->handle);
}

#else

#endif

#if 1

#define native_grow_capacity(c_) ((c_) < 8 ? 8 : (c_) * 2)

char *native_copy_string(const char *string) {
    assert(string);

    const u32 len = (u32)strlen(string);

    char *result = (char *)mem_alloc(len + 1);

    strcpy(result, string);
    result[len] = '\0';

    return result;
}

native_script_context_t *native_new_script_context(App *scene, const char *assembly_path) {
    assert(scene);

    native_script_context_t *new_ctx = (native_script_context_t *)mem_alloc(sizeof(native_script_context_t));

    new_ctx->scripts = NULL;
    new_ctx->script_count = 0;
    new_ctx->script_capacity = 0;

    new_ctx->scene = scene;

    native_init_script_context_library(new_ctx, assembly_path);

    return new_ctx;
}

void native_free_script_context(native_script_context_t *context) {
    assert(context);

    native_deinit_script_context_library(context);

    if (context->script_capacity > 0) {
        mem_free(context->scripts);
    }

    mem_free(context);
}

native_script_t *native_new_script(native_script_context_t *context, NativeEntity entity, const char *get_instance_size_name, const char *on_init_name, const char *on_update_name,
                                   const char *on_physics_update_name, const char *on_free_name, bool init_on_create) {
    assert(context);

    if (context->script_count >= context->script_capacity) {
        context->script_capacity = native_grow_capacity(context->script_capacity);
        context->scripts = (native_script_t *)mem_realloc(context->scripts, context->script_capacity * sizeof(native_script_t));
    }

    native_script_t *new_ctx = &context->scripts[context->script_count++];

    new_ctx->instance = NULL;

    new_ctx->entity = entity;

    // NativeEntity *entity_ptr = native_get_entity_ptr(context->scene, entity);
    // entity_ptr->script = new_ctx;

    new_ctx->get_instance_size_name = NULL;
    new_ctx->on_init_name = NULL;
    new_ctx->on_update_name = NULL;
    new_ctx->on_physics_update_name = NULL;
    new_ctx->on_free_name = NULL;

    new_ctx->on_init = NULL;
    new_ctx->on_update = NULL;
    new_ctx->on_physics_update = NULL;
    new_ctx->on_free = NULL;

    native_script_get_instance_size_f get_size = NULL;
    if (get_instance_size_name) {
        new_ctx->get_instance_size_name = native_copy_string(get_instance_size_name);

        get_size = (native_script_get_instance_size_f)native_get_script_proc(context, get_instance_size_name);
    }

    if (on_init_name) {
        new_ctx->on_init_name = native_copy_string(on_init_name);
        new_ctx->on_init = (native_script_init_f)native_get_script_proc(context, on_init_name);
    }

    if (on_update_name) {
        new_ctx->on_update_name = native_copy_string(on_update_name);
        new_ctx->on_update = (native_script_update_f)native_get_script_proc(context, on_update_name);
    }

    if (on_physics_update_name) {
        new_ctx->on_physics_update_name = native_copy_string(on_physics_update_name);
        new_ctx->on_physics_update = (native_script_physics_update_f)native_get_script_proc(context, on_physics_update_name);
    }

    if (on_free_name) {
        new_ctx->on_free_name = native_copy_string(on_free_name);
        new_ctx->on_free = (native_script_free_f)native_get_script_proc(context, on_free_name);
    }

    if (get_size) {
        new_ctx->instance = mem_alloc(get_size());
    }

    if (init_on_create && new_ctx->on_init) {
        new_ctx->on_init(context->scene, entity, new_ctx->instance);
    }

    return new_ctx;
}

void native_deinit_script(native_script_context_t *context, native_script_t *script);

void native_delete_script(native_script_context_t *context, native_script_t *script) {
    assert(context);
    assert(script);

    i32 index = -1;

    for (u32 i = 0; i < context->script_count; i++) {
        if (&context->scripts[i] == script) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    native_deinit_script(context, &context->scripts[index]);

    for (u32 i = index; i < context->script_count - 1; i++) {
        context->scripts[i] = context->scripts[i + 1];
    }

    context->script_count--;
}

void native_deinit_script(native_script_context_t *context, native_script_t *script) {
    assert(context);
    assert(script);

    if (script->on_free) {
        script->on_free(context->scene, script->entity, script->instance);
    }

    if (script->instance) {
        mem_free(script->instance);
    }

    // NativeEntity *entity_ptr = native_get_entity_ptr(context->scene, script->entity);
    // entity_ptr->script = NULL;

    mem_free(script->get_instance_size_name);
    mem_free(script->on_init_name);
    mem_free(script->on_update_name);
    mem_free(script->on_free_name);
    mem_free(script->on_physics_update_name);
}

void native_init_scripts(native_script_context_t *context) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        native_script_t *script = &context->scripts[i];
        if (script->on_init) {
            script->on_init(context->scene, script->entity, script->instance);
        }
    }
}

void native_update_scripts(native_script_context_t *context, double timestep) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        native_script_t *script = &context->scripts[i];
        if (script->on_update) {
            script->on_update(context->scene, script->entity, script->instance, timestep);
        }
    }
}

void native_physics_update_scripts(native_script_context_t *context, double timestep) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        native_script_t *script = &context->scripts[i];
        if (script->on_physics_update) {
            script->on_physics_update(context->scene, script->entity, script->instance, timestep);
        }
    }
}

void native_free_scripts(native_script_context_t *context) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        native_script_t *script = &context->scripts[i];
        native_deinit_script(context, script);
    }

    mem_free(context->scripts);
    mem_free(context);
}

#endif