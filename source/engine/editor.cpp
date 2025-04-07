
#include "editor.h"

#include <inttypes.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <typeindex>
#include <vector>

#include "engine/asset.h"
#include "base/common/base.hpp"
#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/graphics.h"
#include "engine/window.h"
#include "engine/ecs/entitybase.hpp"
#include "engine/edit.h"
#include "base/common/profiler.hpp"
#include "engine/component.h"
#include "engine/scripting/lua_util.h"

using namespace Neko::ImGuiWrap;

template <>
struct std::formatter<Neko::LuaInspector*> : std::formatter<void*> {
    auto format(Neko::LuaInspector* ptr, std::format_context& ctx) const { return std::formatter<void*>::format(static_cast<void*>(ptr), ctx); }
};

static int __luainspector_echo(lua_State* L) {
    Neko::LuaInspector* m = *static_cast<Neko::LuaInspector**>(lua_touserdata(L, lua_upvalueindex(1)));
    if (m) m->print(luaL_checkstring(L, 1), Logger::Level::INFO);
    return 0;
}

static int __luainspector_gc(lua_State* L) {
    Neko::LuaInspector* m = *static_cast<Neko::LuaInspector**>(lua_touserdata(L, 1));
    if (m) {
        m->variable_pool_free();
        m->setL(0x0);
    }

    LOG_INFO("luainspector __gc {}", m);
    return 0;
}

namespace Neko {

const size_t MAX_LOG_ENTRIES = 50;       // 内存中最大日志条数
const size_t MAX_VISIBLE_LOGS = 10;      // 屏幕上最多显示10条日志
const size_t MAX_MESSAGE_LENGTH = 1024;  // 单条日志最大长度
const float LOG_PADDING = 8.0f;          // 日志之间的间距
const float LOG_MARGIN_X = 15.0f;        // 左侧边距
const float LOG_MARGIN_Y = 15.0f;        // 顶部边距
const float ENTRY_ANIM_DURATION = 0.3f;  // 入场动画持续时间

struct ConsoleLogEntry {
    std::string message;
    Logger::Level level;
    float alpha;         // 当前透明度 (0.0-1.0)
    float duration;      // 总显示时间
    float fadeDuration;  // 淡出持续时间
    float slideOffset;   // 滑动动画偏移量
    float scale;         // 缩放动画 (1.0为正常大小)
    std::chrono::steady_clock::time_point startTime;
    float height;  // 该条日志的显示高度
    bool isNew;    // 是否是新添加的日志

    ConsoleLogEntry(std::string_view msg, Logger::Level lvl, float dur, float fadeDur)
        : message(msg.substr(0, MAX_MESSAGE_LENGTH)),
          level(lvl),
          alpha(0.0f),
          duration(dur),
          fadeDuration(fadeDur),
          slideOffset(20.0f),
          scale(1.05f),
          startTime(std::chrono::steady_clock::now()),
          height(0.0f),
          isNew(true) {}
};

std::vector<ConsoleLogEntry> consolelogEntries;
std::mutex logMutex;

void ConsoleLogAdd(std::string_view message, Logger::Level level) noexcept {
    try {
        if (message.empty()) return;
        std::lock_guard<std::mutex> lock(logMutex);
        if (consolelogEntries.size() >= MAX_LOG_ENTRIES) {
            consolelogEntries.erase(std::remove_if(consolelogEntries.begin(), consolelogEntries.end(), [](const ConsoleLogEntry& entry) { return entry.alpha <= 0.0f; }), consolelogEntries.end());
            if (consolelogEntries.size() >= MAX_LOG_ENTRIES) {
                consolelogEntries.erase(consolelogEntries.begin());
            }
        }
        consolelogEntries.emplace_back(message, level, 3.0f, 0.8f);
    } catch (...) {
    }
}

float SmoothStep(float edge0, float edge1, float x) {
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

float CalculateTextHeight(const std::string& text, float wrapWidth) {
    if (text.empty()) return 0.0f;
    ImFont* font = ImGui::GetFont();
    float fontSize = ImGui::GetFontSize();
    if (wrapWidth <= 0) {
        return fontSize;
    }
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, wrapWidth, text.c_str());
    return textSize.y;
}

void RenderConsoleLogs() noexcept {
    try {
        std::lock_guard<std::mutex> lock(logMutex);

        const ImVec2 initialPos(LOG_MARGIN_X, LOG_MARGIN_Y);
        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
        auto now = std::chrono::steady_clock::now();

        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        float wrapWidth = screenSize.x - 2 * LOG_MARGIN_X;

        ImVec2 currentPos = initialPos;
        size_t visibleCount = 0;

        for (auto it = consolelogEntries.rbegin(); it != consolelogEntries.rend() && visibleCount < MAX_VISIBLE_LOGS; ++it) {
            auto& entry = *it;
            float elapsed = std::chrono::duration<float>(now - entry.startTime).count();

            // 处理入场动画
            if (entry.isNew) {
                float animProgress = std::min(elapsed / ENTRY_ANIM_DURATION, 1.0f);
                entry.alpha = SmoothStep(0.0f, 1.0f, animProgress);
                entry.slideOffset = 20.0f * (1.0f - animProgress);
                entry.scale = 1.0f + 0.05f * (1.0f - animProgress);

                if (animProgress >= 1.0f) {
                    entry.isNew = false;
                }
            }

            // 处理淡出动画
            if (elapsed > entry.duration) {
                float fadeProgress = (elapsed - entry.duration) / entry.fadeDuration;
                entry.alpha = 1.0f - SmoothStep(0.0f, 1.0f, fadeProgress);
            }

            if (entry.alpha <= 0.0f) {
                continue;
            }

            if (entry.height <= 0.0f) {
                entry.height = CalculateTextHeight(entry.message, wrapWidth) + LOG_PADDING;
            }

            if (currentPos.y + entry.height > screenSize.y) {
                break;
            }

            ImVec4 baseColor;
            switch (entry.level) {
                case Logger::Level::WARNING:
                    baseColor = ImVec4(1.0f, 0.8f, 0.0f, entry.alpha);
                    break;
                case Logger::Level::ERR:
                    baseColor = ImVec4(1.0f, 0.3f, 0.3f, entry.alpha);
                    break;
                case Logger::Level::INFO:
                    baseColor = ImVec4(0.3f, 0.7f, 1.0f, entry.alpha);
                    break;
                case Logger::Level::TRACE:
                    baseColor = ImVec4(1.0f, 1.0f, 1.0f, entry.alpha);
                    break;
                default:
                    baseColor = ImVec4(1.0f, 1.0f, 1.0f, entry.alpha);
                    break;
            }

            ImVec2 textPos = currentPos;
            textPos.x += entry.slideOffset;

            if (entry.alpha > 0.3f) {
                ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize() * entry.scale, FLT_MAX, wrapWidth, entry.message.c_str());

                ImVec4 bgColor = baseColor;
                bgColor.w *= 0.15f;  // 背景透明度较低
                draw_list->AddRectFilled(ImVec2(textPos.x - 4.0f, textPos.y - 2.0f), ImVec2(textPos.x + textSize.x + 4.0f, textPos.y + entry.height - 2.0f), ImGui::ColorConvertFloat4ToU32(bgColor),
                                         4.0f  // 圆角
                );
            }

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, entry.alpha);
            Neko::ImGuiWrap::OutlineText(draw_list, textPos, "%s", entry.message.c_str());
            ImGui::PopStyleVar();

            currentPos.y += entry.height;
            visibleCount++;
        }

