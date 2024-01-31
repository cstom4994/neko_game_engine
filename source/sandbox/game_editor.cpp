
#include "game_editor.h"

#include <inttypes.h>

// ImGui
#include "libs/imgui/imgui.h"

static const s32 __profiler_max_level_colors = 11;
static const ImU32 __profiler_level_colors[__profiler_max_level_colors] = {IM_COL32(90, 150, 110, 255), IM_COL32(80, 180, 115, 255),  IM_COL32(129, 195, 110, 255), IM_COL32(170, 190, 100, 255),
                                                                           IM_COL32(210, 200, 80, 255), IM_COL32(230, 210, 115, 255), IM_COL32(240, 180, 90, 255),  IM_COL32(240, 140, 65, 255),
                                                                           IM_COL32(250, 110, 40, 255), IM_COL32(250, 75, 25, 255),   IM_COL32(250, 50, 0, 255)};

static u64 time_since_stat_clicked = neko_profiler_get_clock();
static const char *stat_clicked_name = 0;
static u32 stat_clicked_level = 0;

neko_inline void flash_color(ImU32 &_drawColor, uint64_t _elapsedTime) {
    ImVec4 white4 = ImColor(IM_COL32_WHITE);

    f32 msSince = __neko_profiler_clock2ms(_elapsedTime, __neko_profiler_get_clock_frequency());
    msSince = neko_min(msSince, __neko_flash_time_in_ms);
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
    static s32 thresholdLevel = 0;

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

        if ((thresholdLevel == (s32)cs.level + 1) && (threshold <= __neko_profiler_clock2ms(cs.end - cs.start, _data->cpu_frequency))) flash_color(drawColor, currTime - _data->end_time);

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