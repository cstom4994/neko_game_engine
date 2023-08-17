#ifndef NEKO_PLATFORM_H
#define NEKO_PLATFORM_H

#include "engine/common/neko_containers.h"
#include "engine/common/neko_str.h"
#include "engine/common/neko_types.h"
#include "engine/common/neko_util.h"
#include "engine/math/neko_math.h"
#include "engine/scripting/neko_lua_base.h"

#if (defined __APPLE__ || defined _APPLE)

#define NEKO_PLATFORM_APPLE

#elif (defined _WIN32 || defined _WIN64)

#define NEKO_PLATFORM_WIN
#include <windows.h>

#define WIN32_LEAN_AND_MEAN

#elif (defined linux || defined _linux || defined __linux__)

#define NEKO_PLATFORM_LINUX

#endif

// Forward Decl.
struct neko_uuid;
struct neko_platform_input;
struct neko_platform_window;

/*============================================================
// Platform Time
============================================================*/

typedef struct neko_platform_time {
    f64 max_fps;
    f64 current;
    f64 previous;
    f64 update;
    f64 render;
    f64 delta;
    f64 frame;
} neko_platform_time;

/*============================================================
// Platform UUID
============================================================*/

#define neko_uuid_str_size_constant 32

// 33 characters, all set to 0
#define neko_uuid_temp_str_buffer() \
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

typedef struct neko_uuid {
    const char *id;
    u8 bytes[16];
} neko_uuid;

// /*============================================================
// // Platform Window
// ============================================================*/

neko_enum_decl(neko_window_flags, resizable = 1 << 1, fullscreen = 1 << 2, highdpi = 1 << 3);
neko_enum_flag_operator(neko_window_flags);

namespace neko {

// HACK: 我知道这很丑 但是 neko_window_flags 在这里
template <>
neko_inline neko_window_flags neko_lua_to<neko_window_flags>(lua_State *L, int index) {
    luaL_argcheck(L, lua_isnumber(L, index), index, "neko_window_flags expected");
    return static_cast<neko_window_flags>(lua_tointeger(L, index));
}

}  // namespace neko

// // Forward Decl
struct neko_platform_window;
typedef void *neko_platform_window_ptr;

// Internal handle for windows
typedef u32 neko_resource_handle;

// Declare slot array
neko_slot_array_decl(neko_platform_window_ptr);

typedef enum neko_platform_cursor {
    neko_platform_cursor_arrow,
    neko_platform_cursor_ibeam,
    neko_platform_cursor_size_nw_se,
    neko_platform_cursor_size_ne_sw,
    neko_platform_cursor_size_ns,
    neko_platform_cursor_size_we,
    neko_platform_cursor_size_all,
    neko_platform_cursor_hand,
    neko_platform_cursor_no,
    neko_platform_cursor_count
} neko_platform_cursor;

// /*============================================================
// // Platform Input
// ============================================================*/

