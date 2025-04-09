#include "engine/imgui.hpp"

#include "base/common/color.hpp"
#include "base/common/profiler.hpp"
#include "base/common/string.hpp"
#include "engine/bootstrap.h"
#include "engine/scripting/lua_util.h"

using namespace Neko::luabind;

// imgui
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#if 1

namespace Neko::ImGuiWrap::wrap_ImGuiInputTextCallbackData {
void pointer(lua_State* L, ImGuiInputTextCallbackData& v);
}

namespace Neko::ImGuiWrap::util {

static lua_CFunction str_format = NULL;

lua_Integer field_tointeger(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    auto v = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return v;
}

lua_Number field_tonumber(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    auto v = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return v;
}

bool field_toboolean(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    bool v = !!lua_toboolean(L, -1);
    lua_pop(L, 1);
    return v;
}

ImTextureID get_texture_id(lua_State* L, int idx) {
    // int lua_handle = (int)luaL_checkinteger(L, idx);
    // if (auto id = ImGui_ImplBgfx_GetTextureID(lua_handle)) {
    //     return *id;
    // }
    // luaL_error(L, "Invalid handle type TEXTURE");
    // std::unreachable();
    neko_assert(0);
    return 0;
}

const char* format(lua_State* L, int idx) {
    lua_pushcfunction(L, str_format);
    lua_insert(L, idx);
    lua_call(L, lua_gettop(L) - idx, 1);
    return lua_tostring(L, -1);
}

static void* strbuf_realloc(lua_State* L, void* ptr, size_t osize, size_t nsize) {
    void* ud;
    lua_Alloc allocator = lua_getallocf(L, &ud);
    return allocator(ud, ptr, osize, nsize);
}

static int strbuf_assgin(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    size_t newsize = 0;
    const char* newbuf = luaL_checklstring(L, 2, &newsize);
    newsize++;
    if (newsize > sbuf->size) {
        sbuf->data = (char*)strbuf_realloc(L, sbuf->data, sbuf->size, newsize);
        sbuf->size = newsize;
    }
    memcpy(sbuf->data, newbuf, newsize);
    return 0;
}

static int strbuf_resize(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    size_t newsize = (size_t)luaL_checkinteger(L, 2);
    sbuf->data = (char*)strbuf_realloc(L, sbuf->data, sbuf->size, newsize);
    sbuf->size = newsize;
    return 0;
}

static int strbuf_tostring(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    lua_pushstring(L, sbuf->data);
    return 1;
}

static int strbuf_release(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    strbuf_realloc(L, sbuf->data, sbuf->size, 0);
    sbuf->data = NULL;
    sbuf->size = 0;
    return 0;
}

static constexpr size_t kStrBufMinSize = 256;

strbuf* strbuf_create(lua_State* L, int idx) {
    size_t sz;
    const char* text = lua_tolstring(L, idx, &sz);
    auto sbuf = (strbuf*)lua_newuserdatauv(L, sizeof(strbuf), 0);
    if (text == NULL) {
        sbuf->size = kStrBufMinSize;
        sbuf->data = (char*)strbuf_realloc(L, NULL, 0, sbuf->size);
        sbuf->data[0] = '\0';
    } else {
        sbuf->size = (std::max)(sz + 1, kStrBufMinSize);
        sbuf->data = (char*)strbuf_realloc(L, NULL, 0, sbuf->size);
        memcpy(sbuf->data, text, sz + 1);
    }
    if (luaL_newmetatable(L, "ImGui::StringBuf")) {
        lua_pushcfunction(L, strbuf_tostring);
        lua_setfield(L, -2, "__tostring");
        lua_pushcfunction(L, strbuf_release);
        lua_setfield(L, -2, "__gc");
        static luaL_Reg l[] = {
                {"Assgin", strbuf_assgin},
                {"Resize", strbuf_resize},
                {NULL, NULL},
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
    return sbuf;
}

strbuf* strbuf_get(lua_State* L, int idx) {
    if (lua_type(L, idx) == LUA_TUSERDATA) {
        auto sbuf = (strbuf*)luaL_checkudata(L, idx, "ImGui::StringBuf");
        return sbuf;
    }
    luaL_checktype(L, idx, LUA_TTABLE);
    int t = lua_geti(L, idx, 1);
    if (t != LUA_TSTRING && t != LUA_TNIL) {
        auto sbuf = (strbuf*)luaL_checkudata(L, -1, "ImGui::StringBuf");
        lua_pop(L, 1);
        return sbuf;
    }
    auto sbuf = strbuf_create(L, -1);
    lua_replace(L, -2);
    lua_seti(L, idx, 1);
    return sbuf;
}

int input_callback(ImGuiInputTextCallbackData* data) {
    auto ctx = (input_context*)data->UserData;
    lua_State* L = ctx->L;
    lua_pushvalue(L, ctx->callback);
    wrap_ImGuiInputTextCallbackData::pointer(L, *data);
    if (lua_pcall(L, 1, 1, 1) != LUA_OK) {
        return 1;
    }
    lua_Integer retval = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return (int)retval;
}

void create_table(lua_State* L, std::span<TableInteger> l) {
    lua_createtable(L, 0, (int)l.size());
    for (auto const& e : l) {
        lua_pushinteger(L, e.value);
        lua_setfield(L, -2, e.name);
    }
}

void set_table(lua_State* L, std::span<TableAny> l) {
    for (auto const& e : l) {
        e.value(L);
        lua_setfield(L, -2, e.name);
    }
}

static void set_table(lua_State* L, std::span<luaL_Reg> l, int nup) {
    luaL_checkstack(L, nup, "too many upvalues");
    for (auto const& e : l) {
        for (int i = 0; i < nup; i++) {
            lua_pushvalue(L, -nup);
        }
        lua_pushcclosure(L, e.func, nup);
        lua_setfield(L, -(nup + 2), e.name);
    }
    lua_pop(L, nup);
}

static int make_flags(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int i, t;
    lua_Integer r = 0;
    for (i = 1; (t = lua_geti(L, 1, i)) != LUA_TNIL; i++) {
        if (t != LUA_TSTRING) luaL_error(L, "Flag name should be string, it's %s", lua_typename(L, t));
        if (lua_gettable(L, lua_upvalueindex(1)) != LUA_TNUMBER) {
            lua_geti(L, 1, i);
            luaL_error(L, "Invalid flag %s.%s", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, -1));
        }
        lua_Integer v = lua_tointeger(L, -1);
        lua_pop(L, 1);
        r |= v;
    }
    lua_pushinteger(L, r);
    return 1;
}

void struct_gen(lua_State* L, const char* name, std::span<luaL_Reg> funcs, std::span<luaL_Reg> setters, std::span<luaL_Reg> getters) {
    lua_newuserdatauv(L, sizeof(uintptr_t), 0);
    int ud = lua_gettop(L);
    lua_newtable(L);
    if (!setters.empty()) {
        static lua_CFunction setter_func = +[](lua_State* L) {
            lua_pushvalue(L, 2);
            if (LUA_TNIL == lua_gettable(L, lua_upvalueindex(1))) {
                return luaL_error(L, "%s.%s is invalid.", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, 2));
            }
            lua_pushvalue(L, 3);
            lua_call(L, 1, 0);
            return 0;
        };
        lua_createtable(L, 0, (int)setters.size());
        lua_pushvalue(L, ud);
        set_table(L, setters, 1);
        lua_pushstring(L, name);
        lua_pushcclosure(L, setter_func, 2);
        lua_setfield(L, -2, "__newindex");
    }
    if (!funcs.empty()) {
        lua_createtable(L, 0, (int)funcs.size());
        lua_pushvalue(L, ud);
        set_table(L, funcs, 1);
        lua_newtable(L);
    }
    static lua_CFunction getter_func = +[](lua_State* L) {
        lua_pushvalue(L, 2);
        if (LUA_TNIL == lua_gettable(L, lua_upvalueindex(1))) {
            return luaL_error(L, "%s.%s is invalid.", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, 2));
        }
        lua_call(L, 0, 1);
        return 1;
    };
    lua_createtable(L, 0, (int)getters.size());
    lua_pushvalue(L, ud);
    set_table(L, getters, 1);
    lua_pushstring(L, name);
    lua_pushcclosure(L, getter_func, 2);
    lua_setfield(L, -2, "__index");
    if (!funcs.empty()) {
        lua_setmetatable(L, -2);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
}

void flags_gen(lua_State* L, const char* name) {
    lua_pushstring(L, name);
    lua_pushcclosure(L, make_flags, 2);
}

void init(lua_State* L) {
    luaopen_string(L);
    lua_getfield(L, -1, "format");
    str_format = lua_tocfunction(L, -1);
    lua_pop(L, 2);
}

}  // namespace Neko::ImGuiWrap::util

