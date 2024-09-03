#ifndef EDIT_H
#define EDIT_H

#include <stdbool.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/bootstrap.h"
#include "engine/entity.h"
#include "engine/graphics.h"
#include "engine/luax.hpp"

void render_uniform_variable(GLuint program, GLenum type, const char *name, GLint location);
void inspect_shader(const char *label, GLuint program);
void inspect_vertex_array(const char *label, GLuint vao);

NEKO_SCRIPT(console,

            // 打印到此实体 添加到 gui_text 系统 (如果尚未在其中) 设置为entity_nil以禁用
            NEKO_EXPORT void console_set_entity(NativeEntity ent);

            NEKO_EXPORT NativeEntity console_get_entity();  // 如果没有设置则entity_nil

            NEKO_EXPORT void console_set_visible(bool visible);

            NEKO_EXPORT bool console_get_visible();

            NEKO_EXPORT void console_puts(const char *s);

            NEKO_EXPORT void console_printf(const char *fmt, ...);

)

void console_init();
void console_fini();

/*=============================
// Console
=============================*/

typedef void (*neko_console_func)(int argc, char **argv);

typedef struct neko_console_command_t {
    neko_console_func func;
    const char *name;
    const char *desc;
} neko_console_command_t;

typedef struct neko_console_t {
    char tb[2048];     // text buffer
    char cb[10][256];  // "command" buffer
    int current_cb_idx;

    f32 y;
    f32 size;
    f32 open_speed;
    f32 close_speed;

    bool open;
    int last_open_state;
    bool autoscroll;

    neko_console_command_t *commands;
    int commands_len;
} neko_console_t;

extern neko_console_t g_console;

struct ui_context_t;
struct ui_selector_desc_t;
struct rect_t;

void neko_console(neko_console_t *console, ui_context_t *ctx, rect_t *rect, const ui_selector_desc_t *desc);

/*=============================
// Inspect
=============================*/

namespace neko {

class CCharacter {
public:
    int age = 0;
    float stamina = 0.0f;
    double skill = 0.0;
};

struct luainspector_property_st {
    static void Render_TypeGeneric(const char *label, void *value) { neko_assert(value); }

    static void Render_TypeFloat(const char *label, void *value) {
        NEKO_STATIC_CAST(float, value, fValue);
        // ImGui::InputFloat(label, fValue);
    }

    static void Render_TypeBool(const char *label, void *value) {
        NEKO_STATIC_CAST(bool, value, bValue);
        // ImGui::Checkbox(label, bValue);
    }

    static void Render_TypeConstChar(const char *label, void *value) {
        NEKO_STATIC_CAST(char, value, cValue);
        // ImGui::InputText(label, cValue, std::strlen(cValue));
    }

    static void Render_TypeDouble(const char *label, void *value) {
        NEKO_STATIC_CAST(double, value, dValue);
        // ImGui::InputDouble(label, dValue);
    }

    static void Render_TypeInt(const char *label, void *value) {
        NEKO_STATIC_CAST(int, value, iValue);
        // ImGui::InputInt(label, iValue);
    }

    static void Render_TypeCharacter(const char *label, void *value) {
        NEKO_STATIC_CAST(CCharacter, value, character);
        // ImGui::InputInt("Age", &character->age);
        // ImGui::InputFloat("Stamina", &character->stamina);
        // ImGui::InputDouble("Skill", &character->skill);
    }
};

struct luainspector_property {
    std::string name;
    std::string label;
    std::type_index param_type = std::type_index(typeid(int));
    void *param = nullptr;

    luainspector_property() {}
};

inline bool incomplete_chunk_error(const char *err, std::size_t len) { return err && (std::strlen(err) >= 5u) && (0 == std::strcmp(err + len - 5u, "<eof>")); }

struct luainspector_hints {
    static std::string clean_table_list(const std::string &str);
    static bool try_replace_with_metaindex(lua_State *L);
    static bool collect_hints_recurse(lua_State *L, std::vector<std::string> &possible, const std::string &last, bool usehidden, unsigned left);
    static void prepare_hints(lua_State *L, std::string str, std::string &last);
    static bool collect_hints(lua_State *L, std::vector<std::string> &possible, const std::string &last, bool usehidden);
    static std::string common_prefix(const std::vector<std::string> &possible);
};

class luainspector;

struct command_line_input_callback_UserData {
    std::string *Str;
    // ImGuiInputTextCallback ChainCallback;
    void *ChainCallbackUserData;
    neko::luainspector *luainspector_ptr;
};

struct inspect_table_config {
    const_str search_str = 0;
    bool is_non_function = false;
};

enum luainspector_logtype { LUACON_LOG_TYPE_WARNING = 1, LUACON_LOG_TYPE_ERROR = 2, LUACON_LOG_TYPE_NOTE = 4, LUACON_LOG_TYPE_SUCCESS = 0, LUACON_LOG_TYPE_MESSAGE = 3 };

class luainspector {
private:
    std::vector<std::pair<std::string, luainspector_logtype>> messageLog;

