#pragma once

#include "engine/imgui.hpp"
#include "engine/scripting/scripting.h"
#include "engine/ecs/entity.h"
#include "base/common/color.hpp"

// OpenGL
#include <glad/glad.h>

namespace Neko {

using namespace luabind;

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

    // static void Render_TypeCharacter(const char* label, void* value) {
    //     NEKO_STATIC_CAST(CCharacter, value, character);
    //     ImGui::InputInt("Age", &character->age);
    //     ImGui::InputFloat("Stamina", &character->stamina);
    //     ImGui::InputDouble("Skill", &character->skill);
    // }
};

struct luainspector_property {
    std::string name;
    std::string label;
    std::type_index param_type = std::type_index(typeid(int));
    void* param = nullptr;

    luainspector_property() {}
};

inline bool incomplete_chunk_error(const char* err, std::size_t len) { return err && (std::strlen(err) >= 5u) && (0 == std::strcmp(err + len - 5u, "<eof>")); }

class Inspector;

struct command_line_input_callback_UserData {
    std::string* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
    Inspector* luainspector_ptr;
};

struct inspect_table_config {
    const_str search_str = 0;
    bool is_non_function = false;
};

class Inspector {
public:
    struct Hints {
        static std::string clean_table_list(const std::string& str);
        static bool try_replace_with_metaindex(lua_State* L);
        static bool collect_hints_recurse(lua_State* L, std::vector<std::string>& possible, const std::string& last, bool usehidden, unsigned left);
        static void prepare_hints(lua_State* L, std::string str, std::string& last);
        static bool collect_hints(lua_State* L, std::vector<std::string>& possible, const std::string& last, bool usehidden);
        static std::string common_prefix(const std::vector<std::string>& possible);
    };

    std::unordered_map<int, std::string> aseModeNames;

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

    std::deque<std::string> messageLog;

    bool hotload_enable;

private:
    inline int try_push_style(ImGuiCol col, const std::optional<ImVec4>& color) {
        if (color) {
            ImGui::PushStyleColor(col, *color);
            return 1;
        }
        return 0;
    }

    void GuiAnalysisWindow();
    void GuiAsepriteWindow();

public:
    Inspector();
    ~Inspector();

    void console_draw(bool& textbox_react) noexcept;

    void inspect_table(lua_State* L, inspect_table_config& cfg);
    int init(lua_State* L);
    int OnImGui(lua_State* L);

    void setL(lua_State* L);
    int command_line_input_callback(ImGuiInputTextCallbackData* data);
    bool command_line_input(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    void show_autocomplete() noexcept;
    std::string read_history(int change);
    std::string try_complete(std::string inputbuffer);
    void print(std::string msg, Logger::Level logtype = Logger::Level::INFO);
    void print_luastack(int first, int last, Logger::Level logtype);
    bool try_eval(std::string m_buffcmd, bool addreturn);
};

}  // namespace Neko

void render_uniform_variable(GLuint program, GLenum type, const char* name, GLint location);
void inspect_shader(const char* label, GLuint program);
void inspect_vertex_array(const char* label, GLuint vao);

void edit_set_editable(CEntity ent, bool editable);
bool edit_get_editable(CEntity ent);
void edit_set_grid_size(vec2 size);
vec2 edit_get_grid_size();
bool edit_bboxes_has(CEntity ent);
int edit_bboxes_get_num();
CEntity edit_bboxes_get_nth_ent(int n);
void edit_bboxes_set_selected(CEntity ent, bool selected);
void edit_line_add(vec2 a, vec2 b, f32 point_size, Color color);
void edit_set_enabled(bool e);
bool edit_get_enabled();

// 用于点击选择等
void edit_bboxes_update(CEntity ent, BBox bbox);  // 合并bbox
BBox edit_bboxes_get_nth_bbox(int n);
void edit_line_add_xy(vec2 p, f32 point_size, Color color);
int edit_clear(Event evt);

class Editor : public SingletonClass<Editor> {
private:
    bool enabled;
    bool ViewportOnEvent;

    Inspector* inspector{};
    int ctag_tid;

    int callbackId;

public:
    f32 viewportMouseX, viewportMouseY;

    std::unordered_set<CEntity, CEntityHash, CEntityEqual> SelectTable;
    EcsId SingleSelectID{0};
    CEntity SingleSelectEnt;

    CEntity editCamera;

    // 拖拽功能
    vec2 camera_drag_mouse_prev;
    bool edit_camera_drag{false};

    // 缩放功能
    float camera_zoom_factor = 0.0f;

    float camera_default_height = 150.0f;

public:
    void edit_init();
    void edit_fini();
    int OnPostUpdate(Event evt);
    int OnUpdate(Event evt);
    void edit_draw_all();
    void OnImGui();

    int GetEditorLuaFunc(String name);

    std::vector<CEntity> GetEntitiesUnderPos(vec2 pos);
    vec2 EditorGetMouseUnit();
    std::vector<CEntity> GetEntitiesUnderMouse();

    inline EcsId GetSingleSelectID() const { return SingleSelectID; }
    inline CEntity GetSingleSelectEnt() const { return SingleSelectEnt; }

    void SelectClickSingle();
    void SelectClickMulti();

    void initializeEditCamera();

    void camera_drag_OnUpdate();
    void camera_drag_start();
    void camera_drag_end();
    void camera_zoom(float f);
    void camera_zoom_in();
    void camera_zoom_out();

    template <typename... Args>
    inline void PushEditorEvent(EventEnum type, Args&&... args) {
        lua_State* L = ENGINE_LUA();
        String& evt_name = the<EventHandler>().EventName(type);
        int n = sizeof...(args);
        // call ns.edit.__OnEvent(event, ...)
        GetEditorLuaFunc("__OnEvent");
        lua_pushstring(L, evt_name.cstr());
        the<EventHandler>().EventPushLuaArgs(L, std::forward<Args>(args)...);
        errcheck(L, luax_pcall_nothrow(L, 1 + n, 0));
    }

    bool& get_enable() { return enabled; }
    bool& ViewportIsOnEvent() { return ViewportOnEvent; }
};