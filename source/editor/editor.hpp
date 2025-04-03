#pragma once

#include "engine/imgui.hpp"
#include "base/scripting/scripting.h"

namespace Neko {

using namespace luabind;

class CCharacter {
public:
    int age = 0;
    float stamina = 0.0f;
    double skill = 0.0;
};

struct luainspector_property_st {
    static void Render_TypeGeneric(const char* label, void* value) { IM_ASSERT(value); }

    static void Render_TypeFloat(const char* label, void* value) {
        NEKO_STATIC_CAST(float, value, fValue);
        ImGui::InputFloat(label, fValue);
    }

    static void Render_TypeBool(const char* label, void* value) {
        NEKO_STATIC_CAST(bool, value, bValue);
        ImGui::Checkbox(label, bValue);
    }

    static void Render_TypeConstChar(const char* label, void* value) {
        NEKO_STATIC_CAST(char, value, cValue);
        ImGui::InputText(label, cValue, std::strlen(cValue));
    }

    static void Render_TypeDouble(const char* label, void* value) {
        NEKO_STATIC_CAST(double, value, dValue);
        ImGui::InputDouble(label, dValue);
    }

    static void Render_TypeInt(const char* label, void* value) {
        NEKO_STATIC_CAST(int, value, iValue);
        ImGui::InputInt(label, iValue);
    }

    static void Render_TypeCharacter(const char* label, void* value) {
        NEKO_STATIC_CAST(CCharacter, value, character);
        ImGui::InputInt("Age", &character->age);
        ImGui::InputFloat("Stamina", &character->stamina);
        ImGui::InputDouble("Skill", &character->skill);
    }
};

struct luainspector_property {
    std::string name;
    std::string label;
    std::type_index param_type = std::type_index(typeid(int));
    void* param = nullptr;

    luainspector_property() {}
};

inline bool incomplete_chunk_error(const char* err, std::size_t len) { return err && (std::strlen(err) >= 5u) && (0 == std::strcmp(err + len - 5u, "<eof>")); }

class LuaInspector;

struct command_line_input_callback_UserData {
    std::string* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
    Neko::LuaInspector* luainspector_ptr;
};

struct inspect_table_config {
    const_str search_str = 0;
    bool is_non_function = false;
};

class LuaInspector {
public:
    struct Hints {
        static std::string clean_table_list(const std::string& str);
        static bool try_replace_with_metaindex(lua_State* L);
        static bool collect_hints_recurse(lua_State* L, std::vector<std::string>& possible, const std::string& last, bool usehidden, unsigned left);
        static void prepare_hints(lua_State* L, std::string str, std::string& last);
        static bool collect_hints(lua_State* L, std::vector<std::string>& possible, const std::string& last, bool usehidden);
        static std::string common_prefix(const std::vector<std::string>& possible);
    };

private:
    lua_State* L{};
    std::vector<std::string> m_history;
    std::size_t m_hindex;

    std::string cmd, cmd2;
    bool m_should_take_focus{false};
    ImGuiID m_input_text_id{0u};
    ImGuiID m_previously_active_id{0u};
    std::string_view m_autocomlete_separator{" | "};
    std::vector<std::string> m_current_autocomplete_strings{};

    std::unordered_map<std::type_index, std::function<void(const char*, void*)>> m_type_render_functions;
    std::vector<luainspector_property> m_property_map;
    std::vector<void*> m_variable_pool;

    std::deque<std::string> messageLog;

    int callbackId;

private:
    inline int try_push_style(ImGuiCol col, const std::optional<ImVec4>& color) {
        if (color) {
            ImGui::PushStyleColor(col, *color);
            return 1;
        }
        return 0;
    }

public:
    LuaInspector();
    ~LuaInspector();

    void console_draw(bool& textbox_react) noexcept;

    bool visible{false};

    void inspect_table(lua_State* L, inspect_table_config& cfg);
    int luainspector_init(lua_State* L);
    int luainspector_draw(lua_State* L);

    void setL(lua_State* L);
    int command_line_input_callback(ImGuiInputTextCallbackData* data);
    bool command_line_input(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    void show_autocomplete() noexcept;
    std::string read_history(int change);
    std::string try_complete(std::string inputbuffer);
    void print(std::string msg, Logger::Level logtype = Logger::Level::INFO);
    void print_luastack(int first, int last, Logger::Level logtype);
    bool try_eval(std::string m_buffcmd, bool addreturn);

    inline void variable_pool_free() {
        for (const auto& var : this->m_variable_pool) {
            delete (char*)var;
        }
    }

    template <typename T>
    void register_function(std::function<void(const char*, void*)> function) {
        this->m_type_render_functions.insert_or_assign(std::type_index(typeid(T)), function);
    }

    template <typename T>
    void property_register(const std::string& name, T* param, const std::string& label = "") {
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
    void property_register(const std::string& name, const std::string& label = "") {

        T* param = new T();
        this->m_variable_pool.push_back(param);

        property_register<T>(name, param, label);
    }

    inline void property_remove(const std::string& name) {

        auto property_it = this->m_property_map.begin();
        while (property_it != this->m_property_map.end()) {
            if (property_it->name == name) break;
        }

        this->m_property_map.erase(property_it);
    }

    template <typename T>
    void property_update(const std::string& name, const std::string& newName, T* param) {
        property_remove(name);
        property_register<T>(newName, param);
    }

    inline luainspector_property* property_get(const std::string& name) {
        luainspector_property* ret_value = nullptr;

        for (auto& property_it : this->m_property_map) {
            if (property_it.name == name) ret_value = &property_it;
        }

        return ret_value;
    }

    template <typename T>
    T* property_getv(const std::string& name) {

        T* ret_value = nullptr;
        const auto& param_it = property_get(name);
        if (param_it) {
            ret_value = reinterpret_cast<T*>(param_it->param);
        }

        return ret_value;
    }

    template <typename T>
    T* property_setv(const std::string& name, const T& value) {
        const auto& param = property_getv<T>(name);
        IM_ASSERT(param)

        *param = T(value);
    }
};
}  // namespace Neko