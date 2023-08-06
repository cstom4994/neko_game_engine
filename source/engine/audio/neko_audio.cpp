#include "engine/audio/neko_audio.h"

#include "engine/base/neko_engine.h"
#include "engine/platform/neko_platform.h"

// 3rd
#include "libs/dr_libs/dr_mp3.h"
#include "libs/dr_libs/dr_wav.h"
#include "libs/stb/stb_vorbis.c"

// typedef struct neko_audio_source_t
// {
//  s32 channels;
//  s32 sample_rate;
//  void* samples;
//  s32 sample_count;
// } neko_audio_source_t;

b32 __neko_load_ogg_data(const char* file_name, neko_audio_source_t* src) {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Init OGG
    {
        src->sample_count = stb_vorbis_decode_filename(file_name, &src->channels, &src->sample_rate, (s16**)&src->samples);

        if (!src->samples || src->sample_count == -1) {
            src->samples = NULL;
            neko_println("WARNING: Could not load .ogg file: %s", file_name);
            return false;
        }

        src->sample_count *= src->channels;

        return true;
    }

    return false;
}

b32 __neko_load_wav_data(const char* file_name, neko_audio_source_t* src) {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    u64 total_pcm_frame_count = 0;
    src->samples = drwav_open_file_and_read_pcm_frames_s16(file_name, (u32*)&src->channels, (u32*)&src->sample_rate, &total_pcm_frame_count, NULL);

    if (!src->samples) {
        src->samples = NULL;
        neko_println("WARNING: Could not load .ogg file: %s", file_name);
        return false;
    }

    src->sample_count = total_pcm_frame_count * src->channels;

    return true;
}

b32 __neko_load_mp3_data(const char* file_name, neko_audio_source_t* src) {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Decode entire mp3
    u64 total_pcm_frame_count = 0;
    drmp3_config cfg = {0};
    src->samples = drmp3_open_file_and_read_pcm_frames_s16(file_name, &cfg, &total_pcm_frame_count, NULL);

    if (!src->samples) {
        src->samples = NULL;
        neko_println("WARNING: Could not load .ogg file: %s", file_name);
        return false;
    }

    src->channels = cfg.channels;
    src->sample_rate = cfg.sampleRate;
    src->sample_count = total_pcm_frame_count * src->channels;

    return true;
}

// Default audio upate function (taken from Ryan Fluery's win32 app template)
neko_result __neko_audio_update_internal(struct neko_audio_i* audio) {
    // For the update, we're going to iterate through all contiguous instance data, check whether or not it's playing,
    // Then we'll just play the shiz
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;

    if (!__data) {
        return neko_result_incomplete;
    }

    for (neko_resource_cache_iter(neko_audio_instance_t) it = neko_resource_cache_iter_new(__data->instance_cache); neko_resource_cache_iter_valid(__data->instance_cache, it);
         neko_resource_cache_iter_advance(__data->instance_cache, it)) {
        neko_audio_instance_data_t* inst = neko_resource_cache_iter_get_ptr(it);

        // Get raw audio source from instance
        neko_audio_source_t* src = neko_resource_cache_get_ptr(__data->audio_cache, inst->src);

        // Easy out if the instance is not playing currently or the source is invalid
        if (!inst->playing || !src) {
            continue;
        }

        s16* sample_out = (s16*)__data->sample_out;
        s16* samples = (s16*)src->samples;

        u64 samples_to_write = __data->sample_count_to_output;
        f64 sample_volume = inst->volume;

        // Write to channels
        for (u64 write_sample = 0; write_sample < samples_to_write; ++write_sample) {
            s32 channels = src->channels;
            f64 start_sample_position = inst->sample_position;
            s16 start_left_sample;
            s16 start_right_sample;

            // Not sure about this line of code...
            f64 target_sample_position = start_sample_position + (f64)channels * (f64)1.f;  // pitch

            if (target_sample_position >= src->sample_count) {
                target_sample_position -= src->sample_count;
            }

            s16 target_left_sample;
            s16 target_right_sample;

            {
                u64 left_idx = (u64)start_sample_position;
                if (channels > 1) {
                    left_idx &= ~((u64)(0x01));
                }
                u64 right_idx = left_idx + (channels - 1);

                s16 first_left_sample = samples[left_idx];
                s16 first_right_sample = samples[right_idx];
                s16 second_left_sample = samples[left_idx + channels];
                s16 second_right_sample = samples[right_idx + channels];

                start_left_sample = (s16)(first_left_sample + (second_left_sample - first_left_sample) * (start_sample_position / channels - (u64)(start_sample_position / channels)));
                start_right_sample = (s16)(first_right_sample + (second_right_sample - first_right_sample) * (start_sample_position / channels - (u64)(start_sample_position / channels)));
            }

            {
                u64 left_index = (u64)target_sample_position;
                if (channels > 1) {
                    left_index &= ~((u64)(0x01));
                }
                u64 right_index = left_index + (channels - 1);

                s16 first_left_sample = samples[left_index];
                s16 first_right_sample = samples[right_index];
                s16 second_left_sample = samples[left_index + channels];
                s16 second_right_sample = samples[right_index + channels];

                target_left_sample = (s16)(first_left_sample + (second_left_sample - first_left_sample) * (target_sample_position / channels - (u64)(target_sample_position / channels)));
                target_right_sample = (s16)(first_right_sample + (second_right_sample - first_right_sample) * (target_sample_position / channels - (u64)(target_sample_position / channels)));
            }

            s16 left_sample = (s16)((((s64)start_left_sample + (s64)target_left_sample) / 2) * sample_volume);
            s16 right_sample = (s16)((((s64)start_right_sample + (s64)target_right_sample) / 2) * sample_volume);

            *sample_out++ += left_sample;   // Left
            *sample_out++ += right_sample;  // Right

            // Possibly need fixed sampling instead
            inst->sample_position = target_sample_position;

            // Loop sound if necessary
            if (inst->sample_position >= src->sample_count - channels - 1) {
                if (inst->loop) {
                    // inst->sample_position -= src->sample_count;
                    inst->sample_position = 0;
                } else {
                    // Need to destroy the instance at this point...
                    inst->playing = false;
                    inst->sample_position = 0;
                    break;
                }
            }
        }
    }

    return neko_result_success;
}

