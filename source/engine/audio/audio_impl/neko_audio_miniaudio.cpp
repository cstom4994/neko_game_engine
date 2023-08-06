#include "engine/audio/neko_audio.h"
#include "engine/base/neko_engine.h"

#define MINIAUDIO_IMPLEMENTATION
#include "libs/miniaudio/miniaudio.h"

typedef struct miniaudio_data_t {
    ma_context context;
    ma_device device;
    ma_device_config device_config;
    ma_mutex lock;
} miniaudio_data_t;

// typedef struct neko_audio_data_t
// {
//  // Other internal resource data, like audio resources
//  void* sample_out;               // Samples to actually write to hardware buffer
//     u32 sample_count_to_output;
//     u32 samples_per_second;

//     neko_slot_array(neko_audio_source_t)             sources;    // Raw source data
//     neko_slot_array(neko_audio_instance_data_t)  instance_cache;     // Instanced data

//     // Any internal data required for audio API
//     void* internal;

// } neko_audio_data_t;

void ma_audio_commit(ma_device* device, void* output, const void* input, ma_uint32 frame_count) {
    neko_audio_i* audio = neko_engine_instance()->ctx.audio;
    neko_audio_data_t* __data = (neko_audio_data_t*)audio->data;
    miniaudio_data_t* ma = (miniaudio_data_t*)__data->internal;

    memset(output, 0, frame_count * device->playback.channels * ma_get_bytes_per_sample(device->playback.format));

    // Only destroy 32 at a time
    u32 destroy_count = 0;
    neko_resource(neko_audio_instance_t) handles_to_destroy[32];

    ma_mutex_lock(&ma->lock);
    {
        for (neko_resource_cache_iter(neko_audio_instance_t) it = neko_resource_cache_iter_new(__data->instance_cache); neko_resource_cache_iter_valid(__data->instance_cache, it);
             neko_resource_cache_iter_advance(__data->instance_cache, it)) {
            neko_audio_instance_data_t* inst = neko_resource_cache_iter_get_ptr(it);

            // Get raw audio source from instance
            neko_audio_source_t* src = neko_resource_cache_get_ptr(__data->audio_cache, inst->src);

            // Easy out if the instance is not playing currently or the source is invalid
            if (!inst->playing || !src) {
                if (inst->persistent != true && destroy_count < 32) {
                    handles_to_destroy[destroy_count++] = neko_resource(neko_audio_instance_t){it.cur_idx};
                }
                continue;
            }

            s16* sample_out = (s16*)output;
            s16* samples = (s16*)src->samples;

            u64 samples_to_write = (u64)frame_count;
            f64 sample_volume = inst->volume;

            // Write to channels
            for (u64 write_sample = 0; write_sample < samples_to_write; ++write_sample) {
                s32 channels = src->channels;
                f64 start_sample_position = inst->sample_position;
                s16 start_left_sample;
                s16 start_right_sample;

                // Not sure about this line of code...
                f64 target_sample_position = start_sample_position + (f64)channels * (f64)1.f;

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
                        inst->sample_position = 0;
                    } else {
                        // Need to destroy the instance at this point...
                        inst->playing = false;
                        inst->sample_position = 0;

                        if (inst->persistent != true && destroy_count < 32) {
                            handles_to_destroy[destroy_count++] = neko_resource(neko_audio_instance_t){it.cur_idx};
                        }

                        break;
                    }
                }
            }
        }

        neko_for_range_i(destroy_count) { neko_resource_cache_erase(__data->instance_cache, handles_to_destroy[i]); }
    }
    ma_mutex_unlock(&ma->lock);
}

neko_result audio_init(neko_audio_i* audio) {
    // Construct instance of render data
    struct neko_audio_data_t* data = (neko_audio_data_t*)audio->data;
    data->internal = neko_malloc_init(struct miniaudio_data_t);
    miniaudio_data_t* output = (miniaudio_data_t*)data->internal;

    ma_result result = {0};

    // Init audio device
    // NOTE: Using the default device. Format is floating point because it simplifies mixing.
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = NULL;  // NULL for the default playback AUDIO.System.device.
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.capture.pDeviceID = NULL;  // NULL for the default capture AUDIO.System.device.
    config.capture.format = ma_format_s16;
    config.capture.channels = 1;
    config.sampleRate = 44100;
    config.dataCallback = ma_audio_commit;
    config.pUserData = NULL;

    output->device_config = config;

    if ((ma_device_init(NULL, &output->device_config, &output->device)) != MA_SUCCESS) {
        neko_assert(false);
    }

    if ((ma_device_start(&output->device)) != MA_SUCCESS) {
        neko_assert(false);
    }

    return neko_result_success;
}

neko_result audio_commit(neko_audio_i* audio) { return neko_result_success; }

neko_result audio_shutdown(neko_audio_i* audio) { return neko_result_success; }

neko_result audio_update(neko_audio_i* audio) { return neko_result_success; }

/*===================================================================
// Audio API Construction (fill out specific internal for custom API)
====================================================================*/

void neko_audio_construct_internal(struct neko_audio_i* audio) {
    // Audio Init/De-Init Functions
    audio->init = &audio_init;
    audio->shutdown = &audio_shutdown;
    audio->commit = &audio_commit;
    audio->update = &audio_update;
}