#endif

namespace Neko {

namespace ImGuiWrap {

static int Begin(lua_State* L) {
    auto* label = LuaGet<const char*>(L, 1);
    ImGuiWindowFlags flags = 0;
    bool open = true;
    bool has_open = false;
    if (lua_gettop(L) > 1) {
        open = LuaGet<bool>(L, 2);
        has_open = true;
    }
    if (lua_gettop(L) > 2) {
        flags = LuaGet<int>(L, 3);
    }
    bool res = ImGui::Begin(label, has_open ? &open : nullptr, flags);
    lua_pushboolean(L, res);
    if (has_open) lua_pushboolean(L, open);
    return has_open ? 2 : 1;
}

static int BeginTabBar(lua_State* L) {
    auto* label = LuaGet<const char*>(L, 1);
    bool res = ImGui::BeginTabBar(label);
    lua_pushboolean(L, res);
    return 1;
}

static int BeginTabItem(lua_State* L) {
    auto* label = LuaGet<const char*>(L, 1);
    bool res = ImGui::BeginTabItem(label);
    lua_pushboolean(L, res);
    return 1;
}

static int Button(lua_State* L) {
    auto* label = LuaGet<const char*>(L, 1);
    ImVec2 size(0, 0);
    if (lua_gettop(L) > 2) {
        size.x = LuaGet<float>(L, 2);
        size.y = LuaGet<float>(L, 3);
    }
    bool clicked = ImGui::Button(label, size);
    lua_pushboolean(L, clicked);
    return 1;
}

static int SameLine(lua_State* L) {
    float pos_x = 0;
    if (lua_gettop(L) > 0) {
        pos_x = LuaGet<float>(L, 1);
    }
    ImGui::SameLine(pos_x);
    return 0;
}

static int Text(lua_State* L) {
    const char* fmt = util::format(L, 1);
    ImGui::Text("%s", fmt);
    return 0;
}

static int TextUnformatted(lua_State* L) {
    auto* text = LuaGet<const char*>(L, 1);
    ImGui::TextUnformatted(text);
    return 0;
}

static int Checkbox(lua_State* L) {
    auto* label = LuaGet<const char*>(L, 1);
    bool b = LuaGet<bool>(L, 2);
    bool clicked = ImGui::Checkbox(label, &b);
    lua_pushboolean(L, clicked);
    lua_pushboolean(L, b);
    return 2;
}

bool IsCapturedEvent() {
    return ImGui::IsAnyItemHovered() ||                            //
           ImGui::IsAnyItemFocused() ||                            //
           ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||  //
           ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
}

static int InputText(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto _strbuf = util::strbuf_get(L, 2);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputText(label, _strbuf->data, _strbuf->size, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int InputTextEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto _strbuf = util::strbuf_get(L, 2);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiInputTextFlags_None));
    util::input_context _ctx{L, 4};
    auto _top = lua_gettop(L);
    auto&& _retval = ImGui::InputText(label, _strbuf->data, _strbuf->size, flags, util::input_callback, &_ctx);
    lua_pushboolean(L, _retval);
    if (lua_gettop(L) != _top + 1) {
        lua_pop(L, 1);
        lua_error(L);
    }
    return 1;
}

static int InputTextMultiline(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto _strbuf = util::strbuf_get(L, 2);
    auto&& _retval = ImGui::InputTextMultiline(label, _strbuf->data, _strbuf->size);
    lua_pushboolean(L, _retval);
    return 1;
}

static int InputTextMultilineEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto _strbuf = util::strbuf_get(L, 2);
    auto size = ImVec2{
            (float)luaL_optnumber(L, 3, 0),
            (float)luaL_optnumber(L, 4, 0),
    };
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 5, lua_Integer(ImGuiInputTextFlags_None));
    util::input_context _ctx{L, 6};
    auto _top = lua_gettop(L);
    auto&& _retval = ImGui::InputTextMultiline(label, _strbuf->data, _strbuf->size, size, flags, util::input_callback, &_ctx);
    lua_pushboolean(L, _retval);
    if (lua_gettop(L) != _top + 1) {
        lua_pop(L, 1);
        lua_error(L);
    }
    return 1;
}

