
#include "neko_editor.hpp"

#include "engine/base/neko_engine.h"
#include "engine/common/neko_hash.h"
#include "engine/filesystem/neko_packer.h"
#include "engine/gui/neko_imgui_utils.hpp"
#include "engine/meta/neko_refl.hpp"
#include "engine/platform/neko_platform.h"
#include "engine/utility/neko_cpp_misc.hpp"
#include "engine/utility/neko_cpp_utils.hpp"

neko_static_inline const char *__build_date = __DATE__;
neko_static_inline const char *mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
neko_static_inline const char mond[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int neko_buildnum(void) {
    int m = 0, d = 0, y = 0;
    static int b = 0;

    // 优化
    if (b != 0) return b;

    for (m = 0; m < 11; m++) {
        if (!strncmp(&__build_date[0], mon[m], 3)) break;
        d += mond[m];
    }

    d += atoi(&__build_date[4]) - 1;
    y = atoi(&__build_date[7]) - 2022;
    b = d + (int)((y - 1) * 365.25f);

    if (((y % 4) == 0) && m > 1) b += 1;

    b -= 151;

    return b;
}

namespace neko {

using namespace neko::cpp;

static const int __profiler_max_level_colors = 11;
static const ImU32 __profiler_level_colors[__profiler_max_level_colors] = {IM_COL32(90, 150, 110, 255), IM_COL32(80, 180, 115, 255),  IM_COL32(129, 195, 110, 255), IM_COL32(170, 190, 100, 255),
                                                                           IM_COL32(210, 200, 80, 255), IM_COL32(230, 210, 115, 255), IM_COL32(240, 180, 90, 255),  IM_COL32(240, 140, 65, 255),
                                                                           IM_COL32(250, 110, 40, 255), IM_COL32(250, 75, 25, 255),   IM_COL32(250, 50, 0, 255)};

static u64 time_since_stat_clicked = neko_profiler_get_clock();
static const char *stat_clicked_name = 0;
static u32 stat_clicked_level = 0;

neko_inline void flash_color(ImU32 &_drawColor, uint64_t _elapsedTime) {
    ImVec4 white4 = ImColor(IM_COL32_WHITE);

    f32 msSince = __neko_profiler_clock2ms(_elapsedTime, __neko_profiler_get_clock_frequency());
    msSince = std::min(msSince, __neko_flash_time_in_ms);
    msSince = 1.0f - (msSince / __neko_flash_time_in_ms);

    ImVec4 col4 = ImColor(_drawColor);
    _drawColor = ImColor(col4.x + (white4.x - col4.x) * msSince, col4.y + (white4.y - col4.y) * msSince, col4.z + (white4.z - col4.z) * msSince, 255.0f);
}

neko_inline void flash_color_named(ImU32 &_drawColor, profiler_scope &_cs, uint64_t _elapsedTime) {
    if (stat_clicked_name && (strcmp(_cs.name, stat_clicked_name) == 0) && (_cs.level == stat_clicked_level)) flash_color(_drawColor, _elapsedTime);
}

neko_static_inline ImVec4 tri_color(f32 _cmp, f32 _min1, f32 _min2) {
    ImVec4 col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    if (_cmp > _min1) col = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    if (_cmp > _min2) col = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    return col;
}

neko_inline struct sort_scopes {
    neko_inline bool operator()(const profiler_scope &a, const profiler_scope &b) const {
        if (a.thread_id < b.thread_id) return true;
        if (b.thread_id < a.thread_id) return false;

        if (a.level < b.level) return true;
        if (b.level < a.level) return false;

        if (a.start < b.start) return true;
        if (b.start < a.start) return false;

        return false;
    }
} customLess;

neko_inline struct sort_frame_info_chrono {
    neko_inline bool operator()(const frame_info &a, const frame_info &b) const {
        if (a.offset < b.offset) return true;
        return false;
    }
} customChrono;

neko_inline struct sort_frame_info_desc {
    neko_inline bool operator()(const frame_info &a, const frame_info &b) const {
        if (a.time > b.time) return true;
        return false;
    }
} customDesc;

neko_inline struct sort_frame_info_asc {
    neko_inline bool operator()(const frame_info &a, const frame_info &b) const {
        if (a.time < b.time) return true;
        return false;
    }
} customAsc;

void profiler_draw_frame_bavigation(frame_info *_infos, uint32_t _numInfos) {
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(1510.0f, 140.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Frame navigator", 0, ImGuiWindowFlags_NoScrollbar);

    static int sortKind = 0;
    ImGui::Text("Sort frames by:  ");
    ImGui::SameLine();
    ImGui::RadioButton("Chronological", &sortKind, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Descending", &sortKind, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Ascending", &sortKind, 2);
    ImGui::Separator();

    switch (sortKind) {
        case 0:
            std::sort(&_infos[0], &_infos[_numInfos], customChrono);
            break;

        case 1:
            std::sort(&_infos[0], &_infos[_numInfos], customDesc);
            break;

        case 2:
            std::sort(&_infos[0], &_infos[_numInfos], customAsc);
            break;
    };

    f32 maxTime = 0;
    for (uint32_t i = 0; i < _numInfos; ++i) {
        if (maxTime < _infos[i].time) maxTime = _infos[i].time;
    }

    const ImVec2 s = ImGui::GetWindowSize();
    const ImVec2 p = ImGui::GetWindowPos();

    ImGui::BeginChild("", ImVec2(s.x, 70), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

    ImGui::PlotHistogram("", (const f32 *)_infos, _numInfos, 0, "", 0.f, maxTime, ImVec2(_numInfos * 10, 50), sizeof(frame_info));

    // if (ImGui::IsMouseClicked(0) && (idx != -1)) {
    //     profilerFrameLoad(g_fileName, _infos[idx].m_offset, _infos[idx].m_size);
    // }

    ImGui::EndChild();

    ImGui::End();
}

int profiler_draw_frame(profiler_frame *_data, void *_buffer, size_t _bufferSize, bool _inGame, bool _multi) {
    int ret = 0;

    // if (fabs(_data->m_startTime - _data->m_endtime) == 0.0f) return ret;

    std::sort(&_data->scopes[0], &_data->scopes[_data->num_scopes], customLess);

    ImGui::SetNextWindowPos(ImVec2(10.0f, _multi ? 160.0f : 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(900.0f, 480.0f), ImGuiCond_FirstUseEver);

    static bool pause = false;
    bool noMove = (pause && _inGame) || !_inGame;
    noMove = noMove && ImGui::GetIO().KeyCtrl;

    neko_private(ImVec2) winpos = ImGui::GetWindowPos();

    if (noMove) ImGui::SetNextWindowPos(winpos);

    ImGui::Begin("ui_profiler", 0, noMove ? ImGuiWindowFlags_NoMove : 0);

    if (!noMove) winpos = ImGui::GetWindowPos();

    f32 deltaTime = __neko_profiler_clock2ms(_data->end_time - _data->start_time, _data->cpu_frequency);
    f32 frameRate = 1000.0f / deltaTime;

    ImVec4 col = tri_color(frameRate, __neko_minimum_frame_rate, __neko_desired_frame_rate);

    ImGui::Text("FPS: ");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::Text("%.1f    ", 1000.0f / deltaTime);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Text("%s", "帧耗时");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::Text("%.3f ms   ", deltaTime);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (_inGame) {
        ImGui::Text("平均帧率: ");
        ImGui::PushStyleColor(ImGuiCol_Text, tri_color(ImGui::GetIO().Framerate, __neko_minimum_frame_rate, __neko_desired_frame_rate));
        ImGui::SameLine();
        ImGui::Text("%.1f   ", ImGui::GetIO().Framerate);
        ImGui::PopStyleColor();
    } else {
        f32 prevFrameTime = __neko_profiler_clock2ms(_data->prev_frame_time, _data->cpu_frequency);
        ImGui::SameLine();
        ImGui::Text("上一帧: ");
        ImGui::PushStyleColor(ImGuiCol_Text, tri_color(1000.0f / prevFrameTime, __neko_minimum_frame_rate, __neko_desired_frame_rate));
        ImGui::SameLine();
        ImGui::Text("%.3f ms  %.2f fps   ", prevFrameTime, 1000.0f / prevFrameTime);
        ImGui::PopStyleColor();
        ImGui::SameLine();

        // ImGui::Text("Platform: ");
        // ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 1.0f, 1.0f, 1.0f));
        // ImGui::SameLine();
        // ImGui::Text("%s   ", ProfilerGetPlatformName(_data->m_platformID));
        // ImGui::PopStyleColor();
    }

    if (_inGame) {
        ImGui::SameLine();
        ImGui::Checkbox("暂停捕获   ", &pause);
    }

    bool resetZoom = false;
    static f32 threshold = 0.0f;
    static int thresholdLevel = 0;

    if (_inGame) {
        ImGui::PushItemWidth(210);
        ImGui::SliderFloat("阈值   ", &threshold, 0.0f, 15.0f);

        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::SliderInt("阈值级别", &thresholdLevel, 0, 23);

        ImGui::SameLine();
        if (ImGui::Button("保存帧")) ret = neko_profiler_save(_data, _buffer, _bufferSize);
    } else {
        ImGui::Text("捕获阈值: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0, 1.0f, 1.0f, 1.0f), "%.2f ", _data->time_threshold);
        ImGui::SameLine();
        ImGui::Text("ms   ");
        ImGui::SameLine();
        ImGui::Text("门限等级: ");
        ImGui::SameLine();
        if (_data->level_threshold == 0)
            ImGui::TextColored(ImVec4(0, 1.0f, 1.0f, 1.0f), "整帧   ");
        else
            ImGui::TextColored(ImVec4(0, 1.0f, 1.0f, 1.0f), "%d   ", _data->level_threshold);
    }

    ImGui::SameLine();
    resetZoom = ImGui::Button("重置缩放和平移");

    const ImVec2 p = ImGui::GetCursorScreenPos();
    const ImVec2 s = ImGui::GetWindowSize();

    f32 frameStartX = p.x + 3.0f;
    f32 frameEndX = frameStartX + s.x - 23;
    f32 frameStartY = p.y;

    static pan_and_zoon paz;

    ImVec2 mpos = ImGui::GetMousePos();

    if (ImGui::IsMouseDragging(0) && ImGui::GetIO().KeyCtrl) {
        paz.offset -= (mpos.x - paz.start_pan);
        paz.start_pan = mpos.x;
    } else
        paz.start_pan = mpos.x;

    f32 mXpre = paz.s2w(mpos.x, frameStartX, frameEndX);

    if (ImGui::GetIO().KeyCtrl) paz.zoom += ImGui::GetIO().MouseWheel / 30.0f;
    if (ImGui::GetIO().KeysDown[65] && ImGui::IsWindowHovered())  // 'a'
        paz.zoom *= 1.1f;
    if (ImGui::GetIO().KeysDown[90] && ImGui::IsWindowHovered())  // 'z'
        paz.zoom /= 1.1f;

    paz.zoom = std::max(paz.zoom, 1.0f);

    f32 mXpost = paz.s2w(mpos.x, frameStartX, frameEndX);
    f32 mXdelta = mXpost - mXpre;

    paz.offset -= paz.w2sdelta(mXdelta, frameStartX, frameEndX);

    // snap to edge
    f32 leX = paz.w2s(0.0f, frameStartX, frameEndX);
    f32 reX = paz.w2s(1.0f, frameStartX, frameEndX);

    if (leX > frameStartX) paz.offset += leX - frameStartX;
    if (reX < frameEndX) paz.offset -= frameEndX - reX;

    if (resetZoom || (_inGame ? !pause : false)) {
        paz.offset = 0.0f;
        paz.zoom = 1.0f;
    }

    neko_profiler_set_paused(pause);
    neko_profiler_set_threshold(threshold, thresholdLevel);

    static const int ME_MAX_FRAME_TIMES = 128;
    static f32 s_frameTimes[ME_MAX_FRAME_TIMES];
    static int s_currentFrame = 0;

    f32 maxFrameTime = 0.0f;
    if (_inGame) {
        frameStartY += 62.0f;

        if (s_currentFrame == 0) memset(s_frameTimes, 0, sizeof(s_frameTimes));

        if (!neko_profiler_is_paused()) {
            s_frameTimes[s_currentFrame % ME_MAX_FRAME_TIMES] = deltaTime;
            ++s_currentFrame;
        }

        f32 frameTimes[ME_MAX_FRAME_TIMES];
        for (int i = 0; i < ME_MAX_FRAME_TIMES; ++i) {
            frameTimes[i] = s_frameTimes[(s_currentFrame + i) % ME_MAX_FRAME_TIMES];
            maxFrameTime = std::max(maxFrameTime, frameTimes[i]);
        }

        ImGui::Separator();
        ImGui::PlotHistogram("", frameTimes, ME_MAX_FRAME_TIMES, 0, "", 0.f, maxFrameTime, ImVec2(s.x - 9.0f, 45));
    } else {
        frameStartY += 12.0f;
        ImGui::Separator();
    }

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    maxFrameTime = std::max(maxFrameTime, 0.001f);
    f32 pct30fps = 33.33f / maxFrameTime;
    f32 pct60fps = 16.66f / maxFrameTime;

    f32 minHistY = p.y + 6.0f;
    f32 maxHistY = p.y + 45.0f;

    f32 limit30Y = maxHistY - (maxHistY - minHistY) * pct30fps;
    f32 limit60Y = maxHistY - (maxHistY - minHistY) * pct60fps;

    if (pct30fps <= 1.0f) draw_list->AddLine(ImVec2(frameStartX - 3.0f, limit30Y), ImVec2(frameEndX + 3.0f, limit30Y), IM_COL32(255, 255, 96, 255));

    if (pct60fps <= 1.0f) draw_list->AddLine(ImVec2(frameStartX - 3.0f, limit60Y), ImVec2(frameEndX + 3.0f, limit60Y), IM_COL32(96, 255, 96, 255));

    if (_data->num_scopes == 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.23f, 0.23f, 1.0f), "没有范围数据!");
        ImGui::End();
        return ret;
    }

    uint64_t threadID = _data->scopes[0].thread_id;
    bool writeThreadName = true;

    uint64_t totalTime = _data->end_time - _data->start_time;

    f32 barHeight = 21.0f;
    f32 bottom = 0.0f;

    uint64_t currTime = neko_profiler_get_clock();

    for (uint32_t i = 0; i < _data->num_scopes; ++i) {
        profiler_scope &cs = _data->scopes[i];
        if (!cs.name) continue;

        if (cs.thread_id != threadID) {
            threadID = cs.thread_id;
            frameStartY = bottom + barHeight;
            writeThreadName = true;
        }

        if (writeThreadName) {
            ImVec2 tlt = ImVec2(frameStartX, frameStartY);
            ImVec2 brt = ImVec2(frameEndX, frameStartY + barHeight);

            draw_list->PushClipRect(tlt, brt, true);
            draw_list->AddRectFilled(tlt, brt, IM_COL32(45, 45, 60, 255));
            const char *threadName = "Unnamed thread";
            for (uint32_t j = 0; j < _data->num_threads; ++j)
                if (_data->threads[j].thread_id == threadID) {
                    threadName = _data->threads[j].name;
                    break;
                }
            tlt.x += 3;
            char buffer[512];
            snprintf(buffer, 512, "%s  -  0x%" PRIx64, threadName, threadID);
            draw_list->AddText(tlt, IM_COL32(255, 255, 255, 255), buffer);
            draw_list->PopClipRect();

            frameStartY += barHeight;
            writeThreadName = false;
        }

        // handle wrap around
        int64_t sX = int64_t(cs.start - _data->start_time);
        if (sX < 0) sX = -sX;
        int64_t eX = int64_t(cs.end - _data->start_time);
        if (eX < 0) eX = -eX;

        f32 startXpct = f32(sX) / f32(totalTime);
        f32 endXpct = f32(eX) / f32(totalTime);

        f32 startX = paz.w2s(startXpct, frameStartX, frameEndX);
        f32 endX = paz.w2s(endXpct, frameStartX, frameEndX);

        ImVec2 tl = ImVec2(startX, frameStartY + cs.level * (barHeight + 1.0f));
        ImVec2 br = ImVec2(endX, frameStartY + cs.level * (barHeight + 1.0f) + barHeight);

        bottom = std::max(bottom, br.y);

        int level = cs.level;
        if (cs.level >= __profiler_max_level_colors) level = __profiler_max_level_colors - 1;

        ImU32 drawColor = __profiler_level_colors[level];
        flash_color_named(drawColor, cs, currTime - time_since_stat_clicked);

        if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(tl, br) && ImGui::IsWindowHovered()) {
            time_since_stat_clicked = currTime;
            stat_clicked_name = cs.name;
            stat_clicked_level = cs.level;
        }

        if ((thresholdLevel == (int)cs.level + 1) && (threshold <= __neko_profiler_clock2ms(cs.end - cs.start, _data->cpu_frequency))) flash_color(drawColor, currTime - _data->end_time);

        draw_list->PushClipRect(tl, br, true);
        draw_list->AddRectFilled(tl, br, drawColor);
        tl.x += 3;
        draw_list->AddText(tl, IM_COL32(0, 0, 0, 255), cs.name);
        draw_list->PopClipRect();

        if (ImGui::IsMouseHoveringRect(tl, br) && ImGui::IsWindowHovered()) {
            ImGui::BeginTooltip();
            neko_defer([] { ImGui::EndTooltip(); });
            ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s", cs.name);
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0, 255, 255, 255), "Time: ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(230, 230, 230, 255), "%.3f ms", __neko_profiler_clock2ms(cs.end - cs.start, _data->cpu_frequency));
            ImGui::TextColored(ImVec4(0, 255, 255, 255), "File: ");
            ImGui::SameLine();
            ImGui::Text("%s", cs.file);
            ImGui::TextColored(ImVec4(0, 255, 255, 255), "%s", "Line: ");
            ImGui::SameLine();
            ImGui::Text("%d", cs.line);
        }
    }

    ImGui::End();

    return ret;
}

void profiler_draw_stats(profiler_frame *_data, bool _multi) {
    ImGui::SetNextWindowPos(ImVec2(920.0f, _multi ? 160.0f : 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600.0f, 900.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Frame stats");

    if (_data->num_scopes == 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.23f, 0.23f, 1.0f), "No scope data!");
        ImGui::End();
        return;
    }

    f32 deltaTime = __neko_profiler_clock2ms(_data->end_time - _data->start_time, _data->cpu_frequency);

    static int exclusive = 0;
    ImGui::Text("Sort by:  ");
    ImGui::SameLine();
    ImGui::RadioButton("Exclusive time", &exclusive, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Inclusive time", &exclusive, 1);
    ImGui::Separator();

    struct sort_excusive {
        bool operator()(const profiler_scope &a, const profiler_scope &b) const { return (a.stats->exclusive_time_total > b.stats->exclusive_time_total); }
    } customLessExc;

    struct sort_inclusive {
        bool operator()(const profiler_scope &a, const profiler_scope &b) const { return (a.stats->inclusive_time_total > b.stats->inclusive_time_total); }
    } customLessInc;

    if (exclusive == 0)
        std::sort(&_data->scopes_stats[0], &_data->scopes_stats[_data->num_scopes_stats], customLessExc);
    else
        std::sort(&_data->scopes_stats[0], &_data->scopes_stats[_data->num_scopes_stats], customLessInc);

    const ImVec2 p = ImGui::GetCursorScreenPos();
    const ImVec2 s = ImGui::GetWindowSize();

    f32 frameStartX = p.x + 3.0f;
    f32 frameEndX = frameStartX + s.x - 23;
    f32 frameStartY = p.y + 21.0f;

    uint64_t totalTime = 0;
    if (exclusive == 0)
        totalTime = _data->scopes_stats[0].stats->exclusive_time_total;
    else
        totalTime = _data->scopes_stats[0].stats->inclusive_time_total;

    f32 barHeight = 21.0f;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    for (uint32_t i = 0; i < _data->num_scopes_stats; i++) {
        profiler_scope &cs = _data->scopes_stats[i];

        f32 endXpct = f32(cs.stats->exclusive_time_total) / f32(totalTime);
        if (exclusive == 1) endXpct = f32(cs.stats->inclusive_time_total) / f32(totalTime);

        f32 startX = frameStartX;
        f32 endX = frameStartX + endXpct * (frameEndX - frameStartX);

        ImVec2 tl = ImVec2(startX, frameStartY);
        ImVec2 br = ImVec2(endX, frameStartY + barHeight);

        int colIdx = __profiler_max_level_colors - 1 - i;
        if (colIdx < 0) colIdx = 0;
        ImU32 drawColor = __profiler_level_colors[colIdx];

        ImVec2 brE = ImVec2(frameEndX, frameStartY + barHeight);
        bool hoverRow = ImGui::IsMouseHoveringRect(tl, brE);

        if (ImGui::IsMouseClicked(0) && hoverRow && ImGui::IsWindowHovered()) {
            time_since_stat_clicked = neko_profiler_get_clock();
            stat_clicked_name = cs.name;
            stat_clicked_level = cs.level;
        }

        flash_color_named(drawColor, cs, neko_profiler_get_clock() - time_since_stat_clicked);

        char buffer[1024];
        snprintf(buffer, 1024, "[%d] %s", cs.stats->occurences, cs.name);
        draw_list->PushClipRect(tl, br, true);
        draw_list->AddRectFilled(tl, br, drawColor);
        draw_list->AddText(tl, IM_COL32(0, 0, 0, 255), buffer);
        draw_list->AddText(ImVec2(startX + 1, frameStartY + 1), IM_COL32(255, 255, 255, 255), buffer);
        draw_list->PopClipRect();

        if (hoverRow && ImGui::IsWindowHovered()) {
            ImVec2 htl = ImVec2(endX, frameStartY);
            draw_list->PushClipRect(htl, brE, true);
            draw_list->AddRectFilled(htl, brE, IM_COL32(64, 64, 64, 255));
            draw_list->AddText(tl, IM_COL32(0, 0, 0, 255), buffer);
            draw_list->AddText(ImVec2(startX + 1, frameStartY + 1), IM_COL32(192, 192, 192, 255), buffer);
            draw_list->PopClipRect();

            ImGui::BeginTooltip();
            ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s", cs.name);
            ImGui::Separator();

            f32 ttime;
            if (exclusive == 0) {
                ImGui::TextColored(ImVec4(0, 255, 255, 255), "Exclusive time total: ");
                ImGui::SameLine();
                ttime = __neko_profiler_clock2ms(cs.stats->exclusive_time_total, _data->cpu_frequency);
                ImGui::TextColored(ImVec4(230, 230, 230, 255), "%.4f ms", ttime);
            } else {
                ImGui::TextColored(ImVec4(0, 255, 255, 255), "Inclusive time total: ");
                ImGui::SameLine();
                ttime = __neko_profiler_clock2ms(cs.stats->inclusive_time_total, _data->cpu_frequency);
                ImGui::TextColored(ImVec4(230, 230, 230, 255), "%.4f ms", ttime);
            }

            ImGui::TextColored(ImVec4(0, 255, 255, 255), "Of frame: ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(230, 230, 230, 255), "%2.2f %%", 100.0f * ttime / deltaTime);

            ImGui::TextColored(ImVec4(0, 255, 255, 255), "File: ");
            ImGui::SameLine();
            ImGui::Text("%s", cs.file);

            ImGui::TextColored(ImVec4(0, 255, 255, 255), "Line: ");
            ImGui::SameLine();
            ImGui::Text("%d", cs.line);

            ImGui::TextColored(ImVec4(0, 255, 255, 255), "Count: ");
            ImGui::SameLine();
            ImGui::Text("%d", cs.stats->occurences);

            ImGui::EndTooltip();
        }

        frameStartY += 1.0f + barHeight;
    }

    ImGui::End();
}

#define __neko_gl_to_string_def(x) \
    case x:                        \
        return #x;                 \
        break;

const char *neko_gpu_glenum_string(GLenum e) {
    switch (e) {
        // shader:
        __neko_gl_to_string_def(GL_VERTEX_SHADER);
        __neko_gl_to_string_def(GL_GEOMETRY_SHADER);
        __neko_gl_to_string_def(GL_FRAGMENT_SHADER);

        // buffer usage:
        __neko_gl_to_string_def(GL_STREAM_DRAW);
        __neko_gl_to_string_def(GL_STREAM_READ);
        __neko_gl_to_string_def(GL_STREAM_COPY);
        __neko_gl_to_string_def(GL_STATIC_DRAW);
        __neko_gl_to_string_def(GL_STATIC_READ);
        __neko_gl_to_string_def(GL_STATIC_COPY);
        __neko_gl_to_string_def(GL_DYNAMIC_DRAW);
        __neko_gl_to_string_def(GL_DYNAMIC_READ);
        __neko_gl_to_string_def(GL_DYNAMIC_COPY);

        // errors:
        __neko_gl_to_string_def(GL_NO_ERROR);
        __neko_gl_to_string_def(GL_INVALID_ENUM);
        __neko_gl_to_string_def(GL_INVALID_VALUE);
        __neko_gl_to_string_def(GL_INVALID_OPERATION);
        __neko_gl_to_string_def(GL_INVALID_FRAMEBUFFER_OPERATION);
        __neko_gl_to_string_def(GL_OUT_OF_MEMORY);
        __neko_gl_to_string_def(GL_STACK_UNDERFLOW);
        __neko_gl_to_string_def(GL_STACK_OVERFLOW);

        // types:
        __neko_gl_to_string_def(GL_BYTE);
        __neko_gl_to_string_def(GL_UNSIGNED_BYTE);
        __neko_gl_to_string_def(GL_SHORT);
        __neko_gl_to_string_def(GL_UNSIGNED_SHORT);
        __neko_gl_to_string_def(GL_FLOAT);
        __neko_gl_to_string_def(GL_FLOAT_VEC2);
        __neko_gl_to_string_def(GL_FLOAT_VEC3);
        __neko_gl_to_string_def(GL_FLOAT_VEC4);
        __neko_gl_to_string_def(GL_DOUBLE);
        __neko_gl_to_string_def(GL_DOUBLE_VEC2);
        __neko_gl_to_string_def(GL_DOUBLE_VEC3);
        __neko_gl_to_string_def(GL_DOUBLE_VEC4);
        __neko_gl_to_string_def(GL_INT);
        __neko_gl_to_string_def(GL_INT_VEC2);
        __neko_gl_to_string_def(GL_INT_VEC3);
        __neko_gl_to_string_def(GL_INT_VEC4);
        __neko_gl_to_string_def(GL_UNSIGNED_INT);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_VEC2);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_VEC3);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_VEC4);
        __neko_gl_to_string_def(GL_BOOL);
        __neko_gl_to_string_def(GL_BOOL_VEC2);
        __neko_gl_to_string_def(GL_BOOL_VEC3);
        __neko_gl_to_string_def(GL_BOOL_VEC4);
        __neko_gl_to_string_def(GL_FLOAT_MAT2);
        __neko_gl_to_string_def(GL_FLOAT_MAT3);
        __neko_gl_to_string_def(GL_FLOAT_MAT4);
        __neko_gl_to_string_def(GL_FLOAT_MAT2x3);
        __neko_gl_to_string_def(GL_FLOAT_MAT2x4);
        __neko_gl_to_string_def(GL_FLOAT_MAT3x2);
        __neko_gl_to_string_def(GL_FLOAT_MAT3x4);
        __neko_gl_to_string_def(GL_FLOAT_MAT4x2);
        __neko_gl_to_string_def(GL_FLOAT_MAT4x3);
        __neko_gl_to_string_def(GL_DOUBLE_MAT2);
        __neko_gl_to_string_def(GL_DOUBLE_MAT3);
        __neko_gl_to_string_def(GL_DOUBLE_MAT4);
        __neko_gl_to_string_def(GL_DOUBLE_MAT2x3);
        __neko_gl_to_string_def(GL_DOUBLE_MAT2x4);
        __neko_gl_to_string_def(GL_DOUBLE_MAT3x2);
        __neko_gl_to_string_def(GL_DOUBLE_MAT3x4);
        __neko_gl_to_string_def(GL_DOUBLE_MAT4x2);
        __neko_gl_to_string_def(GL_DOUBLE_MAT4x3);
        __neko_gl_to_string_def(GL_SAMPLER_1D);
        __neko_gl_to_string_def(GL_SAMPLER_2D);
        __neko_gl_to_string_def(GL_SAMPLER_3D);
        __neko_gl_to_string_def(GL_SAMPLER_CUBE);
        __neko_gl_to_string_def(GL_SAMPLER_1D_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_2D_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_1D_ARRAY);
        __neko_gl_to_string_def(GL_SAMPLER_2D_ARRAY);
        __neko_gl_to_string_def(GL_SAMPLER_1D_ARRAY_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_2D_ARRAY_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_SAMPLER_CUBE_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_BUFFER);
        __neko_gl_to_string_def(GL_SAMPLER_2D_RECT);
        __neko_gl_to_string_def(GL_SAMPLER_2D_RECT_SHADOW);
        __neko_gl_to_string_def(GL_INT_SAMPLER_1D);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D);
        __neko_gl_to_string_def(GL_INT_SAMPLER_3D);
        __neko_gl_to_string_def(GL_INT_SAMPLER_CUBE);
        __neko_gl_to_string_def(GL_INT_SAMPLER_1D_ARRAY);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D_ARRAY);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_INT_SAMPLER_BUFFER);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D_RECT);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_1D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_3D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_CUBE);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_BUFFER);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D_RECT);

        __neko_gl_to_string_def(GL_IMAGE_1D);
        __neko_gl_to_string_def(GL_IMAGE_2D);
        __neko_gl_to_string_def(GL_IMAGE_3D);
        __neko_gl_to_string_def(GL_IMAGE_2D_RECT);
        __neko_gl_to_string_def(GL_IMAGE_CUBE);
        __neko_gl_to_string_def(GL_IMAGE_BUFFER);
        __neko_gl_to_string_def(GL_IMAGE_1D_ARRAY);
        __neko_gl_to_string_def(GL_IMAGE_2D_ARRAY);
        __neko_gl_to_string_def(GL_IMAGE_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_IMAGE_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_INT_IMAGE_1D);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D);
        __neko_gl_to_string_def(GL_INT_IMAGE_3D);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D_RECT);
        __neko_gl_to_string_def(GL_INT_IMAGE_CUBE);
        __neko_gl_to_string_def(GL_INT_IMAGE_BUFFER);
        __neko_gl_to_string_def(GL_INT_IMAGE_1D_ARRAY);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D_ARRAY);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_1D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_3D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D_RECT);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_CUBE);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_BUFFER);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_ATOMIC_COUNTER);
    }

    static char buffer[32];
    std::sprintf(buffer, "Unknown GLenum: (0x%04x)", e);
    return buffer;
}

