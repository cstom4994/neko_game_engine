#ifndef NEKO_AUDIO_H
#define NEKO_AUDIO_H

#include "engine/common/neko_containers.h"
#include "engine/common/neko_types.h"

typedef enum neko_audio_file_type { neko_ogg = 0x00, neko_wav } neko_audio_file_type;

/*==================
// Audio Source
==================*/

typedef struct neko_audio_source_t {
    s32 channels;
    s32 sample_rate;
    void* samples;
    s32 sample_count;
} neko_audio_source_t;

neko_resource_cache_decl(neko_audio_source_t);

/*=====================
// Audio Instance Data
=====================*/

typedef struct neko_audio_instance_data_t {
    neko_resource(neko_audio_source_t) src;
    f32 volume;
    bool loop;
    bool persistent;
    bool playing;
    f64 sample_position;
    void* user_data;
} neko_audio_instance_data_t;

typedef struct neko_audio_instance_data_t neko_audio_instance_t;

neko_resource_cache_decl(neko_audio_instance_data_t);
neko_resource_cache_decl(neko_audio_instance_t);

/*=====================
// Internal Audio Data
=====================*/

typedef struct neko_audio_data_t {
    // Samples to actually write to hardware
    void* sample_out;

    // Amount of samples to write
    u32 sample_count_to_output;

    // Samples per second for hardward
    u32 samples_per_second;

    // All the registered instances to actually play
    neko_resource_cache(neko_audio_instance_t) instance_cache;

    // All registered sources
    neko_resource_cache(neko_audio_source_t) audio_cache;

    // Any internal data required for audio API
    void* internal;

} neko_audio_data_t;

neko_force_inline neko_audio_instance_data_t neko_audio_instance_data_new(neko_resource(neko_audio_source_t) src) {
    neko_audio_instance_data_t inst = {0};
    inst.src = src;
    inst.volume = 0.5f;
    inst.loop = false;
    inst.persistent = false;
    inst.playing = true;
    inst.user_data = NULL;
    return inst;
}

typedef struct neko_audio_i {
    /*============================================================
    // Audio Initilization / De-Initialization
    ============================================================*/

    neko_result (*init)(struct neko_audio_i*);
    neko_result (*shutdown)(struct neko_audio_i*);
    neko_result (*update)(struct neko_audio_i*);
    neko_result (*commit)(struct neko_audio_i*);

    /*============================================================
    // Audio Source
    ============================================================*/

    neko_resource(neko_audio_source_t) (*load_audio_source_from_file)(const char* file_name);

    /*============================================================
    // Audio Instance Data
    ============================================================*/

    neko_resource(neko_audio_instance_t) (*construct_instance)(neko_audio_instance_data_t);
    void (*play_source)(neko_resource(neko_audio_source_t) src, f32 volume);
    void (*play)(neko_resource(neko_audio_instance_t));
    void (*pause)(neko_resource(neko_audio_instance_t));
    void (*stop)(neko_resource(neko_audio_instance_t));
    void (*restart)(neko_resource(neko_audio_instance_t));
    b32 (*is_playing)(neko_resource(neko_audio_instance_t));

    void (*set_instance_data)(neko_resource(neko_audio_instance_t), neko_audio_instance_data_t);
    neko_audio_instance_data_t (*get_instance_data)(neko_resource(neko_audio_instance_t));
    f32 (*get_volume)(neko_resource(neko_audio_instance_t));
    void (*set_volume)(neko_resource(neko_audio_instance_t), f32);

    void (*get_runtime)(neko_resource(neko_audio_source_t) src, s32* minutes, s32* seconds);
    void (*convert_to_runtime)(s32 sample_count, s32 sample_rate, s32 num_channels, s32 position, s32* minutes_out, s32* seconds_out);

    s32 (*get_sample_count)(neko_resource(neko_audio_source_t) src);
    s32 (*get_sample_rate)(neko_resource(neko_audio_source_t) src);
    s32 (*get_num_channels)(neko_resource(neko_audio_source_t) src);

    // Proably
    f32 max_audio_volume;
    f32 min_audio_volume;

    // All internal API specific data for audio system
    void* data;

    // Any custom user data (for custom API implementations)
    void* user_data;
} neko_audio_i;

// Extern internal functions
extern struct neko_audio_i* __neko_audio_construct();
extern void neko_audio_construct_internal(struct neko_audio_i* audio);
extern void __neko_audio_set_default_functions(struct neko_audio_i* audio);
extern neko_result __neko_audio_update_internal(struct neko_audio_i* audio);
extern void __neko_audio_play_source(neko_resource(neko_audio_source_t) src, f32 volume);
extern void __neko_audio_play(neko_resource(neko_audio_instance_t) inst_h);
extern void __neko_audio_pause(neko_resource(neko_audio_instance_t) inst_h);
extern void __neko_audio_resume(neko_resource(neko_audio_instance_t) inst_h);
extern void __neko_audio_restart(neko_resource(neko_audio_instance_t) inst_h);
extern void __neko_audio_set_instance_data(neko_resource(neko_audio_instance_t) inst_h, neko_audio_instance_data_t data);
extern f32 __neko_audio_get_volume(neko_resource(neko_audio_instance_t) inst_h);
extern void __neko_audio_set_volume(neko_resource(neko_audio_instance_t) inst_h, f32 vol);
extern void __neko_audio_stop(neko_resource(neko_audio_instance_t) inst_h);

#endif  // NEKO_AUDIO_H