
#include "engine/scripting/nativescript.hpp"

typedef struct test_script_t {
    float thing;
} test_script_t;

NEKO_EXPORT u32 get_test_script_instance_size() { return sizeof(test_script_t); }

NEKO_EXPORT void on_test_script_init(App* scene, NativeEntity entity, void* instance) { printf("Hello, world"); }

NEKO_EXPORT void on_test_script_update(App* scene, NativeEntity entity, void* instance, double timestep) {}

NEKO_EXPORT void on_test_script_free(App* scene, NativeEntity entity, void* instance) { printf("Test script free"); }