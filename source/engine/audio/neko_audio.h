#ifndef NEKO_AUDIO_H
#define NEKO_AUDIO_H

#include "engine/common/neko_containers.h"
#include "engine/common/neko_types.h"
#include "libs/cute/cute_sound.h"

/*=====================
// Internal Audio Data
=====================*/

typedef struct neko_audio_data_t {

    // Any internal data required for audio API
    void* internal;

} neko_audio_data_t;

typedef struct neko_audio_i {
    /*============================================================
    // Audio Initilization / De-Initialization
    ============================================================*/

    neko_result (*init)(struct neko_audio_i*);
    neko_result (*shutdown)(struct neko_audio_i*);
    neko_result (*update)(struct neko_audio_i*);
    neko_result (*commit)(struct neko_audio_i*);

    /*============================================================
    // Audio Instance Data
    ============================================================*/

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

#endif  // NEKO_AUDIO_H