    lua_State *L;
    std::vector<std::string> m_history;
    int m_hindex;

    std::string cmd, cmd2;
    bool m_should_take_focus{false};
    // ImGuiID m_input_text_id{0u};
    // ImGuiID m_previously_active_id{0u};
    std::string_view m_autocomlete_separator{" | "};
    std::vector<std::string> m_current_autocomplete_strings{};

    std::unordered_map<std::type_index, std::function<void(const char *, void *)>> m_type_render_functions;
    std::vector<luainspector_property> m_property_map;
    std::vector<void *> m_variable_pool;

private:
    // static int try_push_style(ImGuiCol col, const std::optional<ImVec4>& color) {
    //     if (color) {
    //         ImGui::PushStyleColor(col, *color);
    //         return 1;
    //     }
    //     return 0;
    // }

public:
    void console_draw(bool &textbox_react) noexcept;
    void print_line(const std::string &msg, luainspector_logtype type) noexcept;

    static bool visible;

    static luainspector *get_from_registry(lua_State *L);
    static void inspect_table(lua_State *L, inspect_table_config &cfg);
    static int luainspector_init(lua_State *L);
    static int luainspector_draw(lua_State *L);
    static int luainspector_get(lua_State *L);
    // static int command_line_callback_st(ImGuiInputTextCallbackData* data) noexcept;

    void setL(lua_State *L);
    // int command_line_input_callback(ImGuiInputTextCallbackData* data);
    // bool command_line_input(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    // void show_autocomplete() noexcept;
    std::string read_history(int change);
    std::string try_complete(std::string inputbuffer);
    void print_luastack(int first, int last, luainspector_logtype logtype);
    bool try_eval(std::string m_buffcmd, bool addreturn);

    inline void variable_pool_free() {
        for (const auto &var : this->m_variable_pool) {
            delete (char *)var;
        }
    }

    template <typename T>
    void register_function(std::function<void(const char *, void *)> function) {
        this->m_type_render_functions.insert_or_assign(std::type_index(typeid(T)), function);
    }

    template <typename T>
    void property_register(const std::string &name, T *param, const std::string &label = "") {
        luainspector_property type_property;
        type_property.name = name;
        if (label.empty())
            type_property.label = name;
        else
            type_property.label = label;

        type_property.param_type = std::type_index(typeid(*param));
        type_property.param = param;

        this->m_property_map.push_back(type_property);
    }

    template <typename T>
    void property_register(const std::string &name, const std::string &label = "") {

        T *param = new T();
        this->m_variable_pool.push_back(param);

        property_register<T>(name, param, label);
    }

    inline void property_remove(const std::string &name) {

        auto property_it = this->m_property_map.begin();
        while (property_it != this->m_property_map.end()) {
            if (property_it->name == name) break;
        }

        this->m_property_map.erase(property_it);
    }

    template <typename T>
    void property_update(const std::string &name, const std::string &newName, T *param) {
        property_remove(name);
        property_register<T>(newName, param);
    }

    inline luainspector_property *property_get(const std::string &name) {
        luainspector_property *ret_value = nullptr;

        for (auto &property_it : this->m_property_map) {
            if (property_it.name == name) ret_value = &property_it;
        }

        return ret_value;
    }

    template <typename T>
    T *property_getv(const std::string &name) {

        T *ret_value = nullptr;
        const auto &param_it = property_get(name);
        if (param_it) {
            ret_value = reinterpret_cast<T *>(param_it->param);
        }

        return ret_value;
    }

    template <typename T>
    T *property_setv(const std::string &name, const T &value) {
        const auto &param = property_getv<T>(name);
        IM_ASSERT(param)

        *param = T(value);
    }
};
}  // namespace neko

NEKO_SCRIPT(inspector,

            // NEKO_EXPORT void console_set_entity(NativeEntity ent);

            // NEKO_EXPORT NativeEntity console_get_entity();  // 如果没有设置则entity_nil

            NEKO_EXPORT void inspector_set_visible(bool visible);

            NEKO_EXPORT bool inspector_get_visible();

)

#endif
