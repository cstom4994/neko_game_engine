#include "neko_imgui_utils.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "engine/base/neko_engine.h"
#include "engine/common/neko_util.h"
#include "engine/gui/imgui_impl/imgui_impl_glfw.h"
#include "engine/gui/imgui_impl/imgui_impl_opengl3.h"
#include "engine/platform/neko_platform.h"
#include "libs/glad/glad.h"

namespace neko::imgui {

markdown::markdown() {
    m_md.abi_version = 0;
    m_md.flags = MD_FLAG_TABLES | MD_FLAG_UNDERLINE | MD_FLAG_STRIKETHROUGH;
    m_md.enter_block = [](MD_BLOCKTYPE t, void* d, void* u) { return ((markdown*)u)->block(t, d, true); };
    m_md.leave_block = [](MD_BLOCKTYPE t, void* d, void* u) { return ((markdown*)u)->block(t, d, false); };
    m_md.enter_span = [](MD_SPANTYPE t, void* d, void* u) { return ((markdown*)u)->span(t, d, true); };
    m_md.leave_span = [](MD_SPANTYPE t, void* d, void* u) { return ((markdown*)u)->span(t, d, false); };
    m_md.text = [](MD_TEXTTYPE t, const MD_CHAR* text, MD_SIZE size, void* u) { return ((markdown*)u)->text(t, text, text + size); };

    m_md.debug_log = nullptr;
    m_md.syntax = nullptr;

    m_table_last_pos = ImVec2(0, 0);
}

void markdown::BLOCK_UL(const MD_BLOCK_UL_DETAIL* d, bool e) {
    if (e) {
        m_list_stack.push_back(list_info{0, d->mark, false});
    } else {
        m_list_stack.pop_back();
        if (m_list_stack.empty()) ImGui::NewLine();
    }
}

void markdown::BLOCK_OL(const MD_BLOCK_OL_DETAIL* d, bool e) {
    if (e) {
        m_list_stack.push_back(list_info{d->start, d->mark_delimiter, true});
    } else {
        m_list_stack.pop_back();
        if (m_list_stack.empty()) ImGui::NewLine();
    }
}

void markdown::BLOCK_LI(const MD_BLOCK_LI_DETAIL*, bool e) {
    if (e) {
        ImGui::NewLine();

        list_info& nfo = m_list_stack.back();
        if (nfo.is_ol) {
            ImGui::Text("%d%c", nfo.cur_ol++, nfo.delim);
            ImGui::SameLine();
        } else {
            if (nfo.delim == '*') {
                float cx = ImGui::GetCursorPosX();
                cx -= ImGui::GetStyle().FramePadding.x * 2;
                ImGui::SetCursorPosX(cx);
                ImGui::Bullet();
            } else {
                ImGui::Text("%c", nfo.delim);
                ImGui::SameLine();
            }
        }

        ImGui::Indent();
    } else {
        ImGui::Unindent();
    }
}

void markdown::BLOCK_HR(bool e) {
    if (!e) {
        ImGui::NewLine();
        ImGui::Separator();
    }
}

void markdown::BLOCK_H(const MD_BLOCK_H_DETAIL* d, bool e) {
    if (e) {
        m_hlevel = d->level;
        ImGui::NewLine();
    } else {
        m_hlevel = 0;
    }

    set_font(e);

    if (!e) {
        if (d->level <= 2) {
            ImGui::NewLine();
            ImGui::Separator();
        }
    }
}

void markdown::BLOCK_DOC(bool) {}

void markdown::BLOCK_QUOTE(bool) {}
void markdown::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e) { m_is_code = e; }

void markdown::BLOCK_HTML(bool) {}

void markdown::BLOCK_P(bool) {
    if (!m_list_stack.empty()) return;
    ImGui::NewLine();
}

void markdown::BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL*, bool e) {
    if (e) {
        m_table_row_pos.clear();
        m_table_col_pos.clear();

        m_table_last_pos = ImGui::GetCursorPos();
    } else {

        ImGui::NewLine();

        if (m_table_border) {

            m_table_last_pos.y = ImGui::GetCursorPos().y;

            m_table_col_pos.push_back(m_table_last_pos.x);
            m_table_row_pos.push_back(m_table_last_pos.y);

            const ImVec2 wp = ImGui::GetWindowPos();
            const ImVec2 sp = ImGui::GetStyle().ItemSpacing;
            const float wx = wp.x + sp.x / 2;
            const float wy = wp.y - sp.y / 2 - ImGui::GetScrollY();

            for (int i = 0; i < m_table_col_pos.size(); ++i) {
                m_table_col_pos[i] += wx;
            }

            for (int i = 0; i < m_table_row_pos.size(); ++i) {
                m_table_row_pos[i] += wy;
            }

            ////////////////////////////////////////////////////////////////////

            const ImColor c = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];

            ImDrawList* dl = ImGui::GetWindowDrawList();

            const float xmin = m_table_col_pos.front();
            const float xmax = m_table_col_pos.back();
            for (int i = 0; i < m_table_row_pos.size(); ++i) {
                const float p = m_table_row_pos[i];
                dl->AddLine(ImVec2(xmin, p), ImVec2(xmax, p), c, i == 1 && m_table_header_highlight ? 2.0f : 1.0f);
            }

            const float ymin = m_table_row_pos.front();
            const float ymax = m_table_row_pos.back();
            for (int i = 0; i < m_table_col_pos.size(); ++i) {
                const float p = m_table_col_pos[i];
                dl->AddLine(ImVec2(p, ymin), ImVec2(p, ymax), c, 1.0f);
            }
        }
    }
}