/*============================================================
// Audio Source
============================================================*/

// This will allocate memory for user.
neko_resource(neko_audio_source_t) __neko_load_audio_source_from_file(const char* file_name) {
    neko_audio_source_t source = neko_default_val();
    b32 load_successful = false;

    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;

    if (!platform->file_exists(file_name)) {
        neko_println("WARNING: Could not open file: %s", file_name);
        return neko_resource_invalid(neko_audio_source_t);
    }

    // Lower case and grab file ext.
    char file_ext[64] = {0};
    neko_util_str_to_lower(file_name, file_ext, sizeof(file_ext));
    platform->file_extension(file_ext, sizeof(file_ext), file_ext);

    // Load OGG data
    if (neko_string_compare_equal(file_ext, "ogg")) {
        load_successful = __neko_load_ogg_data(file_name, &source);
    }

    if (neko_string_compare_equal(file_ext, "wav")) {
        load_successful = __neko_load_wav_data(file_name, &source);
    }

    if (neko_string_compare_equal(file_ext, "mp3")) {
        load_successful = __neko_load_mp3_data(file_name, &source);
    }

    // Load raw source into memory and return handle id
    if (load_successful) {
        neko_println("SUCCESS: Audio source loaded: %s", file_name);

        // Add to resource cache
        return neko_resource_cache_insert(__data->audio_cache, source);
    } else {
        neko_println("WARNING: Could not load audio source data: %s", file_name);
        return neko_resource_invalid(neko_audio_source_t);
    }

    return neko_resource_invalid(neko_audio_source_t);
}

/*============================================================
// Audio Instance Data
============================================================*/

neko_resource(neko_audio_instance_t) __neko_audio_construct_instance(neko_audio_instance_data_t inst) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_resource(neko_audio_instance_t) handle = neko_resource_cache_insert(__data->instance_cache, inst);
    return handle;
}

void __neko_audio_play_source(neko_resource(neko_audio_source_t) src, f32 volume) {
    // Construct instance data from source
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_instance_data_t inst = neko_audio_instance_data_new(src);
    inst.volume = neko_clamp(volume, audio->min_audio_volume, audio->max_audio_volume);
    inst.persistent = false;
    neko_resource(neko_audio_instance_t) inst_h = audio->construct_instance(inst);
    audio->play(inst_h);
}

// Essentially just restarts the audio instance
void __neko_audio_play(neko_resource(neko_audio_instance_t) inst_h) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    if (inst) {
        inst->playing = true;
    }
}

void __neko_audio_pause(neko_resource(neko_audio_instance_t) inst_h) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    if (inst) {
        inst->playing = false;
    }
}

void __neko_audio_restart(neko_resource(neko_audio_instance_t) inst_h) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    if (inst) {
        inst->playing = true;
        inst->sample_position = 0;
    }
}

void __neko_audio_set_instance_data(neko_resource(neko_audio_instance_t) inst_h, neko_audio_instance_data_t data) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    if (inst) {
        *inst = data;
    }
}

neko_audio_instance_data_t __neko_audio_get_instance_data(neko_resource(neko_audio_instance_t) inst_h) {
    neko_audio_instance_data_t data = neko_default_val();
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    if (inst) {
        data = *inst;
    }
    return data;
}