        consolelogEntries.erase(std::remove_if(consolelogEntries.begin(), consolelogEntries.end(), [](const ConsoleLogEntry& entry) { return entry.alpha <= 0.0f; }), consolelogEntries.end());
    } catch (...) {
    }
}

std::string LuaInspector::Hints::clean_table_list(const std::string& str) {
    std::string ret;
    bool got_dot = false, got_white = false;
    std::size_t whitespace_start = 0u;
    for (std::size_t i = 0u; i < str.size(); ++i) {
        const char c = str[i] == ':' ? '.' : str[i];
        if (!got_white && c == ' ') {
            got_white = true;
            whitespace_start = i;
        }
        if (c == '.' && got_white) {
            for (std::size_t j = 0u; j < (i - whitespace_start); ++j) ret.erase(--ret.end());
        }
        if (c != ' ') got_white = false;
        if (c != ' ' || !got_dot) ret += c;
        if (c == '.') got_dot = true;
        if (c != '.' && c != ' ') got_dot = false;
    }

    const std::string specials = "()[]{}\"'+-=/*^%#~,";
    for (std::size_t i = 0u; i < specials.size(); ++i) std::replace(ret.begin(), ret.end(), specials[i], ' ');

    ret = ret.substr(ret.find_last_of(' ') + 1u);
    return ret;
}

void LuaInspector::Hints::prepare_hints(lua_State* L, std::string str, std::string& last) {
    str = clean_table_list(str);

    std::vector<std::string> tables;
    std::size_t begin = 0;
    for (std::size_t i = 0u; i < str.size(); ++i) {
        if (str[i] == '.') {
            tables.push_back(str.substr(begin, i - begin));
            begin = i + 1;
        }
    }
    last = str.substr(begin);

    lua_pushglobaltable(L);
    for (std::size_t i = 0u; i < tables.size(); ++i) {
        if (lua_type(L, -1) != LUA_TTABLE) {
            lua_getmetatable(L, -1);
        }

        if (lua_type(L, -1) != LUA_TTABLE && !luaL_getmetafield(L, -1, "__index") && !lua_getmetatable(L, -1)) break;
        if (lua_type(L, -1) != LUA_TTABLE) break;  // no
        lua_pushlstring(L, tables[i].c_str(), tables[i].size());
        lua_gettable(L, -2);
    }
}

bool LuaInspector::Hints::collect_hints_recurse(lua_State* L, std::vector<std::string>& possible, const std::string& last, bool usehidden, unsigned left) {
    if (left == 0u) return true;

    const bool skip_under_score = last.empty() && !usehidden;

    lua_pushnil(L);
    while (lua_next(L, -2)) {
        std::size_t keylen;
        const char* key;
        bool match = true;
        lua_pop(L, 1);
        lua_pushvalue(L, -1);  // for lua_next
        key = lua_tolstring(L, -1, &keylen);
        if (last.size() > keylen) {
            lua_pop(L, 1);
            continue;
        }
        for (std::size_t i = 0u; i < last.size(); ++i)
            if (key[i] != last[i]) match = false;
        if (match && (!skip_under_score || key[0] != '_')) possible.push_back(key);
        lua_pop(L, 1);  //
    }

    // Check whether the table itself has an index for linking elements
    if (luaL_getmetafield(L, -1, "__index")) {
        if (lua_istable(L, -1)) return collect_hints_recurse(L, possible, last, usehidden, left - 1);
        lua_pop(L, 1);  // pop
    }
    lua_pop(L, 1);  // pop table
    return true;
}

// Replace the value at the top of the stack with the __index TABLE from the metatable
bool LuaInspector::Hints::try_replace_with_metaindex(lua_State* L) {
    if (!luaL_getmetafield(L, -1, "__index")) return false;

    if (lua_type(L, -1) != LUA_TTABLE) {
        lua_pop(L, 2);  // pop value and key
        return false;
    }

    lua_insert(L, -2);  // move table under value
    lua_pop(L, 1);      // pop value
    return true;
}

bool LuaInspector::Hints::collect_hints(lua_State* L, std::vector<std::string>& possible, const std::string& last, bool usehidden) {
    if (lua_type(L, -1) != LUA_TTABLE && !Hints::try_replace_with_metaindex(L)) return false;
    // table so just collect on it
    return collect_hints_recurse(L, possible, last, usehidden, 10u);
}

std::string LuaInspector::Hints::common_prefix(const std::vector<std::string>& possible) {
    std::string ret;
    std::size_t maxindex = 1000000000u;
    for (std::size_t i = 0u; i < possible.size(); ++i) maxindex = std::min(maxindex, possible[i].size());
    for (std::size_t checking = 0u; checking < maxindex; ++checking) {
        const char c = possible[0u][checking];
        for (std::size_t i = 1u; i < possible.size(); ++i)
            if (c != possible[i][checking]) {
                checking = maxindex;
                break;
            }
        if (checking != maxindex) ret += c;
    }
    return ret;
}

}  // namespace Neko

void Neko::LuaInspector::print_luastack(int first, int last, Logger::Level logtype) {
    std::stringstream ss;
    for (int i = first; i <= last; ++i) {
        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                ss << lua_tostring(L, i);
                break;
            case LUA_TSTRING:
                ss << "'" << lua_tostring(L, i) << "'";
                break;
            case LUA_TBOOLEAN:
                ss << (lua_toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNIL:
                ss << "nil";
                break;
            default:
                ss << luaL_typename(L, i) << ": " << lua_topointer(L, i);
                break;
        }
        ss << ' ';
    }
    print(ss.str(), logtype);
}

bool Neko::LuaInspector::try_eval(std::string m_buffcmd, bool addreturn) {
    if (addreturn) {
        const std::string code = "return " + m_buffcmd;
        if (LUA_OK == luaL_loadstring(L, code.c_str())) {
            return true;
        } else {
            lua_pop(L, 1);  // pop error
            return false;
        }
    } else {
        return LUA_OK == luaL_loadstring(L, m_buffcmd.c_str());
    }
}

// 避免使用非字符串调用时错误
static inline std::string adjust_error_msg(lua_State* L, int idx) {
    const int t = lua_type(L, idx);
    if (t == LUA_TSTRING) return lua_tostring(L, idx);
    return std::string("(non string error value - ") + lua_typename(L, t) + ")";
}

void Neko::LuaInspector::setL(lua_State* L) {
    this->L = L;

    if (!L) return;

    // Neko::LuaInspector** ptr = static_cast<Neko::LuaInspector**>(lua_newuserdata(L, sizeof(Neko::LuaInspector*)));
    //(*ptr) = this;

    // luaL_newmetatable(L, kMetaname);  // table
    // lua_pushliteral(L, "__gc");
    // lua_pushcfunction(L, &__luainspector_gc);
    // lua_settable(L, -3);  // table[gc]=ConsoleModel_gc
    // lua_setmetatable(L, -2);

    // lua_pushlightuserdata(L, __neko_lua_inspector_lightkey());
    // lua_pushvalue(L, -2);
    // lua_settable(L, LUA_REGISTRYINDEX);

    // lua_pushcclosure(L, &__luainspector_echo, 1);
    // lua_setglobal(L, "echo");
}