void markdown::BLOCK_THEAD(bool e) {
    m_is_table_header = e;
    if (m_table_header_highlight) set_font(e);
}

void markdown::BLOCK_TBODY(bool e) { m_is_table_body = e; }

void markdown::BLOCK_TR(bool e) {
    ImGui::SetCursorPosY(m_table_last_pos.y);

    if (e) {
        m_table_next_column = 0;
        ImGui::NewLine();
        m_table_row_pos.push_back(ImGui::GetCursorPosY());
    }
}

void markdown::BLOCK_TH(const MD_BLOCK_TD_DETAIL* d, bool e) { BLOCK_TD(d, e); }

void markdown::BLOCK_TD(const MD_BLOCK_TD_DETAIL*, bool e) {
    if (e) {

        if (m_table_next_column < m_table_col_pos.size()) {
            ImGui::SetCursorPosX(m_table_col_pos[m_table_next_column]);
        } else {
            m_table_col_pos.push_back(m_table_last_pos.x);
        }

        ++m_table_next_column;

        ImGui::Indent(m_table_col_pos[m_table_next_column - 1]);
        ImGui::SetCursorPos(ImVec2(m_table_col_pos[m_table_next_column - 1], m_table_row_pos.back()));

    } else {
        const ImVec2 p = ImGui::GetCursorPos();
        ImGui::Unindent(m_table_col_pos[m_table_next_column - 1]);
        ImGui::SetCursorPosX(p.x);
        if (p.y > m_table_last_pos.y) m_table_last_pos.y = p.y;
    }
    ImGui::TextUnformatted("");

    if (!m_table_border && e && m_table_next_column == 1) {
        ImGui::SameLine(0.0f, 0.0f);
    } else {
        ImGui::SameLine();
    }
}

////////////////////////////////////////////////////////////////////////////////
void markdown::set_href(bool e, const MD_ATTRIBUTE& src) {
    if (e) {
        m_href.assign(src.text, src.size);
    } else {
        m_href.clear();
    }
}

void markdown::set_font(bool e) {
    if (e) {
        ImGui::PushFont(get_font());
    } else {
        ImGui::PopFont();
    }
}

void markdown::set_color(bool e) {
    if (e) {
        ImGui::PushStyleColor(ImGuiCol_Text, get_color());
    } else {
        ImGui::PopStyleColor();
    }
}

void markdown::line(ImColor c, bool under) {
    ImVec2 mi = ImGui::GetItemRectMin();
    ImVec2 ma = ImGui::GetItemRectMax();

    if (!under) {
        ma.y -= ImGui::GetFontSize() / 2;
    }

    mi.y = ma.y;

    ImGui::GetWindowDrawList()->AddLine(mi, ma, c, 1.0f);
}

void markdown::SPAN_A(const MD_SPAN_A_DETAIL* d, bool e) {
    set_href(e, d->href);
    set_color(e);
}

void markdown::SPAN_EM(bool e) {
    m_is_em = e;
    set_font(e);
}

void markdown::SPAN_STRONG(bool e) {
    m_is_strong = e;
    set_font(e);
}

void markdown::SPAN_IMG(const MD_SPAN_IMG_DETAIL* d, bool e) {
    m_is_image = e;

    set_href(e, d->src);

    if (e) {

        image_info nfo;
        if (get_image(nfo)) {

            const float scale = ImGui::GetIO().FontGlobalScale;
            nfo.size.x *= scale;
            nfo.size.y *= scale;

            ImVec2 const csz = ImGui::GetContentRegionAvail();
            if (nfo.size.x > csz.x) {
                const float r = nfo.size.y / nfo.size.x;
                nfo.size.x = csz.x;
                nfo.size.y = csz.x * r;
            }

            ImGui::Image(nfo.texture_id, nfo.size, nfo.uv0, nfo.uv1, nfo.col_tint, nfo.col_border);

            if (ImGui::IsItemHovered()) {

                // if (d->title.size) {
                //  ImGui::SetTooltip("%.*s", (int)d->title.size, d->title.text);
                // }

                if (ImGui::IsMouseReleased(0)) {
                    open_url();
                }
            }
        }
    }
}

void markdown::SPAN_CODE(bool) {}

void markdown::SPAN_LATEXMATH(bool) {}

void markdown::SPAN_LATEXMATH_DISPLAY(bool) {}

void markdown::SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool) {}

void markdown::SPAN_U(bool e) { m_is_underline = e; }

void markdown::SPAN_DEL(bool e) { m_is_strikethrough = e; }