static int InputTextWithHint(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto hint = luaL_checkstring(L, 2);
    auto _strbuf = util::strbuf_get(L, 3);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputTextWithHint(label, hint, _strbuf->data, _strbuf->size, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int InputTextWithHintEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto hint = luaL_checkstring(L, 2);
    auto _strbuf = util::strbuf_get(L, 3);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiInputTextFlags_None));
    util::input_context _ctx{L, 5};
    auto _top = lua_gettop(L);
    auto&& _retval = ImGui::InputTextWithHint(label, hint, _strbuf->data, _strbuf->size, flags, util::input_callback, &_ctx);
    lua_pushboolean(L, _retval);
    if (lua_gettop(L) != _top + 1) {
        lua_pop(L, 1);
        lua_error(L);
    }
    return 1;
}

namespace wrap_ImGuiInputTextCallbackData {

static int DeleteChars(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    auto pos = (int)luaL_checkinteger(L, 1);
    auto bytes_count = (int)luaL_checkinteger(L, 2);
    OBJ.DeleteChars(pos, bytes_count);
    return 0;
}

static int InsertChars(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    auto pos = (int)luaL_checkinteger(L, 1);
    auto text = luaL_checkstring(L, 2);
    auto text_end = luaL_optstring(L, 3, NULL);
    OBJ.InsertChars(pos, text, text_end);
    return 0;
}

static int SelectAll(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.SelectAll();
    return 0;
}

static int ClearSelection(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.ClearSelection();
    return 0;
}

static int HasSelection(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.HasSelection();
    lua_pushboolean(L, _retval);
    return 1;
}

struct EventFlag {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.EventFlag);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.EventFlag = (ImGuiInputTextFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct Flags {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.Flags);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.Flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct UserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.UserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.UserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

struct EventChar {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.EventChar);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.EventChar = (ImWchar)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct EventKey {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.EventKey);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.EventKey = (ImGuiKey)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct BufTextLen {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.BufTextLen);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BufTextLen = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct BufSize {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.BufSize);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BufSize = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct BufDirty {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.BufDirty);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BufDirty = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct CursorPos {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.CursorPos);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.CursorPos = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct SelectionStart {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.SelectionStart);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.SelectionStart = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct SelectionEnd {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.SelectionEnd);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.SelectionEnd = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

static luaL_Reg funcs[] = {
        {"DeleteChars", DeleteChars}, {"InsertChars", InsertChars}, {"SelectAll", SelectAll}, {"ClearSelection", ClearSelection}, {"HasSelection", HasSelection},
};

static luaL_Reg setters[] = {
        {"EventFlag", EventFlag::setter},       {"Flags", Flags::setter},     {"UserData", UserData::setter}, {"EventChar", EventChar::setter}, {"EventKey", EventKey::setter},
        {"BufTextLen", BufTextLen::setter},     {"BufSize", BufSize::setter}, {"BufDirty", BufDirty::setter}, {"CursorPos", CursorPos::setter}, {"SelectionStart", SelectionStart::setter},
        {"SelectionEnd", SelectionEnd::setter},
};

static luaL_Reg getters[] = {
        {"EventFlag", EventFlag::getter},       {"Flags", Flags::getter},     {"UserData", UserData::getter}, {"EventChar", EventChar::getter}, {"EventKey", EventKey::getter},
        {"BufTextLen", BufTextLen::getter},     {"BufSize", BufSize::getter}, {"BufDirty", BufDirty::getter}, {"CursorPos", CursorPos::getter}, {"SelectionStart", SelectionStart::getter},
        {"SelectionEnd", SelectionEnd::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiInputTextCallbackData& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiInputTextCallbackData**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiInputTextCallbackData", funcs, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiInputTextCallbackData

static int StringBuf(lua_State* L) {
    util::strbuf_create(L, 1);
    return 1;
}

static int BindUtils(lua_State* L) {
    util::init(L);

    return 1;
}

static int BindFlags(lua_State* L) {

#define X(prefix, name) {#name, prefix##_##name}

    static util::TableInteger InputTextFlags[] = {
            X(ImGuiInputTextFlags, None),
            X(ImGuiInputTextFlags, CharsDecimal),
            X(ImGuiInputTextFlags, CharsHexadecimal),
            X(ImGuiInputTextFlags, CharsScientific),
            X(ImGuiInputTextFlags, CharsUppercase),
            X(ImGuiInputTextFlags, CharsNoBlank),
            X(ImGuiInputTextFlags, AllowTabInput),
            X(ImGuiInputTextFlags, EnterReturnsTrue),
            X(ImGuiInputTextFlags, EscapeClearsAll),
            X(ImGuiInputTextFlags, CtrlEnterForNewLine),
            X(ImGuiInputTextFlags, ReadOnly),
            X(ImGuiInputTextFlags, Password),
            X(ImGuiInputTextFlags, AlwaysOverwrite),
            X(ImGuiInputTextFlags, AutoSelectAll),
            X(ImGuiInputTextFlags, ParseEmptyRefVal),
            X(ImGuiInputTextFlags, DisplayEmptyRefVal),
            X(ImGuiInputTextFlags, NoHorizontalScroll),
            X(ImGuiInputTextFlags, NoUndoRedo),
            X(ImGuiInputTextFlags, CallbackCompletion),
            X(ImGuiInputTextFlags, CallbackHistory),
            X(ImGuiInputTextFlags, CallbackAlways),
            X(ImGuiInputTextFlags, CallbackCharFilter),
            X(ImGuiInputTextFlags, CallbackResize),
            X(ImGuiInputTextFlags, CallbackEdit),
    };

#undef X

#define GEN_FLAGS(name)                  \
    {                                    \
        #name, +[](lua_State* L) {       \
            util::create_table(L, name); \
            util::flags_gen(L, #name);   \
        }                                \
    }
    static util::TableAny flags[] = {GEN_FLAGS(InputTextFlags)};
#undef GEN_FLAGS

    util::set_table(L, flags);

    wrap_ImGuiInputTextCallbackData::init(L);

    return 1;
}

void ImGuiRender::imgui_init() {
    PROFILE_FUNC();

    ImGui::SetAllocatorFunctions(+[](size_t sz, void* user_data) { return mem_alloc(sz); }, +[](void* ptr, void* user_data) { return mem_free(ptr); });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(the<Window>().glfwWindow(), true);
    ImGui_ImplOpenGL3_Init();

#if defined(DEBUG)
    if (the<CL>().state.default_font.len > 0) {
        auto& io = ImGui::GetIO();

        ImFontConfig config;
        // config.PixelSnapH = 1;

        String ttf_file;
        the<VFS>().read_entire_file(&ttf_file, the<CL>().state.default_font.cstr());
        io.Fonts->AddFontFromMemoryTTF(ttf_file.data, ttf_file.len, 16.0f, &config, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    }
#endif

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

    the<CL>().devui_vp = ImGui::GetMainViewport()->ID;

    auto type = BUILD_TYPE_SUB(ImGuiRender, "imgui")
                        .Method("AlignTextToFramePadding", &ImGui::AlignTextToFramePadding)          //
                        .Method("End", &ImGui::End)                                                  //
                        .Method("EndTabBar", &ImGui::EndTabBar)                                      //
                        .Method("EndTabItem", &ImGui::EndTabItem)                                    //
                        .Method("Separator", &ImGui::Separator)                                      //
                        .Method("SeparatorText", &ImGui::SeparatorText)                              //
                        .Method("IsItemHovered", &ImGui::IsItemHovered)                              //
                        .Method("Indent", &ImGui::Indent)                                            //
                        .Method("Unindent", &ImGui::Unindent)                                        //
                        .Method("IsMouseDown", [](int b) -> bool { return ImGui::IsMouseDown(b); })  //
                        .Method("IsCapturedEvent", &IsCapturedEvent)                                 //
                        .CClosure({{"Begin", Begin},                                                 //
                                   {"BeginTabBar", BeginTabBar},
                                   {"BeginTabItem", BeginTabItem},
                                   {"Button", Button},
                                   {"Text", Text},
                                   {"TextUnformatted", TextUnformatted},
                                   {"Checkbox", Checkbox},
                                   {"SameLine", SameLine},
                                   {"InputText", InputText},
                                   {"InputTextEx", InputTextEx},
                                   {"InputTextMultiline", InputTextMultiline},
                                   {"InputTextMultilineEx", InputTextMultilineEx},
                                   {"InputTextWithHint", InputTextWithHint},
                                   {"InputTextWithHintEx", InputTextWithHintEx},
                                   {"StringBuf", StringBuf}})
                        .CustomBind("Utils", &BindUtils)
                        //.CustomBind("Flags", &BindFlags)
                        .Build();
}

void ImGuiRender::imgui_fini() {

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

int ImGuiRender::imgui_draw_pre() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    return 0;
}

void ImGuiRender::imgui_draw_post() {
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

}  // namespace ImGuiWrap
}  // namespace Neko