typedef enum neko_platform_keycode {
    neko_keycode_a,
    neko_keycode_b,
    neko_keycode_c,
    neko_keycode_d,
    neko_keycode_e,
    neko_keycode_f,
    neko_keycode_g,
    neko_keycode_h,
    neko_keycode_i,
    neko_keycode_j,
    neko_keycode_k,
    neko_keycode_l,
    neko_keycode_m,
    neko_keycode_n,
    neko_keycode_o,
    neko_keycode_p,
    neko_keycode_q,
    neko_keycode_r,
    neko_keycode_s,
    neko_keycode_t,
    neko_keycode_u,
    neko_keycode_v,
    neko_keycode_w,
    neko_keycode_x,
    neko_keycode_y,
    neko_keycode_z,
    neko_keycode_lshift,
    neko_keycode_rshift,
    neko_keycode_lalt,
    neko_keycode_ralt,
    neko_keycode_lctrl,
    neko_keycode_rctrl,
    neko_keycode_bspace,
    neko_keycode_bslash,
    neko_keycode_qmark,
    neko_keycode_tilde,
    neko_keycode_comma,
    neko_keycode_period,
    neko_keycode_esc,
    neko_keycode_space,
    neko_keycode_left,
    neko_keycode_up,
    neko_keycode_right,
    neko_keycode_down,
    neko_keycode_zero,
    neko_keycode_one,
    neko_keycode_two,
    neko_keycode_three,
    neko_keycode_four,
    neko_keycode_five,
    neko_keycode_six,
    neko_keycode_seven,
    neko_keycode_eight,
    neko_keycode_nine,
    neko_keycode_npzero,
    neko_keycode_npone,
    neko_keycode_nptwo,
    neko_keycode_npthree,
    neko_keycode_npfour,
    neko_keycode_npfive,
    neko_keycode_npsix,
    neko_keycode_npseven,
    neko_keycode_npeight,
    neko_keycode_npnine,
    neko_keycode_caps,
    neko_keycode_delete,
    neko_keycode_end,
    neko_keycode_f1,
    neko_keycode_f2,
    neko_keycode_f3,
    neko_keycode_f4,
    neko_keycode_f5,
    neko_keycode_f6,
    neko_keycode_f7,
    neko_keycode_f8,
    neko_keycode_f9,
    neko_keycode_f10,
    neko_keycode_f11,
    neko_keycode_f12,
    neko_keycode_home,
    neko_keycode_plus,
    neko_keycode_minus,
    neko_keycode_lbracket,
    neko_keycode_rbracket,
    neko_keycode_semi_colon,
    neko_keycode_enter,
    neko_keycode_insert,
    neko_keycode_pgup,
    neko_keycode_pgdown,
    neko_keycode_numlock,
    neko_keycode_tab,
    neko_keycode_npmult,
    neko_keycode_npdiv,
    neko_keycode_npplus,
    neko_keycode_npminus,
    neko_keycode_npenter,
    neko_keycode_npdel,
    neko_keycode_mute,
    neko_keycode_volup,
    neko_keycode_voldown,
    neko_keycode_pause,
    neko_keycode_print,
    neko_keycode_count
} neko_platform_keycode;

typedef struct neko_platform_meminfo {
    u64 virtual_memory_used;
    u64 physical_memory_used;
    u64 peak_virtual_memory_used;
    u64 peak_physical_memory_used;
} neko_platform_meminfo;

typedef enum neko_platform_mouse_button_code { neko_mouse_lbutton, neko_mouse_rbutton, neko_mouse_mbutton, neko_mouse_button_code_count } neko_platform_mouse_button_code;

typedef struct neko_platform_mouse {
    bool button_map[neko_mouse_button_code_count];
    bool prev_button_map[neko_mouse_button_code_count];
    neko_vec2 position;
    neko_vec2 prev_position;
    neko_vec2 wheel;
    bool moved_this_frame;
} neko_platform_mouse;

typedef struct neko_platform_input {
    bool key_map[neko_keycode_count];
    bool prev_key_map[neko_keycode_count];
    neko_platform_mouse mouse;
} neko_platform_input;

typedef struct neko_platform_ctx {
    char *gamepath;
} neko_platform_ctx;

/*===============================================================================================
// Platform API Struct
===============================================================================================*/

// Enumeration of all platform type
typedef enum neko_platform_type { neko_platform_type_unknown = 0, neko_platform_type_windows, neko_platform_type_linux, neko_platform_type_mac } neko_platform_type;

typedef enum neko_platform_video_driver_type {
    neko_platform_video_driver_type_none = 0,
    neko_platform_video_driver_type_opengl,
} neko_platform_video_driver_type;

typedef enum neko_opengl_compatibility_flags {
    neko_opengl_compatibility_flaneko_legacy = 0,
    neko_opengl_compatibility_flaneko_core = 1 << 1,
    neko_opengl_compatibility_flaneko_compatibility = 1 << 2,
    neko_opengl_compatibility_flaneko_forward = 1 << 3,
    neko_opengl_compatibility_flaneko_es = 1 << 4,
} neko_opengl_compatibility_flags;

// A structure that contains OpenGL video settings
typedef struct neko_opengl_video_settings {
    neko_opengl_compatibility_flags compability_flags;
    u32 major_version;
    u32 minor_version;
    u8 multi_sampling_count;
    void *ctx;
} neko_opengl_video_settings;

typedef union neko_graphics_api_settings {
    neko_opengl_video_settings opengl;
    s32 dummy;
} neko_graphics_api_settings;

typedef struct neko_platform_video_settings {
    neko_graphics_api_settings graphics;
    neko_platform_video_driver_type driver;
    u32 vsync_enabled;
} neko_platform_video_settings;

typedef struct neko_platform_settings {
    neko_platform_video_settings video;
} neko_platform_settings;