void markdown::render_text(const char* str, const char* str_end) {
    const float scale = ImGui::GetIO().FontGlobalScale;
    const ImGuiStyle& s = ImGui::GetStyle();
    bool is_lf = false;

    while (!m_is_image && str < str_end) {

        const char* te = str_end;

        if (!m_is_table_header) {

            float wl = ImGui::GetContentRegionAvail().x;

            if (m_is_table_body) {
                wl = (m_table_next_column < m_table_col_pos.size() ? m_table_col_pos[m_table_next_column] : m_table_last_pos.x);
                wl -= ImGui::GetCursorPosX();
            }

            te = ImGui::GetFont()->CalcWordWrapPositionA(scale, str, str_end, wl);

            if (te == str) ++te;
        }

        ImGui::TextUnformatted(str, te);

        if (te > str && *(te - 1) == '\n') {
            is_lf = true;
        }

        if (!m_href.empty()) {

            ImVec4 c;
            if (ImGui::IsItemHovered()) {

                ImGui::SetTooltip("%s", m_href.c_str());

                c = s.Colors[ImGuiCol_ButtonHovered];
                if (ImGui::IsMouseReleased(0)) {
                    open_url();
                }
            } else {
                c = s.Colors[ImGuiCol_Button];
            }
            line(c, true);
        }
        if (m_is_underline) {
            line(s.Colors[ImGuiCol_Text], true);
        }
        if (m_is_strikethrough) {
            line(s.Colors[ImGuiCol_Text], false);
        }

        str = te;

        while (str < str_end && *str == ' ') ++str;
    }

    if (!is_lf) ImGui::SameLine(0.0f, 0.0f);
}

bool markdown::render_entity(const char* str, const char* str_end) {
    const size_t sz = str_end - str;
    if (strncmp(str, "&nbsp;", sz) == 0) {
        ImGui::TextUnformatted("");
        ImGui::SameLine();
        return true;
    }
    return false;
}

static bool skip_spaces(const std::string& d, size_t& p) {
    for (; p < d.length(); ++p) {
        if (d[p] != ' ' && d[p] != '\t') {
            break;
        }
    }
    return p < d.length();
}

static std::string get_div_class(const char* str, const char* str_end) {
    if (str_end <= str) return "";

    std::string d(str, str_end - str);
    if (d.back() == '>') d.pop_back();

    const char attr[] = "class";
    size_t p = d.find(attr);
    if (p == std::string::npos) return "";
    p += sizeof(attr) - 1;

    if (!skip_spaces(d, p)) return "";

    if (d[p] != '=') return "";
    ++p;

    if (!skip_spaces(d, p)) return "";

    bool has_q = false;

    if (d[p] == '"' || d[p] == '\'') {
        has_q = true;
        ++p;
    }
    if (p == d.length()) return "";

    if (!has_q) {
        if (!skip_spaces(d, p)) return "";
    }

    size_t pe;
    for (pe = p; pe < d.length(); ++pe) {

        const char c = d[pe];

        if (has_q) {
            if (c == '"' || c == '\'') {
                break;
            }
        } else {
            if (c == ' ' || c == '\t') {
                break;
            }
        }
    }

    return d.substr(p, pe - p);
}

bool markdown::check_html(const char* str, const char* str_end) {
    const size_t sz = str_end - str;

    if (strncmp(str, "<br>", sz) == 0) {
        ImGui::NewLine();
        return true;
    }
    if (strncmp(str, "<hr>", sz) == 0) {
        ImGui::Separator();
        return true;
    }
    if (strncmp(str, "<u>", sz) == 0) {
        m_is_underline = true;
        return true;
    }
    if (strncmp(str, "</u>", sz) == 0) {
        m_is_underline = false;
        return true;
    }

    const size_t div_sz = 4;
    if (strncmp(str, "<div", sz > div_sz ? div_sz : sz) == 0) {
        m_div_stack.emplace_back(get_div_class(str + div_sz, str_end));
        html_div(m_div_stack.back(), true);
        return true;
    }
    if (strncmp(str, "</div>", sz) == 0) {
        if (m_div_stack.empty()) return false;
        html_div(m_div_stack.back(), false);
        m_div_stack.pop_back();
        return true;
    }
    return false;
}

void markdown::html_div(const std::string& dclass, bool e) {
    // Example:
#if 0 
    if (dclass == "red") {
        if (e) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        } else {
            ImGui::PopStyleColor();
        }
    }
#endif
    dclass;
    e;
}

int markdown::text(MD_TEXTTYPE type, const char* str, const char* str_end) {
    switch (type) {
        case MD_TEXT_NORMAL:
            render_text(str, str_end);
            break;
        case MD_TEXT_CODE:
            render_text(str, str_end);
            break;
        case MD_TEXT_NULLCHAR:
            break;
        case MD_TEXT_BR:
            ImGui::NewLine();
            break;
        case MD_TEXT_SOFTBR:
            soft_break();
            break;
        case MD_TEXT_ENTITY:
            if (!render_entity(str, str_end)) {
                render_text(str, str_end);
            };
            break;
        case MD_TEXT_HTML:
            if (!check_html(str, str_end)) {
                render_text(str, str_end);
            }
            break;
        case MD_TEXT_LATEXMATH:
            render_text(str, str_end);
            break;
        default:
            break;
    }

    if (m_is_table_header) {
        const float x = ImGui::GetCursorPosX();
        if (x > m_table_last_pos.x) m_table_last_pos.x = x;
    }

    return 0;
}