std::string Neko::LuaInspector::read_history(int change) {
    const bool was_promp = static_cast<std::size_t>(m_hindex) == m_history.size();

    m_hindex += change;
    m_hindex = std::max<std::size_t>(m_hindex, 0);
    m_hindex = std::min<std::size_t>(m_hindex, m_history.size());

    if (static_cast<std::size_t>(m_hindex) == m_history.size()) {
        return m_history[m_hindex - 1];
    } else {
        return m_history[m_hindex];
    }
}

std::string Neko::LuaInspector::try_complete(std::string inputbuffer) {
    if (!L) {
        print("Lua state pointer is NULL, no completion available", Logger::Level::ERR);
        return inputbuffer;
    }

    std::vector<std::string> possible;  // possible match
    std::string last;

    const std::string lastbeg = inputbuffer;
    Hints::prepare_hints(L, lastbeg, last);
    if (!Hints::collect_hints(L, possible, last, false)) {
        lua_pushglobaltable(L);
        Hints::collect_hints(L, possible, last, false);
    }

    lua_settop(L, 0);  // Pop all

    if (possible.size() > 1u) {
        const std::string common_prefix = Hints::common_prefix(possible);
        if (common_prefix.empty() || common_prefix.size() <= last.size()) {
            std::string msg = possible[0];
            for (std::size_t i = 1u; i < possible.size(); ++i) msg += " " + possible[i];
            print(msg, Logger::Level::INFO);
            m_current_autocomplete_strings = possible;
        } else {
            const std::string added = common_prefix.substr(last.size());
            inputbuffer = lastbeg + added;
            m_current_autocomplete_strings.clear();
        }
    } else if (possible.size() == 1) {
        const std::string added = possible[0].substr(last.size());
        inputbuffer = lastbeg + added;
        m_current_autocomplete_strings.clear();
    }
    return inputbuffer;
}

void Neko::LuaInspector::print(std::string msg, Logger::Level logtype) {
    if (messageLog.size() >= 64) {
        messageLog.pop_front();
    }
    messageLog.push_back(msg);
}

// void Neko::luainspector::set_print_eval_prettifier(lua_State* L) {
//     if (lua_gettop(L) == 0) return;

//     const int t = lua_type(L, -1);
//     if (!(t == LUA_TFUNCTION || t == LUA_TNIL)) return;

//     lua_pushlightuserdata(L, __neko_lua_inspector_print_func_lightkey());
//     lua_insert(L, -2);
//     lua_settable(L, LUA_REGISTRYINDEX);
// }

// void Neko::luainspector::get_print_eval_prettifier(lua_State* L) const {
//     lua_pushlightuserdata(L, __neko_lua_inspector_print_func_lightkey());
//     lua_gettable(L, LUA_REGISTRYINDEX);
// }

// bool Neko::luainspector::apply_prettifier(int index) {
//     get_print_eval_prettifier(L);
//     if (lua_type(L, -1) == LUA_TNIL) {
//         lua_pop(L, 1);
//         return false;
//     }

//     assert(lua_type(L, -1) == LUA_TFUNCTION);
//     lua_pushvalue(L, index);
//     if (LUA_OK == lua_pcall(L, 1, 1, 0)) {
//         lua_remove(L, index);
//         lua_insert(L, index);
//         return true;
//     } else {
//         print_line(adjust_error_msg(L, -1), LUACON_LOG_TYPE_ERROR);
//         lua_pop(L, 1);
//         return false;
//     }
//     return true;
// }

int command_line_callback_st(ImGuiInputTextCallbackData* data) {
    command_line_input_callback_UserData* user_data = (command_line_input_callback_UserData*)data->UserData;
    return reinterpret_cast<Neko::LuaInspector*>(user_data->luainspector_ptr)->command_line_input_callback(data);
}