typedef void (*dropped_files_callback_t)(void *, s32 count, const char **file_paths);
typedef void (*window_close_callback_t)(void *);

// General API for platform
typedef struct neko_platform_i {
    /*============================================================
    // Platform Initilization / De-Initialization
    ============================================================*/
    neko_result (*init)(struct neko_platform_i *);
    neko_result (*shutdown)(struct neko_platform_i *);

    /*============================================================
    // Platform Util
    ============================================================*/
    void (*sleep)(f32 ms);                   // Sleeps platform for time in ms
    f64 (*elapsed_time)();                   // Returns time in ms since initialization of platform
    neko_platform_meminfo (*get_meminfo)();  // 获取系统内存使用信息
    neko_vec2 (*get_opengl_ver)();           // 获取当前 OpenGL 版本
    void *(*get_sys_handle)(neko_resource_handle handle);

    /*============================================================
    // Platform Video
    ============================================================*/
    void (*enable_vsync)(bool enabled);

    /*============================================================
    // Platform UUID
    ============================================================*/
    struct neko_uuid (*generate_uuid)();
    void (*uuid_to_string)(char *temp_buffer, const struct neko_uuid *uuid);  // Expects a temp buffer with at leat 32 bytes
    u32 (*hash_uuid)(const struct neko_uuid *uuid);

    /*============================================================
    // Platform Input
    ============================================================*/
    neko_result (*process_input)(neko_platform_input *);

    void (*update_input)(neko_platform_input *);
    void (*press_key)(neko_platform_keycode code);
    void (*release_key)(neko_platform_keycode code);
    bool (*was_key_down)(neko_platform_keycode code);
    bool (*key_pressed)(neko_platform_keycode code);
    bool (*key_down)(neko_platform_keycode code);
    bool (*key_released)(neko_platform_keycode code);

    void (*press_mouse_button)(neko_platform_mouse_button_code code);
    void (*release_mouse_button)(neko_platform_mouse_button_code code);
    bool (*was_mouse_down)(neko_platform_mouse_button_code code);
    bool (*mouse_pressed)(neko_platform_mouse_button_code code);
    bool (*mouse_down)(neko_platform_mouse_button_code code);
    bool (*mouse_released)(neko_platform_mouse_button_code code);
    void (*set_mouse_position)(neko_resource_handle, f64, f64);

    neko_vec2 (*mouse_delta)();
    neko_vec2 (*mouse_position)();
    void (*mouse_position_x_y)(f32 *x, f32 *y);
    neko_vec2 (*mouse_wheel)();
    void (*mouse_wheel_x_y)(f32 *x, f32 *y);
    bool (*mouse_moved)();

    /*============================================================
    // Platform Window
    ============================================================*/
    neko_resource_handle (*create_window)(const char *title, u32 width, u32 height);
    void *(*create_window_internal)(const char *title, u32 width, u32 height);
    void (*window_swap_buffer)(neko_resource_handle handle);
    neko_vec2 (*window_size)(neko_resource_handle handle);
    void (*set_window_size)(neko_resource_handle handle, s32 width, s32 height);
    void (*window_size_w_h)(neko_resource_handle handle, s32 *width, s32 *height);
    void (*set_cursor)(neko_resource_handle handle, neko_platform_cursor cursor);
    neko_resource_handle (*main_window)();
    void (*set_dropped_files_callback)(neko_resource_handle, dropped_files_callback_t);
    void (*set_window_close_callback)(neko_resource_handle, window_close_callback_t);
    void *(*raw_window_handle)(neko_resource_handle);  // Do not call unless you know what you're doing
    neko_vec2 (*frame_buffer_size)(neko_resource_handle handle);
    void (*frame_buffer_size_w_h)(neko_resource_handle handle, s32 *w, s32 *h);

    /*============================================================
    // Platform File IO
    ============================================================*/

    // Will return a null buffer if file does not exist or allocation fails
    char *(*read_file_contents)(const char *file_path, const char *mode, s32 *sz);
    neko_result (*write_file_contents)(const char *file_path, const char *mode, void *data, usize data_type_size, usize data_size);
    neko_result (*write_str_to_file)(const char *contents, const char *mode, usize sz, const char *output_path);
    bool (*file_exists)(const char *file_path);
    s32 (*file_size_in_bytes)(const char *file_path);
    void (*file_extension)(char *buffer, usize buffer_sz, const char *file_path);
    neko_string (*get_path)(const neko_string &);
    neko_string (*abbreviate_path)(const neko_string &, s32);

    // Settings for platform, including video, audio
    neko_platform_settings settings;

    // Time
    neko_platform_time time;

    // Input
    neko_platform_input input;

    // Ctx
    neko_platform_ctx ctx;

    // For now, just keep a window here as main window...
    neko_slot_array(neko_platform_window_ptr) windows;
    neko_dyn_array(neko_resource_handle) active_window_handles;

    // Cursors
    void *cursors[neko_platform_cursor_count];

} neko_platform_i;