int markdown::block(MD_BLOCKTYPE type, void* d, bool e) {
    switch (type) {
        case MD_BLOCK_DOC:
            BLOCK_DOC(e);
            break;
        case MD_BLOCK_QUOTE:
            BLOCK_QUOTE(e);
            break;
        case MD_BLOCK_UL:
            BLOCK_UL((MD_BLOCK_UL_DETAIL*)d, e);
            break;
        case MD_BLOCK_OL:
            BLOCK_OL((MD_BLOCK_OL_DETAIL*)d, e);
            break;
        case MD_BLOCK_LI:
            BLOCK_LI((MD_BLOCK_LI_DETAIL*)d, e);
            break;
        case MD_BLOCK_HR:
            BLOCK_HR(e);
            break;
        case MD_BLOCK_H:
            BLOCK_H((MD_BLOCK_H_DETAIL*)d, e);
            break;
        case MD_BLOCK_CODE:
            BLOCK_CODE((MD_BLOCK_CODE_DETAIL*)d, e);
            break;
        case MD_BLOCK_HTML:
            BLOCK_HTML(e);
            break;
        case MD_BLOCK_P:
            BLOCK_P(e);
            break;
        case MD_BLOCK_TABLE:
            BLOCK_TABLE((MD_BLOCK_TABLE_DETAIL*)d, e);
            break;
        case MD_BLOCK_THEAD:
            BLOCK_THEAD(e);
            break;
        case MD_BLOCK_TBODY:
            BLOCK_TBODY(e);
            break;
        case MD_BLOCK_TR:
            BLOCK_TR(e);
            break;
        case MD_BLOCK_TH:
            BLOCK_TH((MD_BLOCK_TD_DETAIL*)d, e);
            break;
        case MD_BLOCK_TD:
            BLOCK_TD((MD_BLOCK_TD_DETAIL*)d, e);
            break;
        default:
            neko_assert(false);
            break;
    }

    return 0;
}

int markdown::span(MD_SPANTYPE type, void* d, bool e) {
    switch (type) {
        case MD_SPAN_EM:
            SPAN_EM(e);
            break;
        case MD_SPAN_STRONG:
            SPAN_STRONG(e);
            break;
        case MD_SPAN_A:
            SPAN_A((MD_SPAN_A_DETAIL*)d, e);
            break;
        case MD_SPAN_IMG:
            SPAN_IMG((MD_SPAN_IMG_DETAIL*)d, e);
            break;
        case MD_SPAN_CODE:
            SPAN_CODE(e);
            break;
        case MD_SPAN_DEL:
            SPAN_DEL(e);
            break;
        case MD_SPAN_LATEXMATH:
            SPAN_LATEXMATH(e);
            break;
        case MD_SPAN_LATEXMATH_DISPLAY:
            SPAN_LATEXMATH_DISPLAY(e);
            break;
        case MD_SPAN_WIKILINK:
            SPAN_WIKILINK((MD_SPAN_WIKILINK_DETAIL*)d, e);
            break;
        case MD_SPAN_U:
            SPAN_U(e);
            break;
        default:
            neko_assert(false);
            break;
    }

    return 0;
}

int markdown::print(const std::string& str) { return md_parse(str.c_str(), (MD_SIZE)str.size(), &m_md, this); }

////////////////////////////////////////////////////////////////////////////////

ImFont* markdown::get_font() const {
    return nullptr;  // default font

    // Example:
#if 0
    if (m_is_table_header) {
        return g_font_bold;
    }

    switch (m_hlevel)
    {
    case 0:
        return m_is_strong ? g_font_bold : g_font_regular;
    case 1:
        return g_font_bold_large;
    default:
        return g_font_bold;
    }
#endif
};

ImVec4 markdown::get_color() const {
    if (!m_href.empty()) {
        return ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
    }
    return ImGui::GetStyle().Colors[ImGuiCol_Text];
}

bool markdown::get_image(image_info& nfo) const {
    // Use m_href to identify images

    // Example - Imgui font texture
    nfo.texture_id = ImGui::GetIO().Fonts->TexID;
    nfo.size = {100, 50};
    nfo.uv0 = {0, 0};
    nfo.uv1 = {1, 1};
    nfo.col_tint = {1, 1, 1, 1};
    nfo.col_border = {0, 0, 0, 0};

    return true;
};

void markdown::open_url() const {
#if 0   
    if (!m_is_image) {
        SDL_OpenURL(m_href.c_str());
    } else {
        //image clicked
    }
#endif
}

void markdown::soft_break() {
    // Example:
#if 0
    ImGui::NewLine();
#endif
}

bool color_picker_3U32(const char* label, ImU32* color, ImGuiColorEditFlags flags) {
    float col[3];
    col[0] = (float)((*color >> 0) & 0xFF) / 255.0f;
    col[1] = (float)((*color >> 8) & 0xFF) / 255.0f;
    col[2] = (float)((*color >> 16) & 0xFF) / 255.0f;

    bool result = ImGui::ColorPicker3(label, col, flags);

    *color = ((ImU32)(col[0] * 255.0f)) | ((ImU32)(col[1] * 255.0f) << 8) | ((ImU32)(col[2] * 255.0f) << 16);

    return result;
}