int Neko::LuaInspector::command_line_input_callback(ImGuiInputTextCallbackData* data) {
    command_line_input_callback_UserData* user_data = (command_line_input_callback_UserData*)data->UserData;

    auto paste_buffer = [data](auto begin, auto end, auto buffer_shift) {
        Neko::copy(begin, end, data->Buf + buffer_shift, data->Buf + data->BufSize - 1);
        data->BufTextLen = std::min(static_cast<int>(std::distance(begin, end) + buffer_shift), data->BufSize - 1);
        data->Buf[data->BufTextLen] = '\0';
        data->BufDirty = true;
        data->SelectionStart = data->SelectionEnd;
        data->CursorPos = data->BufTextLen;
    };

    const char* command_beg = nullptr;
    command_beg = Neko::find_terminating_word(cmd.data(), cmd.data() + cmd.size(), [this](std::string_view sv) { return sv[0] == ' ' ? 1 : 0; });

    if (data->EventKey == ImGuiKey_Tab) {
        std::string complete = this->try_complete(cmd);
        if (!complete.empty()) {
            paste_buffer(complete.data(), complete.data() + complete.size(), command_beg - cmd.data());
            LOG_INFO("{}", complete.c_str());
        }
    }
    if (data->EventKey == ImGuiKey_UpArrow) {
        cmd2 = this->read_history(-1);
        paste_buffer(cmd2.data(), cmd2.data() + cmd2.size(), command_beg - cmd.data());
        LOG_INFO("h:{}", cmd2.c_str());
    }
    if (data->EventKey == ImGuiKey_DownArrow) {
        cmd2 = this->read_history(1);
        paste_buffer(cmd2.data(), cmd2.data() + cmd2.size(), command_beg - cmd.data());
        LOG_INFO("h:{}", cmd2.c_str());
    }

    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string* str = user_data->Str;
        neko_assert(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    } else if (user_data->ChainCallback) {
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool Neko::LuaInspector::command_line_input(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    command_line_input_callback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    cb_user_data.luainspector_ptr = this;
    return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, command_line_callback_st, &cb_user_data);
}

void Neko::LuaInspector::show_autocomplete() noexcept {
    constexpr ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                                               ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings;

    if ((m_input_text_id == ImGui::GetActiveID() || m_should_take_focus) && (!m_current_autocomplete_strings.empty())) {

        ImGui::SetNextWindowBgAlpha(0.9f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::SetNextWindowFocus();

        ImVec2 auto_complete_pos = ImGui::GetItemRectMin();

        auto_complete_pos.y = ImGui::GetItemRectMax().y;

        ImVec2 auto_complete_max_size = ImGui::GetItemRectSize();
        auto_complete_max_size.y = -1.f;
        ImGui::SetNextWindowPos(auto_complete_pos);
        ImGui::SetNextWindowSizeConstraints({0.f, 0.f}, auto_complete_max_size);
        if (ImGui::Begin("##terminal:auto_complete", nullptr, overlay_flags)) {
            ImGui::SetActiveID(m_input_text_id, ImGui::GetCurrentWindow());

            auto print_separator = [this]() {
                ImGui::SameLine(0.f, 0.f);
                int pop = try_push_style(ImGuiCol_Text, ImVec4{0.000f, 0.000f, 0.000f, 0.392f});
                ImGui::TextUnformatted(m_autocomlete_separator.data(), m_autocomlete_separator.data() + m_autocomlete_separator.size());
                ImGui::PopStyleColor(pop);
                ImGui::SameLine(0.f, 0.f);
            };

            int max_displayable_sv = 0;
            float separator_length = ImGui::CalcTextSize(m_autocomlete_separator.data(), m_autocomlete_separator.data() + m_autocomlete_separator.size()).x;
            float total_text_length = ImGui::CalcTextSize("...").x;

            std::vector<std::string_view> autocomplete_text;

            if (m_current_autocomplete_strings.empty()) {
            } else {
                autocomplete_text.reserve(m_current_autocomplete_strings.size());
                for (const std::string& str : m_current_autocomplete_strings) {
                    autocomplete_text.emplace_back(str);
                }
            }

            for (const std::string_view& sv : autocomplete_text) {
                float t_len = ImGui::CalcTextSize(sv.data(), sv.data() + sv.size()).x + separator_length;
                if (t_len + total_text_length < auto_complete_max_size.x) {
                    total_text_length += t_len;
                    ++max_displayable_sv;
                } else {
                    break;
                }
            }

            std::string_view last;
            int pop_count = 0;

            if (max_displayable_sv != 0) {
                const std::string_view& first = autocomplete_text[0];
                pop_count += try_push_style(ImGuiCol_Text, ImVec4{1.000f, 1.000f, 1.000f, 1.000f});
                ImGui::TextUnformatted(first.data(), first.data() + first.size());
                pop_count += try_push_style(ImGuiCol_Text, ImVec4{0.500f, 0.450f, 0.450f, 1.000f});
                for (int i = 1; i < max_displayable_sv; ++i) {
                    const std::string_view vs = autocomplete_text[i];
                    print_separator();
                    ImGui::TextUnformatted(vs.data(), vs.data() + vs.size());
                }
                ImGui::PopStyleColor(pop_count);
                if (max_displayable_sv < static_cast<long>(autocomplete_text.size())) last = autocomplete_text[max_displayable_sv];
            }

            pop_count = 0;
            if (max_displayable_sv < static_cast<long>(autocomplete_text.size())) {

                if (max_displayable_sv == 0) {
                    last = autocomplete_text.front();
                    pop_count += try_push_style(ImGuiCol_Text, ImVec4{1.000f, 1.000f, 1.000f, 1.000f});
                    total_text_length -= separator_length;
                } else {
                    pop_count += try_push_style(ImGuiCol_Text, ImVec4{0.500f, 0.450f, 0.450f, 1.000f});
                    print_separator();
                }

                std::vector<char> buf;
                buf.resize(last.size() + 4);
                std::copy(last.begin(), last.end(), buf.begin());
                std::fill(buf.begin() + last.size(), buf.end(), '.');
                auto size = static_cast<unsigned>(last.size() + 3);
                while (size >= 4 && total_text_length + ImGui::CalcTextSize(buf.data(), buf.data() + size).x >= auto_complete_max_size.x) {
                    buf[size - 4] = '.';
                    --size;
                }
                while (size != 0 && total_text_length + ImGui::CalcTextSize(buf.data(), buf.data() + size).x >= auto_complete_max_size.x) {
                    --size;
                }
                ImGui::TextUnformatted(buf.data(), buf.data() + size);
                ImGui::PopStyleColor(pop_count);
            }

            if (ImGui::IsKeyDown(ImGuiKey_Enter)) {
                cmd.clear();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

Neko::LuaInspector::LuaInspector() {
    callbackId = Logger::getInstance()->registerCallback([this](const std::string& msg, const Logger::Level& level) { ConsoleLogAdd(msg, level); });
}

Neko::LuaInspector::~LuaInspector() { Logger::getInstance()->unregisterCallback(callbackId); }

void Neko::LuaInspector::console_draw(bool& textbox_react) noexcept {

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    const float TEXT_BASE_HIGHT = ImGui::CalcTextSize("A").y;

    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 80 - TEXT_BASE_HIGHT * 2);
    if (ImGui::BeginChild("##LOG_INFO", size)) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

        for (auto& msg : messageLog) {
            ImVec4 colour;
            colour = {0.13f, 0.44f, 0.61f, 1.0f};
            ImGui::TextColored(colour, "%s", msg.c_str());
        }

        ImGui::PopTextWrapPos();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor();

    if (m_should_take_focus) {
        ImGui::SetKeyboardFocusHere();
        m_should_take_focus = false;
    }

    ImGui::PushItemWidth(-1.f);
    if (this->command_line_input("##Input", &cmd, ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory) || ImGui::IsItemActive()) textbox_react = true;
    ImGui::PopItemWidth();

    if (m_input_text_id == 0u) {
        m_input_text_id = ImGui::GetItemID();
    }

    auto call_command = [&]() {
        if (!m_history.empty() && m_history.back() != cmd) {
            m_history.push_back(cmd);
            m_history.erase(m_history.begin());
        }

        m_hindex = m_history.size();

        if (L) {
            const int oldtop = lua_gettop(L);
            bool evalok = try_eval(cmd, true) || try_eval(cmd, false);

            if (evalok && LUA_OK == lua_pcall(L, 0, LUA_MULTRET, 0)) {
                if (oldtop != lua_gettop(L)) print_luastack(oldtop + 1, lua_gettop(L), Logger::Level::TRACE);

                lua_settop(L, oldtop);
            } else {
                const std::string err = adjust_error_msg(L, -1);
                if (evalok || !Neko::incomplete_chunk_error(err.c_str(), err.length())) {
                    print(err, Logger::Level::ERR);
                }
                lua_pop(L, 1);
            }
        } else {
            print("Lua state pointer is NULL, commands have no effect", Logger::Level::ERR);
        }
        cmd.clear();
    };

    if (m_previously_active_id == m_input_text_id && ImGui::GetActiveID() != m_input_text_id) {
        if (ImGui::IsKeyDown(ImGuiKey_Enter)) {
            call_command();
            m_should_take_focus = true;
        } else {
            m_should_take_focus = false;
        }
    }
    m_previously_active_id = ImGui::GetActiveID();

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);

    show_autocomplete();
}

void Neko::LuaInspector::inspect_table(lua_State* L, inspect_table_config& cfg) {
    auto is_multiline = [](const_str str) -> bool {
        while (*str != '\0') {
            if (*str == '\n') return true;
            str++;
        }
        return false;
    };

    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {

        if (lua_isnil(L, -2)) {
            lua_pop(L, 2);
            break;
        }

        bool edited = false;
        auto name = LuaGet<const_str>(L, -2);
        if (cfg.search_str != 0 && !strstr(name, cfg.search_str)) {
            lua_pop(L, 1);
            continue;
        }

        int type = lua_type(L, -1);

        if (cfg.is_non_function && type == LUA_TFUNCTION) {
            goto skip;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

        if (type == LUA_TSTRING) {

            bool open = ImGui::TreeNodeEx(name, tree_node_flags);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", lua_typename(L, lua_type(L, -1)));
            ImGui::TableNextColumn();
            // ImGui::Text("%p", lua_topointer(L, -1));

            const_str str_mem = LuaGet<const_str>(L, -1);
            size_t buffer_size = 256;
            for (; buffer_size < strlen(str_mem);) buffer_size += 128;
            std::string v(LuaGet<const_str>(L, -1), buffer_size);
            if (!is_multiline(str_mem) && strlen(str_mem) < 32) {
                ImGui::TextColored(rgba_to_imvec(40, 220, 55, 255), "\"%s\"", str_mem);
            } else {
                ImGui::TextColored(rgba_to_imvec(40, 220, 55, 255), "\"...\"");
            }

            if (open) {

                ImGui::InputTextMultiline("value", const_cast<char*>(v.c_str()), buffer_size);
                if (ImGui::IsKeyDown(ImGuiKey_Enter) && v != LuaGet<const_str>(L, -1)) {
                    edited = true;
                    lua_pop(L, 1);                 // # -1 pop value
                    lua_pushstring(L, v.c_str());  // # -1 push new value
                    lua_setfield(L, -3, name);     // -3 table
                    LOG_INFO("改 {} = {}", name, v.c_str());
                }

                ImGui::TreePop();
            }

        } else if (type == LUA_TNUMBER) {

            bool open = ImGui::TreeNodeEx(name, tree_node_flags);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", lua_typename(L, lua_type(L, -1)));
            ImGui::TableNextColumn();
            ImGui::Text("%f", LuaGet<f64>(L, -1));

            if (open) {
                f64 v = LuaGet<f64>(L, -1);
                // ImGui::Text("lua_v: %f", v);
                ImGui::InputDouble("value", &v);
                if (ImGui::IsKeyDown(ImGuiKey_Enter) && v != LuaGet<f64>(L, -1)) {
                    edited = true;
                    lua_pop(L, 1);              // # -1 pop value
                    lua_pushnumber(L, v);       // # -1 push new value
                    lua_setfield(L, -3, name);  // -3 table
                }
                ImGui::TreePop();
            }

        } else if (type == LUA_TFUNCTION) {
            ImGui::TreeNodeEx(name, tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", lua_typename(L, lua_type(L, -1)));
            ImGui::TableNextColumn();
            ImGui::TextColored(rgba_to_imvec(110, 180, 255, 255), "%p", lua_topointer(L, -1));

        } else if (type == LUA_TTABLE) {
            // ImGui::Text("lua_v: %p", lua_topointer(L, -1));
            // ImGui::Indent();
            // inspect_table(L);
            // ImGui::Unindent();

            bool open = ImGui::TreeNodeEx(name, tree_node_flags);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", lua_typename(L, lua_type(L, -1)));
            ImGui::TableNextColumn();
            ImGui::TextDisabled("--");
            if (open) {
                inspect_table(L, cfg);
                ImGui::TreePop();
            }

        } else if (type == LUA_TUSERDATA) {

            bool open = ImGui::TreeNodeEx(name, tree_node_flags);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", lua_typename(L, lua_type(L, -1)));
            ImGui::TableNextColumn();
            ImGui::TextColored(rgba_to_imvec(75, 230, 250, 255), "%p", lua_topointer(L, -1));

            if (open) {

                ImGui::Text("lua_v: %p", lua_topointer(L, -1));

                const_str metafields[] = {"__name", "__index"};

                if (lua_getmetatable(L, -1)) {
                    for (int i = 0; i < NEKO_ARR_SIZE(metafields); i++) {
                        lua_pushstring(L, metafields[i]);
                        lua_gettable(L, -2);
                        if (lua_isstring(L, -1)) {
                            const char* name = lua_tostring(L, -1);
                            ImGui::Text("%s: %s", metafields[i], name);
                        } else if (lua_isnumber(L, -1)) {
                            f64 name = lua_tonumber(L, -1);
                            ImGui::Text("%s: %lf", metafields[i], name);
                        } else {
                            ImGui::Text("%s field not exist", metafields[i]);
                        }
                        // pop value and table
                        lua_pop(L, 1);
                    }
                    lua_pop(L, 1);
                } else {
                    ImGui::TextColored(rgba_to_imvec(240, 0, 0, 255), "Unknown Metatable");
                }

                ImGui::TreePop();
            }
        } else if (type == LUA_TBOOLEAN) {
            ImGui::TreeNodeEx(name, tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", lua_typename(L, lua_type(L, -1)));
            ImGui::TableNextColumn();
            ImGui::TextColored(rgba_to_imvec(220, 160, 40, 255), "%s", NEKO_BOOL_STR(lua_toboolean(L, -1)));
        } else if (type == LUA_TCDATA) {
            ImGui::TreeNodeEx(name, tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", lua_typename(L, lua_type(L, -1)));
            ImGui::TableNextColumn();
            ImGui::TextColored(rgba_to_imvec(220, 160, 40, 255), "%p", lua_topointer(L, -1));
        } else {
            ImGui::TreeNodeEx(name, tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TableNextColumn();
            ImGui::TextColored(rgba_to_imvec(240, 0, 0, 255), "Unknown %d", type);
            ImGui::TableNextColumn();
            ImGui::Text("Unknown");
        }

    skip:
        if (edited) {
        } else {
            lua_pop(L, 1);
        }
    }
}

Neko::CCharacter cJohn;

void inspect_shader(const char* label, GLuint program);

int Neko::LuaInspector::luainspector_init(lua_State* L) {

    this->setL(L);
    this->m_history.resize(8);

    this->register_function<float>(std::bind(&luainspector_property_st::Render_TypeFloat, std::placeholders::_1, std::placeholders::_2));
    this->register_function<bool>(std::bind(&luainspector_property_st::Render_TypeBool, std::placeholders::_1, std::placeholders::_2));
    this->register_function<char>(std::bind(&luainspector_property_st::Render_TypeConstChar, std::placeholders::_1, std::placeholders::_2));
    this->register_function<double>(std::bind(&luainspector_property_st::Render_TypeDouble, std::placeholders::_1, std::placeholders::_2));
    this->register_function<int>(std::bind(&luainspector_property_st::Render_TypeInt, std::placeholders::_1, std::placeholders::_2));

    this->register_function<CCharacter>(std::bind(&luainspector_property_st::Render_TypeCharacter, std::placeholders::_1, std::placeholders::_2));

    this->property_register<CCharacter>("John", &cJohn, "John");

    return 1;
}

int Neko::LuaInspector::luainspector_draw(lua_State* L) {

    RenderConsoleLogs();

    if (!visible) {
        return 0;
    }

    Assets& g_assets = the<Assets>();

    if (ImGui::Begin("LuaInspector")) {

        if (ImGui::BeginTabBar("lua_inspector", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("LuaConsole")) {
                bool textbox_react;
                this->console_draw(textbox_react);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Registry")) {
                lua_pushglobaltable(L);  // _G
                static char search_text[256] = "";
                static inspect_table_config config;
                config.search_str = search_text;

                ImGui::InputTextWithHint("Search", "Search...", search_text, IM_ARRAYSIZE(search_text));

                ImGui::Checkbox("Non-Function", &config.is_non_function);

                ImGui::Text("Registry contents:");

                const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
                const float TEXT_BASE_HIGHT = ImGui::CalcTextSize("A").y;

                ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 100 - TEXT_BASE_HIGHT * 4);
                if (ImGui::BeginChild("##lua_registry", size)) {
                    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

                    static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

                    if (ImGui::BeginTable("lua_inspector_reg", 3, flags)) {
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
                        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8.0f);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 32.0f);
                        ImGui::TableHeadersRow();

                        inspect_table(L, config);

                        ImGui::EndTable();
                    }

                    ImGui::PopTextWrapPos();

                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 50) ImGui::SetScrollY(ImGui::GetScrollMaxY());
                }
                ImGui::EndChild();

                lua_pop(L, 1);  // pop _G
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Property")) {

                auto& properties = this->m_property_map;

                ImGui::Text("Property count: %lld\nType count: %lld", properties.size(), this->m_type_render_functions.size());

                for (auto& property_it : properties) {

                    auto prop_render_func_it = this->m_type_render_functions.find(property_it.param_type);
                    neko_assert(prop_render_func_it != this->m_type_render_functions.end());  // unsupported type, render function not found
                    const_str label = property_it.label.c_str();
                    void* value = property_it.param;
                    if (ImGui::CollapsingHeader(std::format("{0} | {1}", label, property_it.param_type.name()).c_str())) {
                        ImGui::Indent();
                        prop_render_func_it->second(label, value);
                        ImGui::Unindent();
                    }
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Info")) {
                lua_Integer kb = lua_gc(L, LUA_GCCOUNT, 0);
                lua_Integer bytes = lua_gc(L, LUA_GCCOUNTB, 0);

                auto UpdateLuaMemPlot = [](float bytes) {
                    static std::deque<float> frameTimes;
                    static std::mutex frameTimesMutex;
                    {
                        std::lock_guard<std::mutex> lock(frameTimesMutex);  // 加锁以保证线程安全

                        // 如果 deque 为空或最后一个值不等于当前 bytes，则添加新数据
                        if (frameTimes.empty() || frameTimes.back() != bytes) {
                            frameTimes.push_back(bytes);

                            // 限制 deque 的最大大小为 100，超过时移除头部数据
                            if (frameTimes.size() > 100) {
                                frameTimes.pop_front();
                            }
                        }
                    }
                    float minVal = 0.0f, maxVal = 4000.0f;  // 默认范围
                    {
                        std::lock_guard<std::mutex> lock(frameTimesMutex);  // 加锁以保证线程安全
                        if (!frameTimes.empty()) {
                            minVal = *std::min_element(frameTimes.begin(), frameTimes.end());
                            maxVal = *std::max_element(frameTimes.begin(), frameTimes.end());
                        }
                    }
                    std::vector<float> dataCopy;
                    {
                        std::lock_guard<std::mutex> lock(frameTimesMutex);  // 加锁以保证线程安全
                        dataCopy.assign(frameTimes.begin(), frameTimes.end());
                    }
                    ImGui::PlotLines("LuaVM Memory", dataCopy.data(), static_cast<int>(dataCopy.size()), 0, nullptr, minVal, maxVal, ImVec2(0, 80.0f));
                };
                UpdateLuaMemPlot(bytes);

                ImGui::Text("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                ImGui::Text("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

                if (ImGui::Button("GC")) lua_gc(L, LUA_GCCOLLECT, 0);

                for (auto kv : g_assets.table) {
                    ImGui::Text("%lld %s", kv.key, kv.value->name.cstr());
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Shaders")) {
                asset_view_each([](const Asset& view) {
                    if (view.kind == AssetKind_Shader) inspect_shader(view.name.cstr(), assets_get<AssetShader>(view).id);
                });
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Textures")) {
                asset_view_each([](const Asset& view) {
                    if (view.kind == AssetKind_Image) {
                        const auto tex = assets_get<AssetTexture>(view);
                        f32 scale = 250.0 / tex.height;
                        ImGui::Image(tex.id, ImVec2(tex.width, tex.height) * scale);
                    }
                    if (view.kind == AssetKind_AseSprite) {
                        const auto spr = assets_get<AseSpriteData>(view);
                        f32 scale = 250.0 / spr.tex.height;
                        ImGui::Image(spr.tex.id, ImVec2(spr.tex.width, spr.tex.height) * scale);
                    }
                });
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    return 0;
}

#if 1

// 生成宏 以避免始终重复代码
#define INSPECTOR_GENERATE_VARIABLE(cputype, count, gltype, glread, glwrite, imguifunc) \
    {                                                                                   \
        ImGui::Text(#gltype " %s:", name);                                              \
        cputype value[count];                                                           \
        glread(program, location, &value[0]);                                           \
        if (imguifunc("", &value[0], 0.25f)) glwrite(program, location, 1, &value[0]);  \
    }

#define INSPECTOR_GENERATE_MATRIX(cputype, rows, columns, gltype, glread, glwrite, imguifunc) \
    {                                                                                         \
        ImGui::Text(#gltype " %s:", name);                                                    \
        cputype value[rows * columns];                                                        \
        int size = rows * columns;                                                            \
        glread(program, location, &value[0]);                                                 \
        int modified = 0;                                                                     \
        for (int i = 0; i < size; i += rows) {                                                \
            ImGui::PushID(i);                                                                 \
            modified += imguifunc("", &value[i], 0.25f);                                      \
            ImGui::PopID();                                                                   \
        }                                                                                     \
        if (modified) glwrite(program, location, 1, GL_FALSE, value);                         \
    }

void render_uniform_variable(GLuint program, GLenum type, const char* name, GLint location) {
    static bool is_color = false;
    switch (type) {
        case GL_FLOAT:
            INSPECTOR_GENERATE_VARIABLE(GLfloat, 1, GL_FLOAT, glGetUniformfv, glProgramUniform1fv, ImGui::DragFloat);
            break;

        case GL_FLOAT_VEC2:
            INSPECTOR_GENERATE_VARIABLE(GLfloat, 2, GL_FLOAT_VEC2, glGetUniformfv, glProgramUniform2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_VEC3: {
            ImGui::Checkbox("##is_color", &is_color);
            ImGui::SameLine();
            ImGui::Text("GL_FLOAT_VEC3 %s", name);
            ImGui::SameLine();
            float value[3];
            glGetUniformfv(program, location, &value[0]);
            if ((!is_color && ImGui::DragFloat3("", &value[0])) || (is_color && ImGui::ColorEdit3("Color", &value[0], ImGuiColorEditFlags_NoLabel)))
                glProgramUniform3fv(program, location, 1, &value[0]);
        } break;

        case GL_FLOAT_VEC4: {
            ImGui::Checkbox("##is_color", &is_color);
            ImGui::SameLine();
            ImGui::Text("GL_FLOAT_VEC4 %s", name);
            ImGui::SameLine();
            float value[4];
            glGetUniformfv(program, location, &value[0]);
            if ((!is_color && ImGui::DragFloat4("", &value[0])) ||
                (is_color && ImGui::ColorEdit4("Color", &value[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)))
                glProgramUniform4fv(program, location, 1, &value[0]);
        } break;

        case GL_INT:
            INSPECTOR_GENERATE_VARIABLE(GLint, 1, GL_INT, glGetUniformiv, glProgramUniform1iv, ImGui::DragInt);
            break;

        case GL_INT_VEC2:
            INSPECTOR_GENERATE_VARIABLE(GLint, 2, GL_INT, glGetUniformiv, glProgramUniform2iv, ImGui::DragInt2);
            break;

        case GL_INT_VEC3:
            INSPECTOR_GENERATE_VARIABLE(GLint, 3, GL_INT, glGetUniformiv, glProgramUniform3iv, ImGui::DragInt3);
            break;

        case GL_INT_VEC4:
            INSPECTOR_GENERATE_VARIABLE(GLint, 4, GL_INT, glGetUniformiv, glProgramUniform4iv, ImGui::DragInt4);
            break;

        case GL_UNSIGNED_INT: {
            ImGui::Text("GL_UNSIGNED_INT %s:", name);
            ImGui::SameLine();
            GLuint value[1];
            glGetUniformuiv(program, location, &value[0]);
            if (ImGui::DragScalar("", ImGuiDataType_U32, &value[0], 0.25f)) glProgramUniform1uiv(program, location, 1, &value[0]);
        } break;

        case GL_UNSIGNED_INT_VEC3: {
            ImGui::Text("GL_UNSIGNED_INT_VEC3 %s:", name);
            ImGui::SameLine();
            GLuint value[1];
            glGetUniformuiv(program, location, &value[0]);
            if (ImGui::DragScalarN("", ImGuiDataType_U32, &value[0], 3, 0.25f)) glProgramUniform3uiv(program, location, 1, &value[0]);
        } break;

        case GL_SAMPLER_2D:
            INSPECTOR_GENERATE_VARIABLE(GLint, 1, GL_SAMPLER_2D, glGetUniformiv, glProgramUniform1iv, ImGui::DragInt);
            break;

        case GL_FLOAT_MAT2:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 2, 2, GL_FLOAT_MAT2, glGetUniformfv, glProgramUniformMatrix2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_MAT3:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 3, 3, GL_FLOAT_MAT3, glGetUniformfv, glProgramUniformMatrix3fv, ImGui::DragFloat3);
            break;

        case GL_FLOAT_MAT4:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 4, 4, GL_FLOAT_MAT4, glGetUniformfv, glProgramUniformMatrix4fv, ImGui::DragFloat4);
            break;

        case GL_FLOAT_MAT2x3:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 3, 2, GL_FLOAT_MAT2x3, glGetUniformfv, glProgramUniformMatrix2x3fv, ImGui::DragFloat3);
            break;

        case GL_FLOAT_MAT2x4:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 4, 2, GL_FLOAT_MAT2x4, glGetUniformfv, glProgramUniformMatrix2x4fv, ImGui::DragFloat4);
            break;

        case GL_FLOAT_MAT3x2:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 2, 3, GL_FLOAT_MAT3x2, glGetUniformfv, glProgramUniformMatrix3x2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_MAT3x4:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 4, 3, GL_FLOAT_MAT3x4, glGetUniformfv, glProgramUniformMatrix3x2fv, ImGui::DragFloat4);
            break;

        case GL_BOOL: {
            ImGui::Text("GL_BOOL %s:", name);
            ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            if (ImGui::Checkbox("", (bool*)&value)) glProgramUniform1uiv(program, location, 1, &value);
        } break;

            // #if !defined(NEKO_IS_APPLE)
            //         case GL_IMAGE_2D: {
            //             ImGui::Text("GL_IMAGE_2D %s:", name);
            //             // ImGui::SameLine();
            //             GLuint value;
            //             glGetUniformuiv(program, location, &value);
            //             // if (ImGui::Checkbox("", (bool*)&value)) glProgramUniform1iv(program, location, 1, &value);
            //             ImGui::Image((void*)(intptr_t)value, ImVec2(256, 256));
            //         } break;
            // #endif

        case GL_SAMPLER_CUBE: {
            ImGui::Text("GL_SAMPLER_CUBE %s:", name);
            // ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            ImGui::Image((ImTextureID)(intptr_t)value, ImVec2(256, 256));
        } break;

        default:
            ImGui::TextColored(rgba_to_imvec(255, 64, 64), "%s has type %s, which isn't supported yet!", name, opengl_string(type));
            break;
    }
}

float get_scrollable_height() { return ImGui::GetTextLineHeight() * 16; }

void inspect_shader(const char* label, GLuint program) {
    neko_assert(label != nullptr);

    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
        if (!glIsProgram(program)) {
            ImGui::Text("%d glIsProgram failed", program);
        } else {
            // Uniforms
            ImGui::Indent();
            if (ImGui::CollapsingHeader("Uniforms", ImGuiTreeNodeFlags_DefaultOpen)) {
                GLint uniform_count;
                glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

                // Read the length of the longest active uniform.
                GLint max_name_length;
                glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_length);

                static std::vector<char> name;
                name.resize(max_name_length);

                for (int i = 0; i < uniform_count; i++) {
                    GLint ignored;
                    GLenum type;
                    glGetActiveUniform(program, i, max_name_length, nullptr, &ignored, &type, name.data());

                    const auto location = glGetUniformLocation(program, name.data());
                    ImGui::Indent();
                    ImGui::PushID(i);
                    ImGui::PushItemWidth(-1.0f);
                    render_uniform_variable(program, type, name.data(), location);
                    ImGui::PopItemWidth();
                    ImGui::PopID();
                    ImGui::Unindent();
                }
            }
            ImGui::Unindent();

            // Shaders
            ImGui::Indent();
            if (ImGui::CollapsingHeader("Shaders")) {
                GLint shader_count;
                glGetProgramiv(program, GL_ATTACHED_SHADERS, &shader_count);

                static std::vector<GLuint> attached_shaders;
                attached_shaders.resize(shader_count);
                glGetAttachedShaders(program, shader_count, nullptr, attached_shaders.data());

                for (const auto& shader : attached_shaders) {
                    GLint source_length = 0;
                    glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &source_length);
                    static std::vector<char> source;
                    source.resize(source_length);
                    glGetShaderSource(shader, source_length, nullptr, source.data());

                    GLint type = 0;
                    glGetShaderiv(shader, GL_SHADER_TYPE, &type);

                    ImGui::Indent();
                    auto string_type = opengl_string(type);
                    ImGui::PushID(string_type);
                    if (ImGui::CollapsingHeader(string_type)) {
                        auto y_size = std::min(ImGui::CalcTextSize(source.data()).y, get_scrollable_height());
                        ImGui::InputTextMultiline("", source.data(), source.size(), ImVec2(-1.0f, y_size), ImGuiInputTextFlags_ReadOnly);
                    }
                    ImGui::PopID();
                    ImGui::Unindent();
                }
            }
            ImGui::Unindent();
        }
    }
    ImGui::PopID();
}

void inspect_vertex_array(const char* label, GLuint vao) {
    neko_assert(label != nullptr);
    neko_assert(glIsVertexArray(vao));

    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
        ImGui::Indent();

        // 获取当前绑定的顶点缓冲区对象 以便我们可以在完成后将其重置回来
        GLint current_vbo = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_vbo);

        // 获取当前绑定的顶点数组对象 以便我们可以在完成后将其重置回来
        GLint current_vao = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
        glBindVertexArray(vao);

        // 获取顶点属性的最大数量
        // 无论这里有多少个属性 迭代都应该是合理的
        GLint max_vertex_attribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);

        GLint ebo = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo);

        // EBO Visualization
        char buffer[128];
        std::snprintf(buffer, 128, "Element Array Buffer: %d", ebo);
        ImGui::PushID(buffer);
        if (ImGui::CollapsingHeader(buffer)) {
            ImGui::Indent();
            // 假设为 unsigned int
            int size = 0;
            glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
            size /= sizeof(GLuint);
            ImGui::Text("Size: %d", size);

            if (ImGui::TreeNode("Buffer Contents")) {
                // TODO 找到一种更好的方法将其显示在屏幕上 因为当我们获得大量索引时 该解决方案可能不会有很好的伸缩性
                // 可能的解决方案 像VBO一样将其做成列 并将索引显示为三角形
                auto ptr = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
                for (int i = 0; i < size; i++) {
                    ImGui::Text("%u", ptr[i]);
                    ImGui::SameLine();
                    if ((i + 1) % 3 == 0) ImGui::NewLine();
                }

                glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

                ImGui::TreePop();
            }

            ImGui::Unindent();
        }
        ImGui::PopID();

        // VBO Visualization
        for (intptr_t i = 0; i < max_vertex_attribs; i++) {
            GLint enabled = 0;
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

            if (!enabled) continue;

            std::snprintf(buffer, 128, "Attribute: %" PRIdPTR "", i);
            ImGui::PushID(buffer);
            if (ImGui::CollapsingHeader(buffer)) {
                ImGui::Indent();
                // 元数据显示
                GLint buffer = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
                ImGui::Text("Buffer: %d", buffer);

                GLint type = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
                ImGui::Text("Type: %s", opengl_string(type));

                GLint dimensions = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &dimensions);
                ImGui::Text("Dimensions: %d", dimensions);

                // 需要绑定缓冲区以访问 parameteriv 并在以后进行映射
                glBindBuffer(GL_ARRAY_BUFFER, buffer);

                GLint size = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
                ImGui::Text("Size in bytes: %d", size);

                GLint stride = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
                ImGui::Text("Stride in bytes: %d", stride);

                GLvoid* offset = nullptr;
                glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &offset);
                ImGui::Text("Offset in bytes: %" PRIdPTR "", (intptr_t)offset);

                GLint usage = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_USAGE, &usage);
                ImGui::Text("Usage: %s", opengl_string(usage));

                // 创建包含索引和实际内容的表
                if (ImGui::TreeNode("Buffer Contents")) {
                    ImGui::BeginChild(ImGui::GetID("vbo contents"), ImVec2(-1.0f, get_scrollable_height()), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                    ImGui::Columns(dimensions + 1);
                    const char* descriptors[] = {"index", "x", "y", "z", "w"};
                    for (int j = 0; j < dimensions + 1; j++) {
                        ImGui::Text("%s", descriptors[j]);
                        ImGui::NextColumn();
                    }
                    ImGui::Separator();

                    auto ptr = (char*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY) + (intptr_t)offset;
                    for (int j = 0, c = 0; j < size; j += stride, c++) {
                        ImGui::Text("%d", c);
                        ImGui::NextColumn();
                        for (int k = 0; k < dimensions; k++) {
                            switch (type) {
                                case GL_BYTE:
                                    ImGui::Text("% d", *(GLbyte*)&ptr[j + k * sizeof(GLbyte)]);
                                    break;
                                case GL_UNSIGNED_BYTE:
                                    ImGui::Text("%u", *(GLubyte*)&ptr[j + k * sizeof(GLubyte)]);
                                    break;
                                case GL_SHORT:
                                    ImGui::Text("% d", *(GLshort*)&ptr[j + k * sizeof(GLshort)]);
                                    break;
                                case GL_UNSIGNED_SHORT:
                                    ImGui::Text("%u", *(GLushort*)&ptr[j + k * sizeof(GLushort)]);
                                    break;
                                case GL_INT:
                                    ImGui::Text("% d", *(GLint*)&ptr[j + k * sizeof(GLint)]);
                                    break;
                                case GL_UNSIGNED_INT:
                                    ImGui::Text("%u", *(GLuint*)&ptr[j + k * sizeof(GLuint)]);
                                    break;
                                case GL_FLOAT:
                                    ImGui::Text("% f", *(GLfloat*)&ptr[j + k * sizeof(GLfloat)]);
                                    break;
                                case GL_DOUBLE:
                                    ImGui::Text("% f", *(GLdouble*)&ptr[j + k * sizeof(GLdouble)]);
                                    break;
                            }
                            ImGui::NextColumn();
                        }
                    }
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                    ImGui::EndChild();
                    ImGui::TreePop();
                }
                ImGui::Unindent();
            }
            ImGui::PopID();
        }

        // Cleanup
        glBindVertexArray(current_vao);
        glBindBuffer(GL_ARRAY_BUFFER, current_vbo);

        ImGui::Unindent();
    }
    ImGui::PopID();
}

#endif

// -------------------------------------------------------------------------

static int l_edit_bboxes_get_nth_bbox(lua_State* L) {
    int i = lua_tointeger(L, 1);
    BBox v = edit_bboxes_get_nth_bbox(i);
    LuaPush<BBox>(L, v);
    return 1;
}

static int l_edit_line_add(lua_State* L) {
    vec2* a = LuaGet<vec2>(L, 1);
    vec2* b = LuaGet<vec2>(L, 2);
    f32 p = lua_tonumber(L, 3);
    Color* col = LuaGet<Color>(L, 4);
    edit_line_add(*a, *b, p, *col);
    return 0;
}

static int l_edit_set_enabled(lua_State* L) {
    bool enable = lua_toboolean(L, 1);
    edit_set_enabled(enable);
    return 0;
}

static int l_edit_get_enabled(lua_State* L) {
    bool enable = edit_get_enabled();
    lua_pushboolean(L, enable);
    return 1;
}

// -------------------------------------------------------------------------

void Editor::edit_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    edit_init_impl(L);

    auto type = BUILD_TYPE(Editor)
                        .Method("edit_set_editable", &edit_set_editable)                //
                        .Method("edit_get_editable", &edit_get_editable)                //
                        .Method("edit_set_grid_size", &edit_set_grid_size)              //
                        .Method("edit_get_grid_size", &edit_get_grid_size)              //
                        .Method("edit_bboxes_has", &edit_bboxes_has)                    //
                        .Method("edit_bboxes_get_num", &edit_bboxes_get_num)            //
                        .Method("edit_bboxes_get_nth_ent", &edit_bboxes_get_nth_ent)    //
                        .Method("edit_bboxes_set_selected", &edit_bboxes_set_selected)  //
                        .CClosure({{"edit_bboxes_get_nth_bbox", l_edit_bboxes_get_nth_bbox},
                                   {"edit_line_add", l_edit_line_add},
                                   {"edit_set_enabled", l_edit_set_enabled},
                                   {"edit_get_enabled", l_edit_get_enabled}})
                        .Build();
}

void Editor::edit_fini() { edit_fini_impl(); }

int Editor::edit_update_all(Event evt) {
    edit_update_impl();

    return 0;
}

void Editor::edit_draw_all() {
    if (!enabled) return;

    edit_draw_impl();
}