f32 __neko_audio_get_volume(neko_resource(neko_audio_instance_t) inst_h) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    if (inst) {
        return inst->volume;
    }

    return 0.f;
}

void __neko_audio_set_volume(neko_resource(neko_audio_instance_t) inst_h, f32 vol) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    if (inst) {
        inst->volume = neko_clamp(vol, audio->min_audio_volume, audio->max_audio_volume);
    }
}

void __neko_audio_stop(neko_resource(neko_audio_instance_t) inst_h) {
    // Should actually destroy a sound if it's not persistent
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    if (inst) {
        inst->playing = false;
        inst->sample_position = 0;
    }
}

b32 __neko_audio_is_playing(neko_resource(neko_audio_instance_t) inst_h) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_instance_data_t* inst = neko_resource_cache_get_ptr(__data->instance_cache, inst_h);
    return (inst && inst->playing);
}

void __neko_audio_get_runtime(neko_resource(neko_audio_source_t) src_h, s32* _minutes, s32* _seconds) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_source_t* src = neko_resource_cache_get_ptr(__data->audio_cache, src_h);
    if (src) {
        // Calculate total length in seconds
        f64 total_seconds = ((f32)src->sample_count / (f32)src->sample_rate) / src->channels;
        s32 seconds = (s32)(fmodf(total_seconds, 60.f));
        s32 minutes = (s32)(total_seconds / 60.f);

        if (_minutes) {
            *_minutes = minutes;
        }

        if (_seconds) {
            *_seconds = seconds;
        }
    }
}

s32 __neko_get_sample_count(neko_resource(neko_audio_source_t) src) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_source_t* sp = neko_resource_cache_get_ptr(__data->audio_cache, src);
    if (sp) {
        return sp->sample_count;
    }
    return 0;
}

s32 __neko_get_sample_rate(neko_resource(neko_audio_source_t) src) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_source_t* sp = neko_resource_cache_get_ptr(__data->audio_cache, src);
    if (sp) {
        return sp->sample_rate;
    }
    return 0;
}

s32 __neko_get_num_channels(neko_resource(neko_audio_source_t) src) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    neko_audio_source_t* sp = neko_resource_cache_get_ptr(__data->audio_cache, src);
    if (sp) {
        return sp->channels;
    }
    return 0;
}

void __neko_audio_convert_to_runtime(s32 sample_count, s32 sample_rate, s32 num_channels, s32 position, s32* minutes_out, s32* seconds_out) {
    // Calculate total length in seconds
    f64 frac = (f64)position / (f64)sample_count;
    f64 total_seconds = ((f64)sample_count / (f64)sample_rate) / num_channels;
    total_seconds = total_seconds * frac;
    s32 seconds = (s32)(fmodf(total_seconds, 60.f));
    s32 minutes = (s32)(total_seconds / 60.f);

    if (minutes_out) {
        *minutes_out = minutes;
    }

    if (seconds_out) {
        *seconds_out = seconds;
    }
}

struct neko_audio_i* __neko_audio_construct() {
    neko_malloc_init_ex(audio, neko_audio_i);
    neko_malloc_init_ex(data, neko_audio_data_t);

    data->internal = NULL;
    data->instance_cache = neko_resource_cache_new(neko_audio_instance_t);
    data->audio_cache = neko_resource_cache_new(neko_audio_source_t);

    // Set data
    audio->user_data = NULL;
    audio->data = data;

    // Default audio max/min
    audio->max_audio_volume = 1.f;
    audio->min_audio_volume = 0.f;

    // Default internals
    __neko_audio_set_default_functions(audio);

    return audio;
}

void __neko_audio_set_default_functions(struct neko_audio_i* audio) {
    audio->update = &__neko_audio_update_internal;
    audio->load_audio_source_from_file = &__neko_load_audio_source_from_file;
    audio->construct_instance = &__neko_audio_construct_instance;
    audio->play = &__neko_audio_play;
    audio->play_source = &__neko_audio_play_source;
    audio->pause = &__neko_audio_pause;
    audio->restart = &__neko_audio_restart;
    audio->stop = &__neko_audio_stop;
    audio->get_volume = &__neko_audio_get_volume;
    audio->set_volume = &__neko_audio_set_volume;
    audio->is_playing = &__neko_audio_is_playing;
    audio->set_instance_data = &__neko_audio_set_instance_data;
    audio->get_instance_data = &__neko_audio_get_instance_data;
    audio->get_runtime = &__neko_audio_get_runtime;
    audio->convert_to_runtime = &__neko_audio_convert_to_runtime;
    audio->get_sample_count = &__neko_get_sample_count;
    audio->get_sample_rate = &__neko_get_sample_rate;
    audio->get_num_channels = &__neko_get_num_channels;

    neko_audio_construct_internal(audio);
}