void file_browser(std::string& path) {

    ImGui::Text("Current Path: %s", path.c_str());
    ImGui::Separator();

    if (ImGui::Button("Parent Directory")) {
        std::filesystem::path current_path(path);
        if (!current_path.empty()) {
            current_path = current_path.parent_path();
            path = current_path.string();
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        const auto& entry_path = entry.path();
        const auto& filename = entry_path.filename().string();
        if (entry.is_directory()) {
            if (ImGui::Selectable((filename + "/").c_str())) path = entry_path.string();

        } else {
            if (ImGui::Selectable(filename.c_str())) path = entry_path.string();
        }
    }
}

}  // namespace neko::imgui

namespace neko {

void neko_imgui_style() {

    auto& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    const ImVec4 bgColor = neko_imgui_color_from_byte(37, 37, 38);
    const ImVec4 lightBgColor = neko_imgui_color_from_byte(82, 82, 85);
    const ImVec4 veryLightBgColor = neko_imgui_color_from_byte(90, 90, 95);

    const ImVec4 panelColor = neko_imgui_color_from_byte(51, 51, 55);
    const ImVec4 panelHoverColor = neko_imgui_color_from_byte(29, 151, 236);
    const ImVec4 panelActiveColor = neko_imgui_color_from_byte(0, 119, 200);

    const ImVec4 textColor = neko_imgui_color_from_byte(255, 255, 255);
    const ImVec4 textDisabledColor = neko_imgui_color_from_byte(151, 151, 151);
    const ImVec4 borderColor = neko_imgui_color_from_byte(78, 78, 78);

    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDisabledColor;
    colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;
    colors[ImGuiCol_FrameBg] = panelColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_TitleBgCollapsed] = bgColor;
    colors[ImGuiCol_MenuBarBg] = panelColor;
    colors[ImGuiCol_ScrollbarBg] = panelColor;
    colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
    colors[ImGuiCol_CheckMark] = panelActiveColor;
    colors[ImGuiCol_SliderGrab] = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
    colors[ImGuiCol_Button] = panelColor;
    colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    colors[ImGuiCol_ButtonActive] = panelHoverColor;
    colors[ImGuiCol_Header] = panelColor;
    colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    colors[ImGuiCol_HeaderActive] = panelActiveColor;
    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = borderColor;
    colors[ImGuiCol_SeparatorActive] = borderColor;
    colors[ImGuiCol_ResizeGrip] = bgColor;
    colors[ImGuiCol_ResizeGripHovered] = panelColor;
    colors[ImGuiCol_ResizeGripActive] = lightBgColor;
    colors[ImGuiCol_PlotLines] = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
    colors[ImGuiCol_PlotHistogram] = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    // colors[ImGuiCol_ModalWindowDarkening] = bgColor;
    colors[ImGuiCol_DragDropTarget] = bgColor;
    colors[ImGuiCol_NavHighlight] = bgColor;
    colors[ImGuiCol_DockingPreview] = panelActiveColor;
    colors[ImGuiCol_Tab] = bgColor;
    colors[ImGuiCol_TabActive] = panelActiveColor;
    colors[ImGuiCol_TabUnfocused] = bgColor;
    colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
    colors[ImGuiCol_TabHovered] = panelHoverColor;

    style.WindowRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
}

void neko_draw_text_plate(std::string text, neko_color_t col, int x, int y, neko_color_t backcolor) {
    auto text_size = ImGui::CalcTextSize(text.c_str());
    // R_RectangleFilled(target, x - 4, y - 4, x + text_size.x + 4, y + text_size.y + 4, backcolor);
    neko_draw_text(text, col, x, y);
}

u32 neko_draw_darken_color(u32 color, float brightness) {
    int a = (color >> 24) & 0xFF;
    int r = (int)(((color >> 16) & 0xFF) * brightness);
    int g = (int)(((color >> 8) & 0xFF) * brightness);
    int b = (int)((color & 0xFF) * brightness);

    return (a << 24) | (r << 16) | (g << 8) | b;
}

void neko_draw_text(std::string text, neko_color_t col, int x, int y, bool outline, neko_color_t outline_col) {

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList(viewport);

    if (outline) {

        auto outline_col_im = ImColor(outline_col.r, outline_col.g, outline_col.b, col.a);

        draw_list->AddText(ImVec2(x + 0, y - 1), outline_col_im, text.c_str());  // up
        draw_list->AddText(ImVec2(x + 0, y + 1), outline_col_im, text.c_str());  // down
        draw_list->AddText(ImVec2(x + 1, y + 0), outline_col_im, text.c_str());  // right
        draw_list->AddText(ImVec2(x - 1, y + 0), outline_col_im, text.c_str());  // left

        draw_list->AddText(ImVec2(x + 1, y + 1), outline_col_im, text.c_str());  // down-right
        draw_list->AddText(ImVec2(x - 1, y + 1), outline_col_im, text.c_str());  // down-left

        draw_list->AddText(ImVec2(x + 1, y - 1), outline_col_im, text.c_str());  // up-right
        draw_list->AddText(ImVec2(x - 1, y - 1), outline_col_im, text.c_str());  // up-left
    }

    draw_list->AddText(ImVec2(x, y), ImColor(col.r, col.g, col.b, col.a), text.c_str());  // base
}

}  // namespace neko

#if 1

