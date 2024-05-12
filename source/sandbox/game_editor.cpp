
#include "game_editor.h"

#include <inttypes.h>

// ImGui
#include "engine/neko.h"
#include "sandbox/game_imgui.h"

static const s32 __profiler_max_level_colors = 11;
static const ImU32 __profiler_level_colors[__profiler_max_level_colors] = {IM_COL32(90, 150, 110, 255), IM_COL32(80, 180, 115, 255),  IM_COL32(129, 195, 110, 255), IM_COL32(170, 190, 100, 255),
                                                                           IM_COL32(210, 200, 80, 255), IM_COL32(230, 210, 115, 255), IM_COL32(240, 180, 90, 255),  IM_COL32(240, 140, 65, 255),
                                                                           IM_COL32(250, 110, 40, 255), IM_COL32(250, 75, 25, 255),   IM_COL32(250, 50, 0, 255)};

static u64 time_since_stat_clicked = neko_profiler_get_clock();
static const char *stat_clicked_name = 0;
static u32 stat_clicked_level = 0;

neko_inline void flash_color(ImU32 &_drawColor, u64 _elapsedTime) {
    ImVec4 white4 = ImColor(IM_COL32_WHITE);

    f32 msSince = __neko_profiler_clock2ms(_elapsedTime, __neko_profiler_get_clock_frequency());
    msSince = neko_min(msSince, __neko_flash_time_in_ms);
    msSince = 1.0f - (msSince / __neko_flash_time_in_ms);

    ImVec4 col4 = ImColor(_drawColor);
    _drawColor = ImColor(col4.x + (white4.x - col4.x) * msSince, col4.y + (white4.y - col4.y) * msSince, col4.z + (white4.z - col4.z) * msSince, 255.0f);
}

neko_inline void flash_color_named(ImU32 &_drawColor, neko_profiler_scope_t &_cs, u64 _elapsedTime) {
    if (stat_clicked_name && (strcmp(_cs.name, stat_clicked_name) == 0) && (_cs.level == stat_clicked_level)) flash_color(_drawColor, _elapsedTime);
}

neko_static_inline ImVec4 tri_color(f32 _cmp, f32 _min1, f32 _min2) {
    ImVec4 col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    if (_cmp > _min1) col = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    if (_cmp > _min2) col = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    return col;
}