void neko_editor_render_uniform_variable(GLuint program, GLenum type, const_str name, GLint location) {
    static bool is_color = false;
    switch (type) {
        case GL_FLOAT:
            neko_editor_shader_inspector_gen_var(GLfloat, 1, GL_FLOAT, glGetUniformfv, glProgramUniform1fv, ImGui::DragFloat);
            break;

        case GL_FLOAT_VEC2:
            neko_editor_shader_inspector_gen_var(GLfloat, 2, GL_FLOAT_VEC2, glGetUniformfv, glProgramUniform2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_VEC3: {
            ImGui::Checkbox("##is_color", &is_color);
            ImGui::SameLine();
            ImGui::Text("GL_FLOAT_VEC3 %s", name);
            ImGui::SameLine();
            f32 value[3];
            glGetUniformfv(program, location, &value[0]);
            if ((!is_color && ImGui::DragFloat3("", &value[0])) || (is_color && ImGui::ColorEdit3("Color", &value[0], ImGuiColorEditFlags_NoLabel)))
                glProgramUniform3fv(program, location, 1, &value[0]);
        } break;

        case GL_FLOAT_VEC4: {
            ImGui::Checkbox("##is_color", &is_color);
            ImGui::SameLine();
            ImGui::Text("GL_FLOAT_VEC4 %s", name);
            ImGui::SameLine();
            f32 value[4];
            glGetUniformfv(program, location, &value[0]);
            if ((!is_color && ImGui::DragFloat4("", &value[0])) ||
                (is_color && ImGui::ColorEdit4("Color", &value[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)))
                glProgramUniform4fv(program, location, 1, &value[0]);
        } break;

        case GL_INT:
            neko_editor_shader_inspector_gen_var(GLint, 1, GL_INT, glGetUniformiv, glProgramUniform1iv, ImGui::DragInt);
            break;

        case GL_INT_VEC2:
            neko_editor_shader_inspector_gen_var(GLint, 2, GL_INT, glGetUniformiv, glProgramUniform2iv, ImGui::DragInt2);
            break;

        case GL_INT_VEC3:
            neko_editor_shader_inspector_gen_var(GLint, 3, GL_INT, glGetUniformiv, glProgramUniform3iv, ImGui::DragInt3);
            break;

        case GL_INT_VEC4:
            neko_editor_shader_inspector_gen_var(GLint, 4, GL_INT, glGetUniformiv, glProgramUniform4iv, ImGui::DragInt4);
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
            neko_editor_shader_inspector_gen_var(GLint, 1, GL_SAMPLER_2D, glGetUniformiv, glProgramUniform1iv, ImGui::DragInt);
            break;

        case GL_FLOAT_MAT2:
            neko_editor_shader_inspector_gen_mat(GLfloat, 2, 2, GL_FLOAT_MAT2, glGetUniformfv, glProgramUniformMatrix2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_MAT3:
            neko_editor_shader_inspector_gen_mat(GLfloat, 3, 3, GL_FLOAT_MAT3, glGetUniformfv, glProgramUniformMatrix3fv, ImGui::DragFloat3);
            break;

        case GL_FLOAT_MAT4:
            neko_editor_shader_inspector_gen_mat(GLfloat, 4, 4, GL_FLOAT_MAT4, glGetUniformfv, glProgramUniformMatrix4fv, ImGui::DragFloat4);
            break;

        case GL_FLOAT_MAT2x3:
            neko_editor_shader_inspector_gen_mat(GLfloat, 3, 2, GL_FLOAT_MAT2x3, glGetUniformfv, glProgramUniformMatrix2x3fv, ImGui::DragFloat3);
            break;

        case GL_FLOAT_MAT2x4:
            neko_editor_shader_inspector_gen_mat(GLfloat, 4, 2, GL_FLOAT_MAT2x4, glGetUniformfv, glProgramUniformMatrix2x4fv, ImGui::DragFloat4);
            break;

        case GL_FLOAT_MAT3x2:
            neko_editor_shader_inspector_gen_mat(GLfloat, 2, 3, GL_FLOAT_MAT3x2, glGetUniformfv, glProgramUniformMatrix3x2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_MAT3x4:
            neko_editor_shader_inspector_gen_mat(GLfloat, 4, 3, GL_FLOAT_MAT3x4, glGetUniformfv, glProgramUniformMatrix3x2fv, ImGui::DragFloat4);
            break;

        case GL_BOOL: {
            ImGui::Text("GL_BOOL %s:", name);
            ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            if (ImGui::Checkbox("", (bool *)&value)) glProgramUniform1uiv(program, location, 1, &value);
        } break;

        case GL_IMAGE_2D: {
            ImGui::Text("GL_IMAGE_2D %s:", name);
            // ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            // if (ImGui::Checkbox("", (bool*)&value)) glProgramUniform1iv(program, location, 1, &value);
            ImGui::Image((void *)(intptr_t)value, ImVec2(256, 256));
        } break;

        case GL_SAMPLER_CUBE: {
            ImGui::Text("GL_SAMPLER_CUBE %s:", name);
            // ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            ImGui::Image((void *)(intptr_t)value, ImVec2(256, 256));
        } break;

        default:
            ImGui::TextColored(neko_rgba2imvec(255, 64, 64), "%s has type %s, which isn't supported yet!", name, neko_gpu_glenum_string(type));
            break;
    }
}

neko_inline f32 get_scrollable_height() { return ImGui::GetTextLineHeight() * 16; }

void neko_editor_inspect_shader(const_str label, GLuint program) {
    neko_assert(label != nullptr, ("The label supplied with program: {} is nullptr", program));
    neko_assert(glIsProgram(program), ("The program: {} is not a valid shader program", program));

    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
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
                neko_editor_render_uniform_variable(program, type, name.data(), location);
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

            for (const auto &shader : attached_shaders) {
                GLint source_length = 0;
                glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &source_length);
                static std::vector<char> source;
                source.resize(source_length);
                glGetShaderSource(shader, source_length, nullptr, source.data());

                GLint type = 0;
                glGetShaderiv(shader, GL_SHADER_TYPE, &type);

                ImGui::Indent();
                auto string_type = neko_gpu_glenum_string(type);
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
    ImGui::PopID();
}

void neko_editor_inspect_vertex_array(const_str label, GLuint vao) {
    neko_assert(label != nullptr, ("The label supplied with VAO: %u is nullptr", vao));
    neko_assert(glIsVertexArray(vao), ("The VAO: %u is not a valid vertex array object", vao));

    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
        ImGui::Indent();

        // Get current bound vertex buffer object so we can reset it back once we are finished.
        GLint current_vbo = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_vbo);

        // Get current bound vertex array object so we can reset it back once we are finished.
        GLint current_vao = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
        glBindVertexArray(vao);

        // Get the maximum number of vertex attributes,
        // minimum is 4, I have 16, means that whatever number of attributes is here, it should be reasonable to iterate over.
        GLint max_vertex_attribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);

        GLint ebo = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo);

        // EBO Visualization
        char buffer[128];
        std::sprintf(buffer, "Element Array Buffer: %d", ebo);
        ImGui::PushID(buffer);
        if (ImGui::CollapsingHeader(buffer)) {
            ImGui::Indent();
            // Assuming unsigned int atm, as I have not found a way to get out the type of the element array buffer.
            int size = 0;
            glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
            size /= sizeof(GLuint);
            ImGui::Text("Size: %d", size);

            if (ImGui::TreeNode("Buffer Contents")) {
                // TODO: Find a better way to put this out on screen, because this solution will probably not scale good when we get a lot of indices.
                //       Possible solution: Make it into columns, like the VBO's, and present the indices as triangles.
                auto ptr = (GLuint *)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
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

            std::sprintf(buffer, "Attribute: %" PRIdPTR "", i);
            ImGui::PushID(buffer);
            if (ImGui::CollapsingHeader(buffer)) {
                ImGui::Indent();
                // Display meta data
                GLint buffer = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
                ImGui::Text("Buffer: %d", buffer);

                GLint type = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
                ImGui::Text("Type: %s", neko_gpu_glenum_string(type));

                GLint dimensions = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &dimensions);
                ImGui::Text("Dimensions: %d", dimensions);

                // Need to bind buffer to get access to parameteriv, and for mapping later
                glBindBuffer(GL_ARRAY_BUFFER, buffer);

                GLint size = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
                ImGui::Text("Size in bytes: %d", size);

                GLint stride = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
                ImGui::Text("Stride in bytes: %d", stride);

                GLvoid *offset = nullptr;
                glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &offset);
                ImGui::Text("Offset in bytes: %" PRIdPTR "", (intptr_t)offset);

                GLint usage = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_USAGE, &usage);
                ImGui::Text("Usage: %s", neko_gpu_glenum_string(usage));

                // Create table with indexes and actual contents
                if (ImGui::TreeNode("Buffer Contents")) {
                    ImGui::BeginChild(ImGui::GetID("vbo contents"), ImVec2(-1.0f, get_scrollable_height()), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                    ImGui::Columns(dimensions + 1);
                    const char *descriptors[] = {"index", "x", "y", "z", "w"};
                    for (int j = 0; j < dimensions + 1; j++) {
                        ImGui::Text("%s", descriptors[j]);
                        ImGui::NextColumn();
                    }
                    ImGui::Separator();

                    auto ptr = (char *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY) + (intptr_t)offset;
                    for (int j = 0, c = 0; j < size; j += stride, c++) {
                        ImGui::Text("%d", c);
                        ImGui::NextColumn();
                        for (int k = 0; k < dimensions; k++) {
                            switch (type) {
                                case GL_BYTE:
                                    ImGui::Text("% d", *(GLbyte *)&ptr[j + k * sizeof(GLbyte)]);
                                    break;
                                case GL_UNSIGNED_BYTE:
                                    ImGui::Text("%u", *(GLubyte *)&ptr[j + k * sizeof(GLubyte)]);
                                    break;
                                case GL_SHORT:
                                    ImGui::Text("% d", *(GLshort *)&ptr[j + k * sizeof(GLshort)]);
                                    break;
                                case GL_UNSIGNED_SHORT:
                                    ImGui::Text("%u", *(GLushort *)&ptr[j + k * sizeof(GLushort)]);
                                    break;
                                case GL_INT:
                                    ImGui::Text("% d", *(GLint *)&ptr[j + k * sizeof(GLint)]);
                                    break;
                                case GL_UNSIGNED_INT:
                                    ImGui::Text("%u", *(GLuint *)&ptr[j + k * sizeof(GLuint)]);
                                    break;
                                case GL_FLOAT:
                                    ImGui::Text("% f", *(GLfloat *)&ptr[j + k * sizeof(GLfloat)]);
                                    break;
                                case GL_DOUBLE:
                                    ImGui::Text("% f", *(GLdouble *)&ptr[j + k * sizeof(GLdouble)]);
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

auto neko_editor_create(neko_engine_cvar_t &cvar) -> dbgui & {
    return the<dbgui>()
            .create(
                    "utils",
                    [&](neko_dbgui_result) {
                        if (cvar.ui_imgui_debug) {
                            ImGui::ShowDemoWindow();
                        }
                        return neko_dbgui_result_in_progress;
                    },
                    neko_dbgui_flags::no_visible)
            .create("cvar",
                    [&](neko_dbgui_result) {
                        try {

                            ObjectView cvar_view{cvar};

                            auto f = [&]<typename T>(auto &name, auto &var, T &t) {
                                if (var.GetType() == Type_of<T>) neko::imgui::Auto(var.As<T>(), std::string(name.get_view()).c_str());
                            };

                            auto v = cvar_view.GetVars();
                            for (auto &iter = v.begin(); iter != v.end(); ++iter) {
                                const auto &[name, var] = *iter;
                                neko::invoke::apply([&](auto &&...args) { (f(name, var, args), ...); }, std::tuple<float, bool>());
                                for (const auto &attr : iter.GetFieldInfo().attrs)
                                    for (const auto &[name, var] : attr.GetVars()) {
                                        if (name == "info") ImGui::Text("    [%s]", var.As<const_str>());
                                    }
                            }

                        } catch (const std::exception ex) {
                            neko_error(std::format("[Exception] {0}", ex.what()).c_str());
                        }

                        return neko_dbgui_result_in_progress;
                    })
            .create("pack editor", [&](neko_dbgui_result) {
                // pack editor
                neko_private(neko_packreader_t *) pack_reader;
                neko_private(neko_pack_result) result;
                neko_private(bool) pack_reader_is_loaded = false;
                neko_private(u8) majorVersion;
                neko_private(u8) minorVersion;
                neko_private(u8) patchVersion;
                neko_private(bool) isLittleEndian;
                neko_private(u64) itemCount;

                neko_private(neko_string) file = neko_file_path("data");
                neko_private(bool) filebrowser = false;

                if (ImGui::Button("打开包") && !pack_reader_is_loaded) filebrowser = true;
                ImGui::SameLine();
                if (ImGui::Button("关闭包")) {
                    neko_pack_destroy(pack_reader);
                    pack_reader_is_loaded = false;
                }
                ImGui::SameLine();
                ImGui::Text("neko assets pack [%d.%d.%d]\n", PACK_VERSION_MAJOR, PACK_VERSION_MINOR, PACK_VERSION_PATCH);

                if (filebrowser) {
                    imgui::file_browser(file);

                    if (!file.empty() && !std::filesystem::is_directory(file)) {

                        if (pack_reader_is_loaded) {

                        } else {
                            result = neko_pack_info(file.c_str(), &majorVersion, &minorVersion, &patchVersion, &isLittleEndian, &itemCount);
                            if (result != SUCCESS_PACK_RESULT) return neko_dbgui_result_in_progress;

                            result = neko_pack_read(file.c_str(), 0, false, &pack_reader);
                            if (result != SUCCESS_PACK_RESULT) return neko_dbgui_result_in_progress;

                            if (result == SUCCESS_PACK_RESULT) itemCount = neko_pack_item_count(pack_reader);

                            pack_reader_is_loaded = true;
                        }

                        // 复位变量
                        filebrowser = false;
                        file = neko_file_path("data");
                    }
                }

                if (pack_reader_is_loaded && result == SUCCESS_PACK_RESULT) {

                    static int item_current_idx = -1;

                    ImGui::Text(
                            "包信息:\n"
                            " 打包版本: %d.%d.%d\n"
                            " 小端模式: %s\n"
                            " 文件数量: %llu\n\n",
                            majorVersion, minorVersion, patchVersion, isLittleEndian ? "true" : "false", (long long unsigned int)itemCount);

                    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetContentRegionMax().y - 250.0f);
                    if (ImGui::BeginTable("ui_pack_file_table", 4,
                                          ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                                                  ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti,
                                          outer_size)) {
                        ImGui::TableSetupScrollFreeze(0, 1);  // Make top row always visible
                        ImGui::TableSetupColumn("索引", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableSetupColumn("文件名", ImGuiTableColumnFlags_WidthFixed, 550.0f);
                        ImGui::TableSetupColumn("大小", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("操作", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableHeadersRow();

                        for (u64 i = 0; i < itemCount; ++i) {
                            ImGui::PushID(i);
                            ImGui::TableNextRow(ImGuiTableRowFlags_None);
                            if (ImGui::TableSetColumnIndex(0)) {
                                // if (ImGui::Selectable(std::format("{0}", ).c_str(), item_current_idx, ImGuiSelectableFlags_SpanAllColumns)) {
                                //     item_current_idx = i;
                                // }
                                ImGui::Text("%llu", (long long unsigned int)i);
                            }
                            if (ImGui::TableSetColumnIndex(1)) {
                                ImGui::Text("%s", neko_pack_item_path(pack_reader, i));
                            }
                            if (ImGui::TableSetColumnIndex(2)) {
                                ImGui::Text("%u", neko_pack_item_size(pack_reader, i));
                            }
                            if (ImGui::TableSetColumnIndex(3)) {
                                if (ImGui::SmallButton("查看")) {
                                    item_current_idx = i;
                                }
                                ImGui::SameLine();
                                if (ImGui::SmallButton("替换")) {
                                }
                                ImGui::SameLine();
                                if (ImGui::SmallButton("删除")) {
                                }
                            }
                            ImGui::PopID();
                        }

                        ImGui::EndTable();
                    }

                    if (item_current_idx >= 0) {
                        ImGui::Text("文件索引: %u\n包内路径: %s\n文件大小: %u\n", item_current_idx, neko_pack_item_path(pack_reader, item_current_idx),
                                    neko_pack_item_size(pack_reader, item_current_idx));
                    }

                } else if (result != SUCCESS_PACK_RESULT) {
                    ImGui::Text("错误: %s.\n", __neko_pack_result(result));
                }

                return neko_dbgui_result_in_progress;
            });
}

}  // namespace neko