void ShowAutoTestWindow() {

    if (ImGui::Begin("自动序列UI")) {

        auto myCollapsingHeader = [](const char* name) -> bool {
            ImGuiStyle& style = ImGui::GetStyle();
            ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_Button]);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_ButtonHovered]);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_ButtonActive]);
            bool b = ImGui::CollapsingHeader(name);
            ImGui::PopStyleColor(3);
            return b;
        };
        if (myCollapsingHeader("About ImGui::Auto()")) {
            ImGui::Auto(R"comment(
ImGui::Auto() is one simple function that can create GUI for almost any data structure using ImGui functions.
a. Your data is presented in tree-like structure that is defined by your type.
    The generated code is highly efficient.
b. Const types are display only, the user cannot modify them.
    Use ImGui::as_cost() to permit the user to modify a non-const type.
The following types are supported (with nesting):
1 Strings. Flavours of std::string and char*. Use std::string for input.
2 Numbers. Integers, Floating points, ImVec{2,4}
3 STL Containers. Standard containers are supported. (std::vector, std::map,...)
The contained type has to be supported. If it is not, see 8.
4 Pointers, arrays. Pointed type must be supported.
5 std::pair, std::tuple. Types used must be supported.
6 structs and simple classes! The struct is converted to a tuple, and displayed as such.
    * Requirements: C++14 compiler (GCC-5.0+, Clang, Visual Studio 2017 with /std:c++17, ...)
    * How? I'm using this libary https://github.com/apolukhin/magic_get (ImGui::Auto() is only VS2017 C++17 tested.)
7 Functions. A void(void) type function becomes simple button.
For other types, you can input the arguments and calculate the result.
Not yet implemented!
8 You can define ImGui::Auto() for your own type! Use the following macros:
    * ME_GUI_DEFINE_INLINE(template_spec, type_spec, code)  single line definition, teplates shouldn't have commas inside!
    * ME_GUI_DEFINE_BEGIN(template_spec, type_spec)        start multiple line definition, no commas in arguments!
    * ME_GUI_DEFINE_BEGIN_P(template_spec, type_spec)      start multiple line definition, can have commas, use parentheses!
    * ME_GUI_DEFINE_END                                    end multiple line definition with this.
where
    * template_spec   describes how the type is templated. For fully specialized, use "template<>" only
    * type_spec       is the type for witch you define the ImGui::Auto() function.
    * var             will be the generated function argument of type type_spec.
    * name            is the const std::string& given by the user, and/or generated by the caller ImGui::Auto function
Example:           ME_GUI_DEFINE_INLINE(template<>, bool, ImGui::Checkbox(name.c_str(), &var);)
Tipps: - You may use ImGui::Auto_t<type>::Auto(var, name) functions directly.
        - Check imdetail namespace for other helper functions.

The libary uses partial template specialization, so definitions can overlap, the most specialized will be chosen.
However, using large, nested structs can lead to slow compilation.
Container data, large structs and tuples are hidden by default.
)comment");
            if (ImGui::TreeNode("TODO-s")) {
                ImGui::Auto(R"todos(
    1   Insert items to (non-const) set, map, list, ect
    2   Call any function or function object.
    3   Deduce function arguments as a tuple. User can edit the arguments, call the function (button), and view return value.
    4   All of the above needs self ImGui::Auto() allocated memmory. Current plan is to prioritize low memory usage
        over user experiance. (Beacuse if you want good UI, you code it, not generate it.)
        Plan A:
            a)  Data is stored in a map. Key can be the presneted object's address.
            b)  Data is allocated when not already present in the map
            c)  std::unique_ptr<void, deleter> is used in the map with deleter function manually added (or equivalent something)
            d)  The first call for ImGui::Auto at every few frames should delete
                (some of, or) all data that was not accesed in the last (few) frames. How?
        This means after closing a TreeNode, and opening it, the temporary values might be deleted and recreated.
)todos");
                ImGui::TreePop();
            }
        }
        if (myCollapsingHeader("1. String")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    ImGui::Auto("Hello ImAuto() !"); //This is how this text is written as well.)code");
            ImGui::Auto("Hello ImAuto() !");  // This is how this text is written as well.

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::string str = "Hello ImGui::Auto() for strings!";
    ImGui::Auto(str, "asd");)code");
            static std::string str = "Hello ImGui::Auto() for strings!";
            ImGui::Auto(str, "str");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::string str2 = "ImGui::Auto()\n Automatically uses multiline input for strings!\n:)";
    ImGui::Auto(str2, "str2");)code");
            static std::string str2 = "ImGui::Auto()\n Automatically uses multiline input for strings!\n:)";
            ImGui::Auto(str2, "str2");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const std::string conststr = "Const types are not to be changed!";
    ImGui::Auto(conststr, "conststr");)code");
            static const std::string conststr = "Const types are not to be changed!";
            ImGui::Auto(conststr, "conststr");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    char * buffer = "To edit a string use std::string. Manual buffers are unwelcome here.";
    ImGui::Auto(buffer, "buffer");)code");
            char* buffer = "To edit a string use std::string. Manual buffers are unwelcome here.";
            ImGui::Auto(buffer, "buffer");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("2. Numbers")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    static int i = 42;
    ImGui::Auto(i, "i");)code");
            static int i = 42;
            ImGui::Auto(i, "i");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static float f = 3.14;
    ImGui::Auto(f, "f");)code");
            static float f = 3.14;
            ImGui::Auto(f, "f");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static ImVec4 f4 = {1.5f,2.1f,3.4f,4.3f};
    ImGui::Auto(f4, "f4");)code");
            static ImVec4 f4 = {1.5f, 2.1f, 3.4f, 4.3f};
            ImGui::Auto(f4, "f4");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const ImVec2 f2 = {1.f,2.f};
    ImGui::Auto(f2, "f2");)code");
            static const ImVec2 f2 = {1.f, 2.f};
            ImGui::Auto(f2, "f2");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("3. Containers")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    static std::vector<std::string> vec = { "First string","Second str",":)" };
    ImGui::Auto(vec,"vec");)code");
            static std::vector<std::string> vec = {"First string", "Second str", ":)"};
            ImGui::Auto(vec, "vec");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const std::vector<float> constvec = { 3,1,2.1f,4,3,4,5 };
    ImGui::Auto(constvec,"constvec");   //Cannot change vector, nor values)code");
            static const std::vector<float> constvec = {3, 1, 2.1f, 4, 3, 4, 5};
            ImGui::Auto(constvec, "constvec");  // Cannot change vector, nor values

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::vector<bool> bvec = { false, true, false, false };
    ImGui::Auto(bvec,"bvec");)code");
            static std::vector<bool> bvec = {false, true, false, false};
            ImGui::Auto(bvec, "bvec");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const std::vector<bool> constbvec = { false, true, false, false };
    ImGui::Auto(constbvec,"constbvec");
    )code");
            static const std::vector<bool> constbvec = {false, true, false, false};
            ImGui::Auto(constbvec, "constbvec");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::map<int, float> map = { {3,2},{1,2} };
    ImGui::Auto(map, "map");    // insert and other operations)code");
            static std::map<int, float> map = {{3, 2}, {1, 2}};
            ImGui::Auto(map, "map");  // insert and other operations

            if (ImGui::TreeNode("All cases")) {
                ImGui::Auto(R"code(
    static std::deque<bool> deque = { false, true, false, false };
    ImGui::Auto(deque,"deque");)code");
                static std::deque<bool> deque = {false, true, false, false};
                ImGui::Auto(deque, "deque");

                ImGui::NewLine();
                ImGui::Separator();

                ImGui::Auto(R"code(
    static std::set<char*> set = { "set","with","char*" };
    ImGui::Auto(set,"set");)code");
                static std::set<char*> set = {"set", "with", "char*"};  // for some reason, this does not work
                ImGui::Auto(set, "set");                                // the problem is with the const iterator, but

                ImGui::NewLine();
                ImGui::Separator();

                ImGui::Auto(R"code(
    static std::map<char*, std::string> map = { {"asd","somevalue"},{"bsd","value"} };
    ImGui::Auto(map, "map");    // insert and other operations)code");
                static std::map<char*, std::string> map = {{"asd", "somevalue"}, {"bsd", "value"}};
                ImGui::Auto(map, "map");  // insert and other operations

                ImGui::TreePop();
            }
            ImGui::Unindent();
        }
        if (myCollapsingHeader("4. Pointers and Arrays")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    static float *pf = nullptr;
    ImGui::Auto(pf, "pf");)code");
            static float* pf = nullptr;
            ImGui::Auto(pf, "pf");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static int i=10, *pi=&i;
    ImGui::Auto(pi, "pi");)code");
            static int i = 10, *pi = &i;
            ImGui::Auto(pi, "pi");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static const std::string cs= "I cannot be changed!", * cps=&cs;
    ImGui::Auto(cps, "cps");)code");
            static const std::string cs = "I cannot be changed!", *cps = &cs;
            ImGui::Auto(cps, "cps");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::string str = "I can be changed! (my pointee cannot)";
    static std::string *const strpc = &str;)code");
            static std::string str = "I can be changed! (my pointee cannot)";
            static std::string* const strpc = &str;
            ImGui::Auto(strpc, "strpc");

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::Auto(R"code(
    static std::array<float,5> farray = { 1.2, 3.4, 5.6, 7.8, 9.0 };
    ImGui::Auto(farray, "std::array");)code");
            static std::array<float, 5> farray = {1.2, 3.4, 5.6, 7.8, 9.0};
            ImGui::Auto(farray, "std::array");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static float farr[5] = { 1.2, 3.4, 5.6, 7.8, 9.0 };
    ImGui::Auto(farr, "float[5]");)code");
            static float farr[5] = {11.2, 3.4, 5.6, 7.8, 911.0};
            ImGui::Auto(farr, "float[5]");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("5. Pairs and Tuples")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    static std::pair<bool, ImVec2> pair = { true,{2.1f,3.2f} };
    ImGui::Auto(pair, "pair");)code");
            static std::pair<bool, ImVec2> pair = {true, {2.1f, 3.2f}};
            ImGui::Auto(pair, "pair");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    static std::pair<int, std::string> pair2 = { -3,"simple types appear next to each other in a pair" };
    ImGui::Auto(pair2, "pair2");)code");
            static std::pair<int, std::string> pair2 = {-3, "simple types appear next to each other in a pair"};
            ImGui::Auto(pair2, "pair2");

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    ImGui::Auto(ImGui::as_const(pair), "as_const(pair)"); //easy way to view as const)code");
            ImGui::Auto(ImGui::as_const(pair), "as_const(pair)");  // easy way to view as const

            ImGui::NewLine();
            ImGui::Separator();

            ImGui::Auto(R"code(
    std::tuple<const int, std::string, ImVec2> tuple = { 42, "string in tuple", {3.1f,3.2f} };
    ImGui::Auto(tuple, "tuple");)code");
            std::tuple<const int, std::string, ImVec2> tuple = {42, "string in tuple", {3.1f, 3.2f}};
            ImGui::Auto(tuple, "tuple");

            ImGui::NewLine();
            ImGui::Separator();

            //  ImGui::Auto(R"code(
            // const std::tuple<int, const char*, ImVec2> consttuple = { 42, "smaller tuples are inlined", {3.1f,3.2f} };
            // ImGui::Auto(consttuple, "consttuple");)code");
            //  const std::tuple<int, const char*, ImVec2> consttuple = { 42, "Smaller tuples are inlined", {3.1f,3.2f} };
            //  ImGui::Auto(consttuple, "consttuple");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("6. Structs!!")) {
            ImGui::Indent();

            //     ImGui::Auto(R"code(
            // struct A //Structs are automagically converted to tuples!
            // {
            //  int i = 216;
            //  bool b = true;
            // };
            // static A a;
            // ImGui::Auto("a", a);)code");
            //     struct A {
            //         int i = 216;
            //         bool b = true;
            //     };
            //     static A a;
            //     ImGui::Auto(a, "a");

            //     ImGui::NewLine();
            //     ImGui::Separator();

            //     ImGui::Auto(R"code(
            // ImGui::Auto(ImGui::as_const(a), "as_const(a)");// const structs are possible)code");
            //     ImGui::Auto(ImGui::as_const(a), "as_const(a)");

            //     ImGui::NewLine();
            //     ImGui::Separator();

            //     ImGui::Auto(R"code(
            // struct B
            // {
            //  std::string str = "Unfortunatelly, cannot deduce const-ness from within a struct";
            //  const A a = A();
            // };
            // static B b;
            // ImGui::Auto(b, "b");)code");
            //     struct B {
            //         std::string str = "Unfortunatelly, cannot deduce const-ness from within a struct";
            //         const A a = A();
            //     };
            //     static B b;
            //     ImGui::Auto(b, "b");

            //     ImGui::NewLine();
            //     ImGui::Separator();

            //     ImGui::Auto(R"code(
            // static std::vector<B> vec = { {"vector of structs!", A()}, B() };
            // ImGui::Auto(vec, "vec");)code");
            //     static std::vector<B> vec = {{"vector of structs!", A()}, B()};
            //     ImGui::Auto(vec, "vec");

            //     ImGui::NewLine();
            //     ImGui::Separator();

            //     ImGui::Auto(R"code(
            // struct C
            // {
            //  std::list<B> vec;
            //  A *a;
            // };
            // static C c = { {{"Container inside a struct!", A() }}, &a };
            // ImGui::Auto(c, "c");)code");
            //     struct C {
            //         std::list<B> vec;
            //         A *a;
            //     };
            //     static C c = {{{"Container inside a struct!", A()}}, &a};
            //     ImGui::Auto(c, "c");

            ImGui::Unindent();
        }
        if (myCollapsingHeader("Functions")) {
            ImGui::Indent();

            ImGui::Auto(R"code(
    void (*func)() = []() { ImGui::TextColored(ImVec4(0.5, 1, 0.5, 1.0), "Button pressed, function called :)"); };
    ImGui::Auto(func, "void(void) function");)code");
            void (*func)() = []() {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5, 1, 0.5, 1.0), "Button pressed, function called :)");
            };
            ImGui::Auto(func, "void(void) function");

            ImGui::Unindent();
        }
    }
    ImGui::End();
}