/*===============================
// Platform User Provided Funcs
===============================*/

extern struct neko_platform_i *neko_platform_construct();

/*============================
// Platform Default Funcs
============================*/

void __neko_default_init_platform(struct neko_platform_i *platform);

void __neko_verify_platform_correctness(struct neko_platform_i *platform);

/*============================
// Platform Input
============================*/

void __neko_platform_update_input();

bool __neko_platform_was_key_down(neko_platform_keycode code);

bool __neko_platform_key_down(neko_platform_keycode code);

bool __neko_platform_key_pressed(neko_platform_keycode code);

bool __neko_platform_key_released(neko_platform_keycode code);

bool __neko_platform_was_mouse_down(neko_platform_mouse_button_code code);

void __neko_platform_press_mouse_button(neko_platform_mouse_button_code code);

void __neko_platform_release_mouse_button(neko_platform_mouse_button_code code);

bool __neko_platform_mouse_down(neko_platform_mouse_button_code code);

bool __neko_platform_mouse_pressed(neko_platform_mouse_button_code code);

bool __neko_platform_mouse_released(neko_platform_mouse_button_code code);

neko_vec2 __neko_platform_mouse_delta();

neko_vec2 __neko_platform_mouse_position();

void __neko_platform_mouse_position_x_y(f32 *x, f32 *y);

void __neko_platform_mouse_wheel_x_y(f32 *x, f32 *y);

void __neko_platform_press_key(neko_platform_keycode code);

void __neko_platform_release_key(neko_platform_keycode code);

/*============================
// Platform Window
============================*/
neko_resource_handle __neko_platform_create_window(const char *title, u32 width, u32 height);
neko_resource_handle __neko_platform_main_window();

/*============================
// Platform File IO
============================*/
bool __neko_platform_file_exists(const char *file_path);
char *__neko_platform_read_file_contents_into_string_null_term(const char *file_path, const char *mode, s32 *sz);
neko_result __neko_platform_write_str_to_file(const char *contents, const char *mode, usize sz, const char *output_path);
void __neko_platform_file_extension(char *buffer, usize buffer_sz, const char *file_path);
neko_string __neko_platform_get_path(const neko_string &path);
neko_string __neko_platform_abbreviate_path(const neko_string &path, s32 max_lenght = 30);

#define neko_file_path(x) (neko_engine_subsystem(platform))->get_path(x).c_str()
#define neko_abbreviate_path(x) (neko_engine_subsystem(platform))->abbreviate_path(x, 30).c_str()

/*============================
// Platform Util
============================*/

struct neko_uuid __neko_platform_generate_uuid();

void __neko_platform_uuid_to_string(char *temp_buffer, const struct neko_uuid *uuid);  // Expects a temp buffer with at leat 32 bytes

u32 __neko_platform_hash_uuid(const struct neko_uuid *uuid);

/*============================
// Platform Thread
============================*/

static inline uint64_t neko_get_thread_id() {
#if defined(NEKO_PLATFORM_WIN)
    return (uint64_t)GetCurrentThreadId();
#elif defined(NEKO_PLATFORM_LINUX)
    return (uint64_t)syscall(SYS_gettid);
#elif defined(NEKO_PLATFORM_APPLE)
    return (mach_port_t)::pthread_mach_thread_np(pthread_self());
#else
#error "Unsupported platform!"
#endif
}

// Thread Local Storage(TLS)
// msvc: https://learn.microsoft.com/en-us/cpp/parallel/thread-local-storage-tls

#ifdef NEKO_PLATFORM_WIN

static inline u32 neko_tls_allocate() { return (u32)TlsAlloc(); }

static inline void neko_tls_set_value(u32 _handle, void *_value) { TlsSetValue(_handle, _value); }

static inline void *neko_tls_get_value(u32 _handle) { return TlsGetValue(_handle); }