neko_inline struct sort_scopes {
    neko_inline bool operator()(const neko_profiler_scope_t &a, const neko_profiler_scope_t &b) const {
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

void profiler_draw_frame_bavigation(frame_info *_infos, u32 _numInfos) {
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(1510.0f, 140.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Frame navigator", 0, ImGuiWindowFlags_NoScrollbar);

    static s32 sortKind = 0;
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
    for (u32 i = 0; i < _numInfos; ++i) {
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

s32 neko_profiler_draw_frame(neko_profiler_frame_t *_data, void *_buffer, size_t _bufferSize, bool _inGame, bool _multi) {
    s32 ret = 0;

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
    static s32 threshold_level = 0;

    if (_inGame) {
        ImGui::PushItemWidth(210);
        ImGui::SliderFloat("阈值   ", &threshold, 0.0f, 15.0f);

        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::SliderInt("阈值级别", &threshold_level, 0, 23);

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
    neko_profiler_set_threshold(threshold, threshold_level);

    static const s32 ME_MAX_FRAME_TIMES = 128;
    static f32 s_frameTimes[ME_MAX_FRAME_TIMES];
    static s32 s_currentFrame = 0;

    f32 maxFrameTime = 0.0f;
    if (_inGame) {
        frameStartY += 62.0f;

        if (s_currentFrame == 0) memset(s_frameTimes, 0, sizeof(s_frameTimes));

        if (!neko_profiler_is_paused()) {
            s_frameTimes[s_currentFrame % ME_MAX_FRAME_TIMES] = deltaTime;
            ++s_currentFrame;
        }

        f32 frameTimes[ME_MAX_FRAME_TIMES];
        for (s32 i = 0; i < ME_MAX_FRAME_TIMES; ++i) {
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

    u64 threadID = _data->scopes[0].thread_id;
    bool writeThreadName = true;

    u64 totalTime = _data->end_time - _data->start_time;

    f32 barHeight = 21.0f;
    f32 bottom = 0.0f;

    u64 currTime = neko_profiler_get_clock();

    for (u32 i = 0; i < _data->num_scopes; ++i) {
        neko_profiler_scope_t &cs = _data->scopes[i];
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
            for (u32 j = 0; j < _data->num_threads; ++j)
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
        s64 sX = s64(cs.start - _data->start_time);
        if (sX < 0) sX = -sX;
        s64 eX = s64(cs.end - _data->start_time);
        if (eX < 0) eX = -eX;

        f32 startXpct = f32(sX) / f32(totalTime);
        f32 endXpct = f32(eX) / f32(totalTime);

        f32 startX = paz.w2s(startXpct, frameStartX, frameEndX);
        f32 endX = paz.w2s(endXpct, frameStartX, frameEndX);

        ImVec2 tl = ImVec2(startX, frameStartY + cs.level * (barHeight + 1.0f));
        ImVec2 br = ImVec2(endX, frameStartY + cs.level * (barHeight + 1.0f) + barHeight);

        bottom = std::max(bottom, br.y);

        s32 level = cs.level;
        if (cs.level >= __profiler_max_level_colors) level = __profiler_max_level_colors - 1;

        ImU32 drawColor = __profiler_level_colors[level];
        flash_color_named(drawColor, cs, currTime - time_since_stat_clicked);

        if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(tl, br) && ImGui::IsWindowHovered()) {
            time_since_stat_clicked = currTime;
            stat_clicked_name = cs.name;
            stat_clicked_level = cs.level;
        }

        if ((threshold_level == (s32)cs.level + 1) && (threshold <= __neko_profiler_clock2ms(cs.end - cs.start, _data->cpu_frequency))) flash_color(drawColor, currTime - _data->end_time);

        draw_list->PushClipRect(tl, br, true);
        draw_list->AddRectFilled(tl, br, drawColor);
        tl.x += 3;
        draw_list->AddText(tl, IM_COL32(0, 0, 0, 255), cs.name);
        draw_list->PopClipRect();

        if (ImGui::IsMouseHoveringRect(tl, br) && ImGui::IsWindowHovered()) {
            ImGui::BeginTooltip();

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

            ImGui::EndTooltip();
        }
    }

    ImGui::End();

    return ret;
}

void neko_profiler_draw_stats(neko_profiler_frame_t *_data, bool _multi) {
    ImGui::SetNextWindowPos(ImVec2(920.0f, _multi ? 160.0f : 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600.0f, 900.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Frame stats");

    if (_data->num_scopes == 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.23f, 0.23f, 1.0f), "No scope data!");
        ImGui::End();
        return;
    }

    f32 deltaTime = __neko_profiler_clock2ms(_data->end_time - _data->start_time, _data->cpu_frequency);

    static s32 exclusive = 0;
    ImGui::Text("Sort by:  ");
    ImGui::SameLine();
    ImGui::RadioButton("Exclusive time", &exclusive, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Inclusive time", &exclusive, 1);
    ImGui::Separator();

    struct sort_excusive {
        bool operator()(const neko_profiler_scope_t &a, const neko_profiler_scope_t &b) const { return (a.stats->exclusive_time_total > b.stats->exclusive_time_total); }
    } customLessExc;

    struct sort_inclusive {
        bool operator()(const neko_profiler_scope_t &a, const neko_profiler_scope_t &b) const { return (a.stats->inclusive_time_total > b.stats->inclusive_time_total); }
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

    u64 totalTime = 0;
    if (exclusive == 0)
        totalTime = _data->scopes_stats[0].stats->exclusive_time_total;
    else
        totalTime = _data->scopes_stats[0].stats->inclusive_time_total;

    f32 barHeight = 21.0f;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    for (u32 i = 0; i < _data->num_scopes_stats; i++) {
        neko_profiler_scope_t &cs = _data->scopes_stats[i];

        f32 endXpct = f32(cs.stats->exclusive_time_total) / f32(totalTime);
        if (exclusive == 1) endXpct = f32(cs.stats->inclusive_time_total) / f32(totalTime);

        f32 startX = frameStartX;
        f32 endX = frameStartX + endXpct * (frameEndX - frameStartX);

        ImVec2 tl = ImVec2(startX, frameStartY);
        ImVec2 br = ImVec2(endX, frameStartY + barHeight);

        s32 colIdx = __profiler_max_level_colors - 1 - i;
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

#define R_TO_STRING_GENERATOR(x) \
    case x:                      \
        return #x;               \
        break;

const char *__neko_glenum_string(GLenum e) {
    switch (e) {
        // shader:
        R_TO_STRING_GENERATOR(GL_VERTEX_SHADER);
        R_TO_STRING_GENERATOR(GL_GEOMETRY_SHADER);
        R_TO_STRING_GENERATOR(GL_FRAGMENT_SHADER);

        // buffer usage:
        R_TO_STRING_GENERATOR(GL_STREAM_DRAW);
        R_TO_STRING_GENERATOR(GL_STREAM_READ);
        R_TO_STRING_GENERATOR(GL_STREAM_COPY);
        R_TO_STRING_GENERATOR(GL_STATIC_DRAW);
        R_TO_STRING_GENERATOR(GL_STATIC_READ);
        R_TO_STRING_GENERATOR(GL_STATIC_COPY);
        R_TO_STRING_GENERATOR(GL_DYNAMIC_DRAW);
        R_TO_STRING_GENERATOR(GL_DYNAMIC_READ);
        R_TO_STRING_GENERATOR(GL_DYNAMIC_COPY);

        // errors:
        R_TO_STRING_GENERATOR(GL_NO_ERROR);
        R_TO_STRING_GENERATOR(GL_INVALID_ENUM);
        R_TO_STRING_GENERATOR(GL_INVALID_VALUE);
        R_TO_STRING_GENERATOR(GL_INVALID_OPERATION);
        R_TO_STRING_GENERATOR(GL_INVALID_FRAMEBUFFER_OPERATION);
        R_TO_STRING_GENERATOR(GL_OUT_OF_MEMORY);
        R_TO_STRING_GENERATOR(GL_STACK_UNDERFLOW);
        R_TO_STRING_GENERATOR(GL_STACK_OVERFLOW);

        // types:
        R_TO_STRING_GENERATOR(GL_BYTE);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_BYTE);
        R_TO_STRING_GENERATOR(GL_SHORT);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_SHORT);
        R_TO_STRING_GENERATOR(GL_FLOAT);
        R_TO_STRING_GENERATOR(GL_FLOAT_VEC2);
        R_TO_STRING_GENERATOR(GL_FLOAT_VEC3);
        R_TO_STRING_GENERATOR(GL_FLOAT_VEC4);
        R_TO_STRING_GENERATOR(GL_DOUBLE);
        R_TO_STRING_GENERATOR(GL_DOUBLE_VEC2);
        R_TO_STRING_GENERATOR(GL_DOUBLE_VEC3);
        R_TO_STRING_GENERATOR(GL_DOUBLE_VEC4);
        R_TO_STRING_GENERATOR(GL_INT);
        R_TO_STRING_GENERATOR(GL_INT_VEC2);
        R_TO_STRING_GENERATOR(GL_INT_VEC3);
        R_TO_STRING_GENERATOR(GL_INT_VEC4);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_VEC2);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_VEC3);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_VEC4);
        R_TO_STRING_GENERATOR(GL_BOOL);
        R_TO_STRING_GENERATOR(GL_BOOL_VEC2);
        R_TO_STRING_GENERATOR(GL_BOOL_VEC3);
        R_TO_STRING_GENERATOR(GL_BOOL_VEC4);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT2);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT3);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT4);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT2x3);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT2x4);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT3x2);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT3x4);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT4x2);
        R_TO_STRING_GENERATOR(GL_FLOAT_MAT4x3);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT2);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT3);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT4);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT2x3);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT2x4);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT3x2);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT3x4);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT4x2);
        R_TO_STRING_GENERATOR(GL_DOUBLE_MAT4x3);
        R_TO_STRING_GENERATOR(GL_SAMPLER_1D);
        R_TO_STRING_GENERATOR(GL_SAMPLER_2D);
        R_TO_STRING_GENERATOR(GL_SAMPLER_3D);
        R_TO_STRING_GENERATOR(GL_SAMPLER_CUBE);
        R_TO_STRING_GENERATOR(GL_SAMPLER_1D_SHADOW);
        R_TO_STRING_GENERATOR(GL_SAMPLER_2D_SHADOW);
        R_TO_STRING_GENERATOR(GL_SAMPLER_1D_ARRAY);
        R_TO_STRING_GENERATOR(GL_SAMPLER_2D_ARRAY);
        R_TO_STRING_GENERATOR(GL_SAMPLER_1D_ARRAY_SHADOW);
        R_TO_STRING_GENERATOR(GL_SAMPLER_2D_ARRAY_SHADOW);
        R_TO_STRING_GENERATOR(GL_SAMPLER_2D_MULTISAMPLE);
        R_TO_STRING_GENERATOR(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
        R_TO_STRING_GENERATOR(GL_SAMPLER_CUBE_SHADOW);
        R_TO_STRING_GENERATOR(GL_SAMPLER_BUFFER);
        R_TO_STRING_GENERATOR(GL_SAMPLER_2D_RECT);
        R_TO_STRING_GENERATOR(GL_SAMPLER_2D_RECT_SHADOW);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_1D);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_2D);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_3D);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_CUBE);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_1D_ARRAY);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_2D_ARRAY);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_2D_MULTISAMPLE);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_BUFFER);
        R_TO_STRING_GENERATOR(GL_INT_SAMPLER_2D_RECT);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_1D);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_2D);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_3D);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_CUBE);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_BUFFER);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_SAMPLER_2D_RECT);

        R_TO_STRING_GENERATOR(GL_IMAGE_1D);
        R_TO_STRING_GENERATOR(GL_IMAGE_2D);
        R_TO_STRING_GENERATOR(GL_IMAGE_3D);
        R_TO_STRING_GENERATOR(GL_IMAGE_2D_RECT);
        R_TO_STRING_GENERATOR(GL_IMAGE_CUBE);
        R_TO_STRING_GENERATOR(GL_IMAGE_BUFFER);
        R_TO_STRING_GENERATOR(GL_IMAGE_1D_ARRAY);
        R_TO_STRING_GENERATOR(GL_IMAGE_2D_ARRAY);
        R_TO_STRING_GENERATOR(GL_IMAGE_2D_MULTISAMPLE);
        R_TO_STRING_GENERATOR(GL_IMAGE_2D_MULTISAMPLE_ARRAY);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_1D);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_2D);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_3D);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_2D_RECT);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_CUBE);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_BUFFER);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_1D_ARRAY);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_2D_ARRAY);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_2D_MULTISAMPLE);
        R_TO_STRING_GENERATOR(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_1D);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_2D);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_3D);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_2D_RECT);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_CUBE);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_BUFFER);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        R_TO_STRING_GENERATOR(GL_UNSIGNED_INT_ATOMIC_COUNTER);
    }

    static char buffer[32];
    std::sprintf(buffer, "Unknown GLenum: (0x%04x)", e);
    return buffer;
}