#endif

namespace neko {

neko_global std::size_t g_imgui_mem_usage = 0;

neko_private(void*) __neko_imgui_malloc(size_t sz, void* user_data) { return __neko_mem_safe_alloc((sz), (char*)__FILE__, __LINE__, &g_imgui_mem_usage); }

neko_private(void) __neko_imgui_free(void* ptr, void* user_data) { __neko_mem_safe_free(ptr, &g_imgui_mem_usage); }

std::size_t __neko_imgui_meminuse() { return g_imgui_mem_usage; }

void neko_imgui_init() {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Get main window from platform
    GLFWwindow* win = (GLFWwindow*)platform->raw_window_handle(platform->main_window());

    ImGui::SetAllocatorFunctions(__neko_imgui_malloc, __neko_imgui_free);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.IniFilename = "imgui.ini";

    io.Fonts->AddFontFromFileTTF(neko_file_path("data/assets/fonts/fusion-pixel-12px-monospaced.ttf"), 20.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    neko_imgui_style();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init();
}

void neko_imgui_new_frame() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void neko_imgui_render() {
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // TODO: 将这一切抽象出来并通过命令缓冲系统渲染
    ImGui::Render();
    neko_vec2 fbs = platform->frame_buffer_size(platform->main_window());
    glViewport(0, 0, fbs.x, fbs.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void neko_imgui_destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

}  // namespace neko