static inline void neko_tls_free(u32 _handle) { TlsFree(_handle); }

#else

static inline pthread_key_t neko_tls_allocate() {
    pthread_key_t handle;
    pthread_key_create(&handle, NULL);
    return handle;
}

static inline void neko_tls_set_value(pthread_key_t _handle, void *_value) { pthread_setspecific(_handle, _value); }

static inline void *neko_tls_get_value(pthread_key_t _handle) { return pthread_getspecific(_handle); }

static inline void neko_tls_free(pthread_key_t _handle) { pthread_key_delete(_handle); }

#endif

#if defined(NEKO_PLATFORM_WIN)
typedef CRITICAL_SECTION neko_mutex;

static inline void neko_mutex_init(neko_mutex *_mutex) { InitializeCriticalSection(_mutex); }

static inline void neko_mutex_destroy(neko_mutex *_mutex) { DeleteCriticalSection(_mutex); }

static inline void neko_mutex_lock(neko_mutex *_mutex) { EnterCriticalSection(_mutex); }

static inline int neko_mutex_trylock(neko_mutex *_mutex) { return TryEnterCriticalSection(_mutex) ? 0 : 1; }

static inline void neko_mutex_unlock(neko_mutex *_mutex) { LeaveCriticalSection(_mutex); }

#elif defined(NEKO_PLATFORM_POSIX)
typedef pthread_mutex_t neko_mutex;

static inline void neko_mutex_init(neko_mutex *_mutex) { pthread_mutex_init(_mutex, NULL); }

static inline void neko_mutex_destroy(neko_mutex *_mutex) { pthread_mutex_destroy(_mutex); }

static inline void neko_mutex_lock(neko_mutex *_mutex) { pthread_mutex_lock(_mutex); }

static inline int neko_mutex_trylock(neko_mutex *_mutex) { return pthread_mutex_trylock(_mutex); }

static inline void neko_mutex_unlock(neko_mutex *_mutex) { pthread_mutex_unlock(_mutex); }

#else
#error "Unsupported platform!"
#endif

class neko_pthread_mutex {
    neko_mutex m_mutex;

    neko_pthread_mutex(const neko_pthread_mutex &_rhs);
    neko_pthread_mutex &operator=(const neko_pthread_mutex &_rhs);

public:
    inline neko_pthread_mutex() { neko_mutex_init(&m_mutex); }
    inline ~neko_pthread_mutex() { neko_mutex_destroy(&m_mutex); }
    inline void lock() { neko_mutex_lock(&m_mutex); }
    inline void unlock() { neko_mutex_unlock(&m_mutex); }
    inline bool tryLock() { return (neko_mutex_trylock(&m_mutex) == 0); }
};

class neko_scoped_mutex_locker {
    neko_pthread_mutex &m_mutex;

    neko_scoped_mutex_locker();
    neko_scoped_mutex_locker(const neko_scoped_mutex_locker &);
    neko_scoped_mutex_locker &operator=(const neko_scoped_mutex_locker &);

public:
    inline neko_scoped_mutex_locker(neko_pthread_mutex &_mutex) : m_mutex(_mutex) { m_mutex.lock(); }
    inline ~neko_scoped_mutex_locker() { m_mutex.unlock(); }
};

#include <functional>

namespace neko {

// Job系统详细设计可以见
// https://turanszkij.wordpress.com/2018/11/24/simple-job-system-using-standard-c/

struct neko_job_dispatch_args {
    u32 job_index;
    u32 group_index;
};

void neko_job_init();
void neko_job_execute(const std::function<void()> &job);
void neko_job_dispatch(u32 jobCount, u32 groupSize, const std::function<void(neko_job_dispatch_args)> &job);
bool neko_job_is_busy();
void neko_job_wait();

typedef struct {
    const char *function;  // name of function containing address of function.
    const char *file;      // file where symbol is defined, might not work on all platforms.
    unsigned int line;     // line in file where symbol is defined, might not work on all platforms.
    unsigned int offset;   // offset from start of function where call was made.
} neko_platform_callstack_symbol_t;

int neko_platform_callstack(int skip_frames, void **addresses, int num_addresses);
int neko_platform_callstack_symbols(void **addresses, neko_platform_callstack_symbol_t *out_syms, int num_addresses, char *memory, int mem_size);

void neko_platform_print_callstack();

}  // namespace neko

#endif  // NEKO_PLATFORM_H