void render_uniform_variable(GLuint program, GLenum type, const char *name, GLint location) {
    static bool is_color = false;
    switch (type) {
        case GL_FLOAT:
            R_INTROSPECTION_GENERATE_VARIABLE_RENDER(GLfloat, 1, GL_FLOAT, glGetUniformfv, glProgramUniform1fv, ImGui::DragFloat);
            break;

        case GL_FLOAT_VEC2:
            R_INTROSPECTION_GENERATE_VARIABLE_RENDER(GLfloat, 2, GL_FLOAT_VEC2, glGetUniformfv, glProgramUniform2fv, ImGui::DragFloat2);
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
            R_INTROSPECTION_GENERATE_VARIABLE_RENDER(GLint, 1, GL_INT, glGetUniformiv, glProgramUniform1iv, ImGui::DragInt);
            break;

        case GL_INT_VEC2:
            R_INTROSPECTION_GENERATE_VARIABLE_RENDER(GLint, 2, GL_INT, glGetUniformiv, glProgramUniform2iv, ImGui::DragInt2);
            break;

        case GL_INT_VEC3:
            R_INTROSPECTION_GENERATE_VARIABLE_RENDER(GLint, 3, GL_INT, glGetUniformiv, glProgramUniform3iv, ImGui::DragInt3);
            break;

        case GL_INT_VEC4:
            R_INTROSPECTION_GENERATE_VARIABLE_RENDER(GLint, 4, GL_INT, glGetUniformiv, glProgramUniform4iv, ImGui::DragInt4);
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
            R_INTROSPECTION_GENERATE_VARIABLE_RENDER(GLint, 1, GL_SAMPLER_2D, glGetUniformiv, glProgramUniform1iv, ImGui::DragInt);
            break;

        case GL_FLOAT_MAT2:
            R_INTROSPECTION_GENERATE_MATRIX_RENDER(GLfloat, 2, 2, GL_FLOAT_MAT2, glGetUniformfv, glProgramUniformMatrix2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_MAT3:
            R_INTROSPECTION_GENERATE_MATRIX_RENDER(GLfloat, 3, 3, GL_FLOAT_MAT3, glGetUniformfv, glProgramUniformMatrix3fv, ImGui::DragFloat3);
            break;

        case GL_FLOAT_MAT4:
            R_INTROSPECTION_GENERATE_MATRIX_RENDER(GLfloat, 4, 4, GL_FLOAT_MAT4, glGetUniformfv, glProgramUniformMatrix4fv, ImGui::DragFloat4);
            break;

        case GL_FLOAT_MAT2x3:
            R_INTROSPECTION_GENERATE_MATRIX_RENDER(GLfloat, 3, 2, GL_FLOAT_MAT2x3, glGetUniformfv, glProgramUniformMatrix2x3fv, ImGui::DragFloat3);
            break;

        case GL_FLOAT_MAT2x4:
            R_INTROSPECTION_GENERATE_MATRIX_RENDER(GLfloat, 4, 2, GL_FLOAT_MAT2x4, glGetUniformfv, glProgramUniformMatrix2x4fv, ImGui::DragFloat4);
            break;

        case GL_FLOAT_MAT3x2:
            R_INTROSPECTION_GENERATE_MATRIX_RENDER(GLfloat, 2, 3, GL_FLOAT_MAT3x2, glGetUniformfv, glProgramUniformMatrix3x2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_MAT3x4:
            R_INTROSPECTION_GENERATE_MATRIX_RENDER(GLfloat, 4, 3, GL_FLOAT_MAT3x4, glGetUniformfv, glProgramUniformMatrix3x2fv, ImGui::DragFloat4);
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
            ImGui::TextColored(rgba_to_imvec(255, 64, 64), "%s has type %s, which isn't supported yet!", name, __neko_glenum_string(type));
            break;
    }
}

float get_scrollable_height() { return ImGui::GetTextLineHeight() * 16; }

void inspect_shader(const char *label, GLuint program) {
    neko_assert(label != nullptr);
    neko_assert(glIsProgram(program));

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

            for (const auto &shader : attached_shaders) {
                GLint source_length = 0;
                glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &source_length);
                static std::vector<char> source;
                source.resize(source_length);
                glGetShaderSource(shader, source_length, nullptr, source.data());

                GLint type = 0;
                glGetShaderiv(shader, GL_SHADER_TYPE, &type);

                ImGui::Indent();
                auto string_type = __neko_glenum_string(type);
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

void inspect_vertex_array(const char *label, GLuint vao) {
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
        std::sprintf(buffer, "Element Array Buffer: %d", ebo);
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
                // 元数据显示
                GLint buffer = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
                ImGui::Text("Buffer: %d", buffer);

                GLint type = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
                ImGui::Text("Type: %s", __neko_glenum_string(type));

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

                GLvoid *offset = nullptr;
                glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &offset);
                ImGui::Text("Offset in bytes: %" PRIdPTR "", (intptr_t)offset);

                GLint usage = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_USAGE, &usage);
                ImGui::Text("Usage: %s", __neko_glenum_string(usage));

                // 创建包含索引和实际内容的表
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