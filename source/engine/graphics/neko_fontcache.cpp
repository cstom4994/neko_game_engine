
#include "neko_fontcache.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "libs/stb/stb_truetype.h"

void __neko_fontcache_init(neko_fontcache* cache) {
    neko_assert(cache);

    // Reserve global context data.
    cache->entry.reserve(8);
    cache->temp_path.reserve(256);
    cache->temp_codepoint_seen.reserve(512);
    cache->drawlist.vertices.reserve(4096);
    cache->drawlist.indices.reserve(8192);
    cache->drawlist.dcalls.reserve(512);

    // Reserve data atlas LRU regions.
    cache->atlas.next_atlas_idx_A = 0;
    cache->atlas.next_atlas_idx_B = 0;
    cache->atlas.next_atlas_idx_C = 0;
    cache->atlas.next_atlas_idx_D = 0;
    __neko_fontcache_LRU_init(cache->atlas.stateA, NEKO_FONTCACHE_ATLAS_REGION_A_CAPACITY);
    __neko_fontcache_LRU_init(cache->atlas.stateB, NEKO_FONTCACHE_ATLAS_REGION_B_CAPACITY);
    __neko_fontcache_LRU_init(cache->atlas.stateC, NEKO_FONTCACHE_ATLAS_REGION_C_CAPACITY);
    __neko_fontcache_LRU_init(cache->atlas.stateD, NEKO_FONTCACHE_ATLAS_REGION_D_CAPACITY);

    // Reserve data for shape cache. This is pretty big!
    __neko_fontcache_LRU_init(cache->shape_cache.state, NEKO_FONTCACHE_SHAPECACHE_SIZE);
    cache->shape_cache.storage.resize(NEKO_FONTCACHE_SHAPECACHE_SIZE);
    for (int i = 0; i < NEKO_FONTCACHE_SHAPECACHE_SIZE; i++) {
        cache->shape_cache.storage[i].glyphs.reserve(NEKO_FONTCACHE_SHAPECACHE_RESERNEKO_LENGTH);
        cache->shape_cache.storage[i].pos.reserve(NEKO_FONTCACHE_SHAPECACHE_RESERNEKO_LENGTH);
    }

    // We can actually go over NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH batches due to smart packing!
    cache->atlas.glyph_update_batch_drawlist.dcalls.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2);
    cache->atlas.glyph_update_batch_drawlist.vertices.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2 * 4);
    cache->atlas.glyph_update_batch_drawlist.indices.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2 * 6);
    cache->atlas.glyph_update_batch_clear_drawlist.dcalls.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2);
    cache->atlas.glyph_update_batch_clear_drawlist.vertices.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2 * 4);
    cache->atlas.glyph_update_batch_clear_drawlist.indices.reserve(NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH * 2 * 6);
}

void __neko_fontcache_shutdown(neko_fontcache* cache) {
    neko_assert(cache);
    for (neko_fontcache_entry& et : cache->entry) {
        __neko_fontcache_unload(cache, et.font_id);
    }
}

ve_font_id __neko_fontcache_load(neko_fontcache* cache, const void* data, size_t data_size, float size_px) {
    neko_assert(cache);
    if (!data) return -1;

    // Allocate cache entry.
    int id = -1;
    for (int i = 0; i < cache->entry.size(); i++) {
        if (!cache->entry[i].used) {
            id = i;
            break;
        }
    }
    if (id == -1) {
        cache->entry.push_back(neko_fontcache_entry());
        id = cache->entry.size() - 1;
    }
    neko_assert(id >= 0 && id < cache->entry.size());

    // Load hb_font from memory.
    auto& et = cache->entry[id];
    int success = stbtt_InitFont(&et.info, (const unsigned char*)data, 0);
    if (!success) {
        return -1;
    }
    et.font_id = id;
    et.size = size_px;
    et.size_scale = size_px < 0.0f ? stbtt_ScaleForPixelHeight(&et.info, -size_px) : stbtt_ScaleForMappingEmToPixels(&et.info, size_px);
    et.used = true;

    return id;
}

ve_font_id __neko_fontcache_loadfile(neko_fontcache* cache, const char* filename, std::vector<uint8_t>& buffer, float size_px) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer.resize(sz);

    if (fread(buffer.data(), 1, sz, fp) != sz) {
        buffer.clear();
        fclose(fp);
        return -1;
    }
    fclose(fp);

    ve_font_id ret = __neko_fontcache_load(cache, buffer.data(), buffer.size(), size_px);
    return ret;
}

void __neko_fontcache_unload(neko_fontcache* cache, ve_font_id font) {
    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());

    auto& et = cache->entry[font];
    et.used = false;
}

void __neko_fontcache_configure_snap(neko_fontcache* cache, uint32_t snap_width, uint32_t snap_height) {
    neko_assert(cache);
    cache->snap_width = snap_width;
    cache->snap_height = snap_height;
}

neko_fontcache_drawlist* __neko_fontcache_get_drawlist(neko_fontcache* cache) {
    neko_assert(cache);
    return &cache->drawlist;
}

void neko_fontcache_clear_drawlist(neko_fontcache_drawlist& drawlist) {
    drawlist.dcalls.clear();
    drawlist.indices.clear();
    drawlist.vertices.clear();
}

void neko_fontcache_merge_drawlist(neko_fontcache_drawlist& dest, const neko_fontcache_drawlist& src) {
    int voffset = dest.vertices.size();
    for (int i = 0; i < src.vertices.size(); i++) {
        dest.vertices.push_back(src.vertices[i]);
    }
    int ioffset = dest.indices.size();
    for (int i = 0; i < src.indices.size(); i++) {
        dest.indices.push_back(src.indices[i] + voffset);
    }
    for (int i = 0; i < src.dcalls.size(); i++) {
        neko_fontcache_draw dcall = src.dcalls[i];
        dcall.start_index += ioffset;
        dcall.end_index += ioffset;
        dest.dcalls.push_back(dcall);
    }
}

void __neko_fontcache_flush_drawlist(neko_fontcache* cache) {
    neko_assert(cache);
    neko_fontcache_clear_drawlist(cache->drawlist);
}

inline neko_vec2 neko_fontcache_eval_bezier(neko_vec2 p0, neko_vec2 p1, neko_vec2 p2, float t) {
    float t2 = t * t, c0 = (1.0f - t) * (1.0f - t), c1 = 2.0f * (1.0f - t) * t, c2 = t2;
    return neko_fontcache_make_vec2(c0 * p0.x + c1 * p1.x + c2 * p2.x, c0 * p0.y + c1 * p1.y + c2 * p2.y);
}

inline neko_vec2 neko_fontcache_eval_bezier(neko_vec2 p0, neko_vec2 p1, neko_vec2 p2, neko_vec2 p3, float t) {
    float t2 = t * t, t3 = t2 * t;
    float c0 = (1.0f - t) * (1.0f - t) * (1.0f - t), c1 = 3.0f * (1.0f - t) * (1.0f - t) * t, c2 = 3.0f * (1.0f - t) * t2, c3 = t3;
    return neko_fontcache_make_vec2(c0 * p0.x + c1 * p1.x + c2 * p2.x + c3 * p3.x, c0 * p0.y + c1 * p1.y + c2 * p2.y + c3 * p3.y);
}

// WARNING: doesn't actually append drawcall; caller is responsible for actually appending the drawcall.
void neko_fontcache_draw_filled_path(neko_fontcache_drawlist& drawlist, neko_vec2 outside, std::vector<neko_vec2>& path, float scaleX = 1.0f, float scaleY = 1.0f, float translateX = 0.0f,
                                     float translateY = 0.0f) {
#ifdef NEKO_FONTCACHE_DEBUGPRINT_VERBOSE
    printf("outline_path: \n");
    for (int i = 0; i < path.size(); i++) {
        printf("    %.2f %.2f\n", path[i].x * scaleX + translateX, path[i].y * scaleY + +translateY);
    }
#endif  // NEKO_FONTCACHE_DEBUGPRINT_VERBOSE

    int voffset = drawlist.vertices.size();
    for (int i = 0; i < path.size(); i++) {
        neko_fontcache_vertex vertex;
        vertex.x = path[i].x * scaleX + translateX;
        vertex.y = path[i].y * scaleY + +translateY;
        vertex.u = 0.0f;
        vertex.v = 0.0f;
        drawlist.vertices.push_back(vertex);
    }
    int voutside = drawlist.vertices.size();
    {
        neko_fontcache_vertex vertex;
        vertex.x = outside.x * scaleX + translateX;
        vertex.y = outside.y * scaleY + +translateY;
        vertex.u = 0.0f;
        vertex.v = 0.0f;
        drawlist.vertices.push_back(vertex);
    }
    for (int i = 1; i < path.size(); i++) {
        drawlist.indices.push_back(voutside);
        drawlist.indices.push_back(voffset + i - 1);
        drawlist.indices.push_back(voffset + i);
    }
}

// WARNING: doesn't actually append drawcall; caller is responsible for actually appending the drawcall.
void neko_fontcache_blit_quad(neko_fontcache_drawlist& drawlist, float x0 = 0.0f, float y0 = 0.0f, float x1 = 1.0f, float y1 = 1.0f, float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f,
                              float v1 = 1.0f) {
    int voffset = drawlist.vertices.size();

    neko_fontcache_vertex vertex;
    vertex.x = x0;
    vertex.y = y0;
    vertex.u = u0;
    vertex.v = v0;
    drawlist.vertices.push_back(vertex);
    vertex.x = x0;
    vertex.y = y1;
    vertex.u = u0;
    vertex.v = v1;
    drawlist.vertices.push_back(vertex);
    vertex.x = x1;
    vertex.y = y0;
    vertex.u = u1;
    vertex.v = v0;
    drawlist.vertices.push_back(vertex);
    vertex.x = x1;
    vertex.y = y1;
    vertex.u = u1;
    vertex.v = v1;
    drawlist.vertices.push_back(vertex);

    static uint32_t quad_indices[] = {0, 1, 2, 2, 1, 3};
    for (int i = 0; i < 6; i++) {
        drawlist.indices.push_back(voffset + quad_indices[i]);
    }
}

bool __neko_fontcache_cache_glyph(neko_fontcache* cache, ve_font_id font, ve_glyph glyph_index, float scaleX, float scaleY, float translateX, float translateY) {
    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());
    neko_fontcache_entry& entry = cache->entry[font];

    if (!glyph_index) {
        // Glyph not in current hb_font.
        return false;
    }

    // Retrieve the shape definition from STB TrueType.
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return true;
    stbtt_vertex* shape = nullptr;
    int nverts = stbtt_GetGlyphShape(&entry.info, glyph_index, &shape);
    if (!nverts || !shape) {
        return false;
    }

#ifdef NEKO_FONTCACHE_DEBUGPRINT_VERBOSE
    printf("shape: \n");
    for (int i = 0; i < nverts; i++) {
        if (shape[i].type == STBTT_vmove) {
            printf("    move_to %d %d\n", shape[i].x, shape[i].y);
        } else if (shape[i].type == STBTT_vline) {
            printf("    line_to %d %d\n", shape[i].x, shape[i].y);
        } else if (shape[i].type == STBTT_vcurve) {
            printf("    curve_to %d %d through %d %d\n", shape[i].x, shape[i].y, shape[i].cx, shape[i].cy);
        } else if (shape[i].type == STBTT_vcubic) {
            printf("    cubic_to %d %d through %d %d and %d %d\n", shape[i].x, shape[i].y, shape[i].cx, shape[i].cy, shape[i].cx1, shape[i].cy1);
        }
    }
#endif  // NEKO_FONTCACHE_DEBUGPRINT_VERBOSE

    // We need a random point that is outside our shape. We simply pick something diagonally across from top-left bound corner.
    // Note that this outside point is scaled alongside the glyph in neko_fontcache_draw_filled_path, so we don't need to handle that here.
    int bounds_x0, bounds_x1, bounds_y0, bounds_y1;
    int success = stbtt_GetGlyphBox(&entry.info, glyph_index, &bounds_x0, &bounds_y0, &bounds_x1, &bounds_y1);
    neko_assert(success);
    neko_vec2 outside = neko_fontcache_make_vec2(bounds_x0 - 21, bounds_y0 - 33);

    // Figure out scaling so it fits within our box.
    neko_fontcache_draw draw;
    draw.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH;
    draw.start_index = cache->drawlist.indices.size();

    // Draw the path using simplified version of https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac.
    // Instead of involving fragment shader code we simply make use of modern GPU ability to crunch triangles and brute force curve definitions.
    //
    std::vector<neko_vec2>& path = cache->temp_path;
    path.clear();
    for (int i = 0; i < nverts; i++) {
        stbtt_vertex& edge = shape[i];
        switch (edge.type) {
            case STBTT_vmove:
                if (path.size() > 0) {
                    neko_fontcache_draw_filled_path(cache->drawlist, outside, path, scaleX, scaleY, translateX, translateY);
                }
                path.clear();
                // Fallthrough.
            case STBTT_vline:
                path.push_back(neko_fontcache_make_vec2(shape[i].x, shape[i].y));
                break;
            case STBTT_vcurve: {
                neko_assert(path.size() > 0);
                neko_vec2 p0 = path[path.size() - 1];
                neko_vec2 p1 = neko_fontcache_make_vec2(shape[i].cx, shape[i].cy);
                neko_vec2 p2 = neko_fontcache_make_vec2(shape[i].x, shape[i].y);

                float step = 1.0f / NEKO_FONTCACHE_CURNEKO_QUALITY, t = step;
                for (int i = 0; i < NEKO_FONTCACHE_CURNEKO_QUALITY; i++) {
                    path.push_back(neko_fontcache_eval_bezier(p0, p1, p2, t));
                    t += step;
                }
                break;
            }
            case STBTT_vcubic: {
                neko_assert(path.size() > 0);
                neko_vec2 p0 = path[path.size() - 1];
                neko_vec2 p1 = neko_fontcache_make_vec2(shape[i].cx, shape[i].cy);
                neko_vec2 p2 = neko_fontcache_make_vec2(shape[i].cx1, shape[i].cy1);
                neko_vec2 p3 = neko_fontcache_make_vec2(shape[i].x, shape[i].y);

                float step = 1.0f / NEKO_FONTCACHE_CURNEKO_QUALITY, t = step;
                for (int i = 0; i < NEKO_FONTCACHE_CURNEKO_QUALITY; i++) {
                    path.push_back(neko_fontcache_eval_bezier(p0, p1, p2, p3, t));
                    t += step;
                }
                break;
            }
            default:
                neko_assert(!"Unknown shape edge type.");
        }
    }
    if (path.size() > 0) {
        neko_fontcache_draw_filled_path(cache->drawlist, outside, path, scaleX, scaleY, translateX, translateY);
    }

    // Append the draw call.
    draw.end_index = cache->drawlist.indices.size();
    if (draw.end_index > draw.start_index) {
        cache->drawlist.dcalls.push_back(draw);
    }

    stbtt_FreeShape(&entry.info, shape);
    return true;
}

static ve_atlas_region neko_fontcache_decide_codepoint_region(neko_fontcache* cache, neko_fontcache_entry& entry, int glyph_index, neko_fontcache_LRU*& state, uint32_t*& next_idx, float& oversample_x,
                                                              float& oversample_y) {
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return '\0';

    // Get hb_font text metrics. These are unscaled!
    int bounds_x0, bounds_x1, bounds_y0, bounds_y1;
    int success = stbtt_GetGlyphBox(&entry.info, glyph_index, &bounds_x0, &bounds_y0, &bounds_x1, &bounds_y1);
    int bounds_width = bounds_x1 - bounds_x0, bounds_height = bounds_y1 - bounds_y0;
    neko_assert(success);

    // Decide which atlas to target. This logic should work well for reasonable on-screen text sizes of around 24px.
    // For 4k+ displays, caching hb_font at a lower pt and drawing it upscaled at a higher pt is recommended.
    //
    ve_atlas_region region;
    float bwidth_scaled = bounds_width * entry.size_scale + 2.0f * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING, bheight_scaled = bounds_height * entry.size_scale + 2.0f * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    if (bwidth_scaled <= NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH && bheight_scaled <= NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT) {
        // Region A for small glyphs. These are good for things such as punctuation.
        region = 'A';
        state = &cache->atlas.stateA;
        next_idx = &cache->atlas.next_atlas_idx_A;
    } else if (bwidth_scaled <= NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH && bheight_scaled > NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT) {
        // Region B for tall glyphs. These are good for things such as european alphabets.
        region = 'B';
        state = &cache->atlas.stateB;
        next_idx = &cache->atlas.next_atlas_idx_B;
    } else if (bwidth_scaled <= NEKO_FONTCACHE_ATLAS_REGION_C_WIDTH && bheight_scaled <= NEKO_FONTCACHE_ATLAS_REGION_C_HEIGHT) {
        // Region C for big glyphs. These are good for things such as asian typography.
        region = 'C';
        state = &cache->atlas.stateC;
        next_idx = &cache->atlas.next_atlas_idx_C;
    } else if (bwidth_scaled <= NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH && bheight_scaled <= NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT) {
        // Region D for huge glyphs. These are good for things such as titles and 4k.
        region = 'D';
        state = &cache->atlas.stateD;
        next_idx = &cache->atlas.next_atlas_idx_D;
    } else if (bwidth_scaled <= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH && bheight_scaled <= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT) {
        // Region 'E' for massive glyphs. These are rendered uncached and un-oversampled.
        region = 'E';
        state = nullptr;
        next_idx = nullptr;
        if (bwidth_scaled <= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH / 2 && bheight_scaled <= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT / 2) {
            oversample_x = 2.0f;
            oversample_y = 2.0f;
        } else {
            oversample_x = 1.0f;
            oversample_y = 1.0f;
        }
        return region;
    } else {
        return '\0';
    }

    neko_assert(state);
    neko_assert(next_idx);

    return region;
}

static void neko_fontcache_flush_glyph_buffer_to_atlas(neko_fontcache* cache) {
    // Flush drawcalls to draw list.
    neko_fontcache_merge_drawlist(cache->drawlist, cache->atlas.glyph_update_batch_clear_drawlist);
    neko_fontcache_merge_drawlist(cache->drawlist, cache->atlas.glyph_update_batch_drawlist);
    neko_fontcache_clear_drawlist(cache->atlas.glyph_update_batch_clear_drawlist);
    neko_fontcache_clear_drawlist(cache->atlas.glyph_update_batch_drawlist);

    // Clear glyph_update_FBO.
    if (cache->atlas.glyph_update_batch_x != 0) {
        neko_fontcache_draw dcall;
        dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH;
        dcall.start_index = 0;
        dcall.end_index = 0;
        dcall.clear_before_draw = true;
        cache->drawlist.dcalls.push_back(dcall);
        cache->atlas.glyph_update_batch_x = 0;
    }
}

static void neko_fontcache_screenspace_xform(float& x, float& y, float& scalex, float& scaley, float width, float height) {
    scalex /= width;
    scaley /= height;
    scalex *= 2.0f;
    scaley *= 2.0f;
    x *= (2.0f / width);
    y *= (2.0f / height);
    x -= 1.0f;
    y -= 1.0f;
}

static void neko_fontcache_texspace_xform(float& x, float& y, float& scalex, float& scaley, float width, float height) {
    x /= width;
    y /= height;
    scalex /= width;
    scaley /= height;
}

static void neko_fontcache_atlas_bbox(ve_atlas_region region, int local_idx, float& x, float& y, float& width, float& height) {
    if (region == 'A') {
        width = NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH;
        height = NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT;
        x = (local_idx % NEKO_FONTCACHE_ATLAS_REGION_A_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH;
        y = (local_idx / NEKO_FONTCACHE_ATLAS_REGION_A_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT;
        x += NEKO_FONTCACHE_ATLAS_REGION_A_XOFFSET;
        y += NEKO_FONTCACHE_ATLAS_REGION_A_YOFFSET;
    } else if (region == 'B') {
        width = NEKO_FONTCACHE_ATLAS_REGION_B_WIDTH;
        height = NEKO_FONTCACHE_ATLAS_REGION_B_HEIGHT;
        x = (local_idx % NEKO_FONTCACHE_ATLAS_REGION_B_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_B_WIDTH;
        y = (local_idx / NEKO_FONTCACHE_ATLAS_REGION_B_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_B_HEIGHT;
        x += NEKO_FONTCACHE_ATLAS_REGION_B_XOFFSET;
        y += NEKO_FONTCACHE_ATLAS_REGION_B_YOFFSET;
    } else if (region == 'C') {
        width = NEKO_FONTCACHE_ATLAS_REGION_C_WIDTH;
        height = NEKO_FONTCACHE_ATLAS_REGION_C_HEIGHT;
        x = (local_idx % NEKO_FONTCACHE_ATLAS_REGION_C_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_C_WIDTH;
        y = (local_idx / NEKO_FONTCACHE_ATLAS_REGION_C_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_C_HEIGHT;
        x += NEKO_FONTCACHE_ATLAS_REGION_C_XOFFSET;
        y += NEKO_FONTCACHE_ATLAS_REGION_C_YOFFSET;
    } else {
        neko_assert(region == 'D');
        width = NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH;
        height = NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT;
        x = (local_idx % NEKO_FONTCACHE_ATLAS_REGION_D_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH;
        y = (local_idx / NEKO_FONTCACHE_ATLAS_REGION_D_XCAPACITY) * NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT;
        x += NEKO_FONTCACHE_ATLAS_REGION_D_XOFFSET;
        y += NEKO_FONTCACHE_ATLAS_REGION_D_YOFFSET;
    }
}

void neko_fontcache_cache_glyph_to_atlas(neko_fontcache* cache, ve_font_id font, ve_glyph glyph_index) {
    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());
    neko_fontcache_entry& entry = cache->entry[font];

    if (!glyph_index) {
        // Glyph not in current hb_font.
        return;
    }
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return;

    // Get hb_font text metrics. These are unscaled!
    int bounds_x0, bounds_x1, bounds_y0, bounds_y1;
    int success = stbtt_GetGlyphBox(&entry.info, glyph_index, &bounds_x0, &bounds_y0, &bounds_x1, &bounds_y1);
    int bounds_width = bounds_x1 - bounds_x0, bounds_height = bounds_y1 - bounds_y0;
    neko_assert(success);

    // Decide which atlas to target.
    neko_fontcache_LRU* state = nullptr;
    uint32_t* next_idx = nullptr;
    float oversample_x = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_X, oversample_y = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_Y;
    ve_atlas_region region = neko_fontcache_decide_codepoint_region(cache, entry, glyph_index, state, next_idx, oversample_x, oversample_y);

    // E region is special case and not cached to atlas.
    if (region == '\0' || region == 'E') return;

    // Grab an atlas LRU cache slot.
    uint64_t lru_code = glyph_index + ((0x100000000ULL * font) & 0xFFFFFFFF00000000ULL);
    int atlas_index = __neko_fontcache_LRU_get(*state, lru_code);
    if (atlas_index == -1) {
        if (*next_idx < state->capacity) {
            uint64_t evicted = __neko_fontcache_LRU_put(*state, lru_code, *next_idx);
            atlas_index = *next_idx;
            (*next_idx)++;
            neko_assert(evicted == lru_code);
        } else {
            uint64_t next_evict_codepoint = __neko_fontcache_LRU_get_next_evicted(*state);
            neko_assert(next_evict_codepoint != 0xFFFFFFFFFFFFFFFFULL);
            atlas_index = __neko_fontcache_LRU_peek(*state, next_evict_codepoint);
            neko_assert(atlas_index != -1);
            uint64_t evicted = __neko_fontcache_LRU_put(*state, lru_code, atlas_index);
            neko_assert(evicted == next_evict_codepoint);
        }
        neko_assert(__neko_fontcache_LRU_get(*state, lru_code) != -1);
    }

#ifdef NEKO_FONTCACHE_DEBUGPRINT
    static int debug_total_cached = 0;
    printf("glyph 0x%x( %c ) caching to atlas region %c at idx %d. %d total glyphs cached.\n", unicode, (char)unicode, (char)region, atlas_index, debug_total_cached++);
#endif  // NEKO_FONTCACHE_DEBUGPRINT

    // Draw oversized glyph to update FBO.
    float glyph_draw_scale_x = entry.size_scale * oversample_x;
    float glyph_draw_scale_y = entry.size_scale * oversample_y;
    float glyph_draw_translate_x = -bounds_x0 * glyph_draw_scale_x + NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    float glyph_draw_translate_y = -bounds_y0 * glyph_draw_scale_y + NEKO_FONTCACHE_GLYPHDRAW_PADDING;

    glyph_draw_translate_x = (int)(glyph_draw_translate_x + 0.9999999f);
    glyph_draw_translate_y = (int)(glyph_draw_translate_y + 0.9999999f);

    // Allocate a glyph_update_FBO region.
    int gdwidth_scaled_px = (int)(bounds_width * glyph_draw_scale_x + 1.0f) + 2 * oversample_x * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    if (cache->atlas.glyph_update_batch_x + gdwidth_scaled_px >= NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH) {
        neko_fontcache_flush_glyph_buffer_to_atlas(cache);
    }

    // Calculate the src and destination regions.

    float destx, desty, destw, desth, srcx, srcy, srcw, srch;
    neko_fontcache_atlas_bbox(region, atlas_index, destx, desty, destw, desth);
    float dest_glyph_x = destx + NEKO_FONTCACHE_ATLAS_GLYPH_PADDING, dest_glyph_y = desty + NEKO_FONTCACHE_ATLAS_GLYPH_PADDING, dest_glyph_w = bounds_width * entry.size_scale,
          dest_glyph_h = bounds_height * entry.size_scale;
    dest_glyph_x -= NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    dest_glyph_y -= NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    dest_glyph_w += 2 * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    dest_glyph_h += 2 * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    neko_fontcache_screenspace_xform(dest_glyph_x, dest_glyph_y, dest_glyph_w, dest_glyph_h, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);
    neko_fontcache_screenspace_xform(destx, desty, destw, desth, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);

    srcx = cache->atlas.glyph_update_batch_x;
    srcy = 0.0;
    srcw = bounds_width * glyph_draw_scale_x;
    srch = bounds_height * glyph_draw_scale_y;
    srcw += 2 * oversample_x * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    srch += 2 * oversample_y * NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    neko_fontcache_texspace_xform(srcx, srcy, srcw, srch, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);

    // Advance glyph_update_batch_x and calcuate final glyph drawing transform.
    glyph_draw_translate_x += cache->atlas.glyph_update_batch_x;
    cache->atlas.glyph_update_batch_x += gdwidth_scaled_px;
    neko_fontcache_screenspace_xform(glyph_draw_translate_x, glyph_draw_translate_y, glyph_draw_scale_x, glyph_draw_scale_y, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH,
                                     NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);

    // Queue up clear on target region on atlas.
    neko_fontcache_draw dcall;
    dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_ATLAS;
    dcall.region = (uint32_t)(-1);
    dcall.start_index = cache->atlas.glyph_update_batch_clear_drawlist.indices.size();
    neko_fontcache_blit_quad(cache->atlas.glyph_update_batch_clear_drawlist, destx, desty, destx + destw, desty + desth, 1.0f, 1.0f, 1.0f, 1.0f);
    dcall.end_index = cache->atlas.glyph_update_batch_clear_drawlist.indices.size();
    cache->atlas.glyph_update_batch_clear_drawlist.dcalls.push_back(dcall);

    // Queue up a blit from glyph_update_FBO to the atlas.
    dcall.region = (uint32_t)(0);
    dcall.start_index = cache->atlas.glyph_update_batch_drawlist.indices.size();
    neko_fontcache_blit_quad(cache->atlas.glyph_update_batch_drawlist, dest_glyph_x, dest_glyph_y, destx + dest_glyph_w, desty + dest_glyph_h, srcx, srcy, srcx + srcw, srcy + srch);
    dcall.end_index = cache->atlas.glyph_update_batch_drawlist.indices.size();
    cache->atlas.glyph_update_batch_drawlist.dcalls.push_back(dcall);

    // the<engine>().eng()-> glyph to glyph_update_FBO.
    __neko_fontcache_cache_glyph(cache, font, glyph_index, glyph_draw_scale_x, glyph_draw_scale_y, glyph_draw_translate_x, glyph_draw_translate_y);
}

void neko_fontcache_shape_text_uncached(neko_fontcache* cache, ve_font_id font, neko_fontcache_shaped_text& output, const std::string& text_utf8) {
    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());

    bool use_full_text_shape = cache->text_shape_advanced;
    neko_fontcache_entry& entry = cache->entry[font];
    output.glyphs.clear();
    output.pos.clear();

    int ascent = 0, descent = 0, line_gap = 0;
    stbtt_GetFontVMetrics(&entry.info, &ascent, &descent, &line_gap);

    cache->text_shape_advanced = false;

    // We use our own fallback dumbass text shaping.
    // WARNING: PLEASE USE HARFBUZZ. GOOD TEXT SHAPING IS IMPORTANT FOR INTERNATIONALISATION.

    utf8_int32_t codepoint, prev_codepoint = 0;
    size_t u32_length = utf8len(text_utf8.data());
    output.glyphs.reserve(u32_length);
    output.pos.reserve(u32_length);

    float pos = 0.0f, vpos = 0.0f;
    int advance = 0, to_left_side_glyph = 0;

    // Loop through text and shape.
    for (void* v = utf8codepoint(text_utf8.data(), &codepoint); codepoint; v = utf8codepoint(v, &codepoint)) {
        if (prev_codepoint) {
            int kern = stbtt_GetCodepointKernAdvance(&entry.info, prev_codepoint, codepoint);
            pos += kern * entry.size_scale;
        }
        if (codepoint == '\n') {
            pos = 0.0f;
            vpos -= (ascent - descent + line_gap) * entry.size_scale;
            vpos = (int)(vpos + 0.5f);
            prev_codepoint = 0;
            continue;
        }
        if (std::abs(entry.size) <= NEKO_FONTCACHE_ADVANCE_SNAP_SMALLFONT_SIZE) {
            // Expand advance to closest pixel for hb_font small sizes.
            pos = std::ceilf(pos);
        }

        output.glyphs.push_back(stbtt_FindGlyphIndex(&entry.info, codepoint));
        stbtt_GetCodepointHMetrics(&entry.info, codepoint, &advance, &to_left_side_glyph);
        output.pos.push_back(neko_fontcache_make_vec2(int(pos + 0.5), vpos));

        float adv = advance * entry.size_scale;

        pos += adv;
        prev_codepoint = codepoint;
    }

    output.end_cursor_pos.x = pos;
    output.end_cursor_pos.y = vpos;
}

template <typename T>
void neko_fontcache_ELFhash64(uint64_t& hash, const T* ptr, size_t count = 1) {
    uint64_t x = 0;
    auto bytes = reinterpret_cast<const uint8_t*>(ptr);

    for (int i = 0; i < sizeof(T) * count; i++) {
        hash = (hash << 4) + bytes[i];
        if ((x = hash & 0xF000000000000000ULL) != 0) {
            hash ^= (x >> 24);
        }
        hash &= ~x;
    }
}

static neko_fontcache_shaped_text& neko_fontcache_shape_text_cached(neko_fontcache* cache, ve_font_id font, const std::string& text_utf8) {
    uint64_t hash = 0x9f8e00d51d263c24ULL;
    neko_fontcache_ELFhash64(hash, (const uint8_t*)text_utf8.data(), text_utf8.size());
    neko_fontcache_ELFhash64(hash, &font);

    neko_fontcache_LRU& state = cache->shape_cache.state;
    int shape_cache_idx = __neko_fontcache_LRU_get(state, hash);
    if (shape_cache_idx == -1) {
        if (cache->shape_cache.next_cache_idx < state.capacity) {
            shape_cache_idx = cache->shape_cache.next_cache_idx++;
            __neko_fontcache_LRU_put(state, hash, shape_cache_idx);
        } else {
            uint64_t next_evict_idx = __neko_fontcache_LRU_get_next_evicted(state);
            neko_assert(next_evict_idx != 0xFFFFFFFFFFFFFFFFULL);
            shape_cache_idx = __neko_fontcache_LRU_peek(state, next_evict_idx);
            neko_assert(shape_cache_idx != -1);
            __neko_fontcache_LRU_put(state, hash, shape_cache_idx);
        }
        neko_fontcache_shape_text_uncached(cache, font, cache->shape_cache.storage[shape_cache_idx], text_utf8);
    }

    return cache->shape_cache.storage[shape_cache_idx];
}

static void neko_fontcache_directly_draw_massive_glyph(neko_fontcache* cache, neko_fontcache_entry& entry, ve_glyph glyph, int bounds_x0, int bounds_y0, int bounds_width, int bounds_height,
                                                       float oversample_x = 1.0f, float oversample_y = 1.0f, float posx = 0.0f, float posy = 0.0f, float scalex = 1.0f, float scaley = 1.0f) {
    // Flush out whatever was in the glyph buffer beforehand to atlas.
    neko_fontcache_flush_glyph_buffer_to_atlas(cache);

    // Draw un-antialiased glyph to update FBO.
    float glyph_draw_scale_x = entry.size_scale * oversample_x;
    float glyph_draw_scale_y = entry.size_scale * oversample_y;
    float glyph_draw_translate_x = -bounds_x0 * glyph_draw_scale_x + NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    float glyph_draw_translate_y = -bounds_y0 * glyph_draw_scale_y + NEKO_FONTCACHE_GLYPHDRAW_PADDING;
    neko_fontcache_screenspace_xform(glyph_draw_translate_x, glyph_draw_translate_y, glyph_draw_scale_x, glyph_draw_scale_y, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH,
                                     NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);

    // the<engine>().eng()-> glyph to glyph_update_FBO.
    __neko_fontcache_cache_glyph(cache, entry.font_id, glyph, glyph_draw_scale_x, glyph_draw_scale_y, glyph_draw_translate_x, glyph_draw_translate_y);

    // Figure out the source rect.
    float glyph_x = 0.0f, glyph_y = 0.0f, glyph_w = bounds_width * entry.size_scale * oversample_x, glyph_h = bounds_height * entry.size_scale * oversample_y;
    float glyph_dest_w = bounds_width * entry.size_scale, glyph_dest_h = bounds_height * entry.size_scale;
    glyph_w += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    glyph_h += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    glyph_dest_w += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    glyph_dest_h += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;

    // Figure out the destination rect.
    float bounds_x0_scaled = int(bounds_x0 * entry.size_scale - 0.5f);
    float bounds_y0_scaled = int(bounds_y0 * entry.size_scale - 0.5f);
    float dest_x = posx + scalex * bounds_x0_scaled;
    float dest_y = posy + scaley * bounds_y0_scaled;
    float dest_w = scalex * glyph_dest_w, dest_h = scaley * glyph_dest_h;
    dest_x -= scalex * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    dest_y -= scaley * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    neko_fontcache_texspace_xform(glyph_x, glyph_y, glyph_w, glyph_h, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH, NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT);

    // Add the glyph drawcall.
    neko_fontcache_draw dcall;
    dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED;
    dcall.colour[0] = cache->colour[0];
    dcall.colour[1] = cache->colour[1];
    dcall.colour[2] = cache->colour[2];
    dcall.colour[3] = cache->colour[3];
    dcall.start_index = cache->drawlist.indices.size();
    neko_fontcache_blit_quad(cache->drawlist, dest_x, dest_y, dest_x + dest_w, dest_y + dest_h, glyph_x, glyph_y, glyph_x + glyph_w, glyph_y + glyph_h);
    dcall.end_index = cache->drawlist.indices.size();
    cache->drawlist.dcalls.push_back(dcall);

    // Clear glyph_update_FBO.
    dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH;
    dcall.start_index = 0;
    dcall.end_index = 0;
    dcall.clear_before_draw = true;
    cache->drawlist.dcalls.push_back(dcall);
}

static bool neko_fontcache_empty(neko_fontcache* cache, neko_fontcache_entry& entry, ve_glyph glyph_index) {
    if (!glyph_index) {
        // Glyph not in current hb_font.
        return true;
    }
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return true;
    return false;
}

// This function only draws codepoints that have been drawn. Returns false without drawing anything if uncached.
bool neko_fontcache_draw_cached_glyph(neko_fontcache* cache, neko_fontcache_entry& entry, ve_glyph glyph_index, float posx = 0.0f, float posy = 0.0f, float scalex = 1.0f, float scaley = 1.0f) {
    if (!glyph_index) {
        // Glyph not in current hb_font.
        return true;
    }
    if (stbtt_IsGlyphEmpty(&entry.info, glyph_index)) return true;

    // Get hb_font text metrics. These are unscaled!
    int bounds_x0, bounds_x1, bounds_y0, bounds_y1;
    int success = stbtt_GetGlyphBox(&entry.info, glyph_index, &bounds_x0, &bounds_y0, &bounds_x1, &bounds_y1);
    int bounds_width = bounds_x1 - bounds_x0, bounds_height = bounds_y1 - bounds_y0;
    neko_assert(success);

    // Decide which atlas to target.
    neko_fontcache_LRU* state = nullptr;
    uint32_t* next_idx = nullptr;
    float oversample_x = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_X, oversample_y = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_Y;
    ve_atlas_region region = neko_fontcache_decide_codepoint_region(cache, entry, glyph_index, state, next_idx, oversample_x, oversample_y);

    // E region is special case and not cached to atlas.
    if (region == 'E') {
        neko_fontcache_directly_draw_massive_glyph(cache, entry, glyph_index, bounds_x0, bounds_y0, bounds_width, bounds_height, oversample_x, oversample_y, posx, posy, scalex, scaley);
        return true;
    }

    // Is this codepoint cached?
    uint64_t lru_code = glyph_index + ((0x100000000ULL * entry.font_id) & 0xFFFFFFFF00000000ULL);
    int atlas_index = __neko_fontcache_LRU_get(*state, lru_code);
    if (atlas_index == -1) {
        return false;
    }

    // Figure out the source bounding box in atlas texture.
    float atlas_x, atlas_y, atlas_w, atlas_h;
    neko_fontcache_atlas_bbox(region, atlas_index, atlas_x, atlas_y, atlas_w, atlas_h);
    float glyph_x = atlas_x, glyph_y = atlas_y, glyph_w = bounds_width * entry.size_scale, glyph_h = bounds_height * entry.size_scale;
    glyph_w += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    glyph_h += 2 * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;

    // Figure out the destination rect.
    float bounds_x0_scaled = int(bounds_x0 * entry.size_scale - 0.5f);
    float bounds_y0_scaled = int(bounds_y0 * entry.size_scale - 0.5f);
    float dest_x = posx + scalex * bounds_x0_scaled;
    float dest_y = posy + scaley * bounds_y0_scaled;
    float dest_w = scalex * glyph_w, dest_h = scaley * glyph_h;
    dest_x -= scalex * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    dest_y -= scaley * NEKO_FONTCACHE_ATLAS_GLYPH_PADDING;
    neko_fontcache_texspace_xform(glyph_x, glyph_y, glyph_w, glyph_h, NEKO_FONTCACHE_ATLAS_WIDTH, NEKO_FONTCACHE_ATLAS_HEIGHT);

    // Add the glyph drawcall.
    neko_fontcache_draw dcall;
    dcall.pass = NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET;
    dcall.colour[0] = cache->colour[0];
    dcall.colour[1] = cache->colour[1];
    dcall.colour[2] = cache->colour[2];
    dcall.colour[3] = cache->colour[3];
    dcall.start_index = cache->drawlist.indices.size();
    neko_fontcache_blit_quad(cache->drawlist, dest_x, dest_y, dest_x + dest_w, dest_y + dest_h, glyph_x, glyph_y, glyph_x + glyph_w, glyph_y + glyph_h);
    dcall.end_index = cache->drawlist.indices.size();
    cache->drawlist.dcalls.push_back(dcall);

    return true;
}

static void neko_fontcache_reset_batch_codepoint_state(neko_fontcache* cache) {
    cache->temp_codepoint_seen.clear();
    cache->temp_codepoint_seen.reserve(256);
}

static bool neko_fontcache_can_batch_glyph(neko_fontcache* cache, ve_font_id font, neko_fontcache_entry& entry, ve_glyph glyph_index) {
    neko_assert(cache);
    neko_assert(entry.font_id == font);

    // Decide which atlas to target.
    neko_assert(glyph_index != -1);
    neko_fontcache_LRU* state = nullptr;
    uint32_t* next_idx = nullptr;
    float oversample_x = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_X, oversample_y = NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_Y;
    ve_atlas_region region = neko_fontcache_decide_codepoint_region(cache, entry, glyph_index, state, next_idx, oversample_x, oversample_y);

    // E region can't batch.
    if (region == 'E' || region == '\0') return false;
    if (cache->temp_codepoint_seen.size() > 1024) return false;

    // Is this glyph cached?
    uint64_t lru_code = glyph_index + ((0x100000000ULL * entry.font_id) & 0xFFFFFFFF00000000ULL);
    int atlas_index = __neko_fontcache_LRU_get(*state, lru_code);
    if (atlas_index == -1) {
        if (*next_idx >= state->capacity) {
            // We will evict LRU. We must predict which LRU will get evicted, and if it's something we've seen then we need to take slowpath and flush batch.
            uint64_t next_evict_codepoint = __neko_fontcache_LRU_get_next_evicted(*state);
            if (cache->temp_codepoint_seen[next_evict_codepoint]) {
                return false;
            }
        }
        neko_fontcache_cache_glyph_to_atlas(cache, font, glyph_index);
    }

    neko_assert(__neko_fontcache_LRU_get(*state, lru_code) != -1);
    cache->temp_codepoint_seen[lru_code] = true;

    return true;
}

static void neko_fontcache_draw_text_batch(neko_fontcache* cache, neko_fontcache_entry& entry, neko_fontcache_shaped_text& shaped, int batch_start_idx, int batch_end_idx, float posx, float posy,
                                           float scalex, float scaley) {
    neko_fontcache_flush_glyph_buffer_to_atlas(cache);
    for (int j = batch_start_idx; j < batch_end_idx; j++) {
        ve_glyph glyph_index = shaped.glyphs[j];
        float glyph_translate_x = posx + shaped.pos[j].x * scalex, glyph_translate_y = posy + shaped.pos[j].y * scaley;
        bool glyph_cached = neko_fontcache_draw_cached_glyph(cache, entry, glyph_index, glyph_translate_x, glyph_translate_y, scalex, scaley);
        neko_assert(glyph_cached);
    }
}

bool __neko_fontcache_draw_text(neko_fontcache* cache, ve_font_id font, const std::string& text_utf8, float posx, float posy, float scalex, float scaley) {
    neko_fontcache_shaped_text& shaped = neko_fontcache_shape_text_cached(cache, font, text_utf8);

    if (cache->snap_width) posx = ((int)(posx * cache->snap_width + 0.5f)) / (float)cache->snap_width;
    if (cache->snap_height) posy = ((int)(posy * cache->snap_height + 0.5f)) / (float)cache->snap_height;

    neko_assert(cache);
    neko_assert(font >= 0 && font < cache->entry.size());
    neko_fontcache_entry& entry = cache->entry[font];

    int batch_start_idx = 0;
    for (int i = 0; i < shaped.glyphs.size(); i++) {
        ve_glyph glyph_index = shaped.glyphs[i];
        if (neko_fontcache_empty(cache, entry, glyph_index)) continue;

        if (neko_fontcache_can_batch_glyph(cache, font, entry, glyph_index)) {
            continue;
        }

        neko_fontcache_draw_text_batch(cache, entry, shaped, batch_start_idx, i, posx, posy, scalex, scaley);
        neko_fontcache_reset_batch_codepoint_state(cache);

        neko_fontcache_cache_glyph_to_atlas(cache, font, glyph_index);
        uint64_t lru_code = glyph_index + ((0x100000000ULL * font) & 0xFFFFFFFF00000000ULL);
        cache->temp_codepoint_seen[lru_code] = true;

        batch_start_idx = i;
    }

    neko_fontcache_draw_text_batch(cache, entry, shaped, batch_start_idx, shaped.glyphs.size(), posx, posy, scalex, scaley);
    neko_fontcache_reset_batch_codepoint_state(cache);
    cache->cursor_pos.x = posx + shaped.end_cursor_pos.x * scalex;
    cache->cursor_pos.y = posy + shaped.end_cursor_pos.x * scaley;

    return true;
}

neko_vec2 __neko_fontcache_get_cursor_pos(neko_fontcache* cache) { return cache->cursor_pos; }

void __neko_fontcache_optimise_drawlist(neko_fontcache* cache) {
    neko_assert(cache);

    int write_idx = 0;
    for (int i = 1; i < cache->drawlist.dcalls.size(); i++) {

        neko_assert(write_idx <= i);
        neko_fontcache_draw& draw0 = cache->drawlist.dcalls[write_idx];
        neko_fontcache_draw& draw1 = cache->drawlist.dcalls[i];

        bool merge = true;
        if (draw0.pass != draw1.pass) merge = false;
        if (draw0.end_index != draw1.start_index) merge = false;
        if (draw0.region != draw1.region) merge = false;
        if (draw1.clear_before_draw) merge = false;
        if (draw0.colour[0] != draw1.colour[0] || draw0.colour[1] != draw1.colour[1] || draw0.colour[2] != draw1.colour[2] || draw0.colour[3] != draw1.colour[3]) merge = false;

        if (merge) {
            draw0.end_index = draw1.end_index;
            draw1.start_index = draw1.end_index = 0;
        } else {
            neko_fontcache_draw& draw2 = cache->drawlist.dcalls[++write_idx];
            if (write_idx != i) draw2 = draw1;
        }
    }
    cache->drawlist.dcalls.resize(write_idx + 1);
}

void __neko_fontcache_set_colour(neko_fontcache* cache, float c[4]) {
    cache->colour[0] = c[0];
    cache->colour[1] = c[1];
    cache->colour[2] = c[2];
    cache->colour[3] = c[3];
}

// ------------------------------------------------------ Generic Data Structure Implementations ------------------------------------------

void __neko_fontcache_poollist_init(neko_fontcache_poollist& plist, int capacity) {
    plist.pool.resize(capacity);
    plist.freelist.resize(capacity);
    plist.capacity = capacity;
    for (int i = 0; i < capacity; i++) plist.freelist[i] = i;
}

void __neko_fontcache_poollist_push_front(neko_fontcache_poollist& plist, neko_fontcache_poollist_value v) {
    if (plist.size >= plist.capacity) return;
    neko_assert(plist.freelist.size() > 0);
    neko_assert(plist.freelist.size() == plist.capacity - plist.size);

    neko_fontcache_poollist_itr idx = plist.freelist.back();
    plist.freelist.pop_back();
    plist.pool[idx].prev = -1;
    plist.pool[idx].next = plist.front;
    plist.pool[idx].value = v;

    if (plist.front != -1) plist.pool[plist.front].prev = idx;
    if (plist.back == -1) plist.back = idx;
    plist.front = idx;
    plist.size++;
}

void __neko_fontcache_poollist_erase(neko_fontcache_poollist& plist, neko_fontcache_poollist_itr it) {
    if (plist.size <= 0) return;
    neko_assert(it >= 0 && it < plist.capacity);
    neko_assert(plist.freelist.size() == plist.capacity - plist.size);

    if (plist.pool[it].prev != -1) plist.pool[plist.pool[it].prev].next = plist.pool[it].next;
    if (plist.pool[it].next != -1) plist.pool[plist.pool[it].next].prev = plist.pool[it].prev;

    if (plist.front == it) plist.front = plist.pool[it].next;
    if (plist.back == it) plist.back = plist.pool[it].prev;

    plist.pool[it].prev = -1;
    plist.pool[it].next = -1;
    plist.pool[it].value = 0;
    plist.freelist.push_back(it);

    if (--plist.size == 0) plist.back = plist.front = -1;
}

neko_fontcache_poollist_value __neko_fontcache_poollist_peek_back(neko_fontcache_poollist& plist) {
    neko_assert(plist.back != -1);
    return plist.pool[plist.back].value;
}

neko_fontcache_poollist_value __neko_fontcache_poollist_pop_back(neko_fontcache_poollist& plist) {
    if (plist.size <= 0) return 0;
    neko_assert(plist.back != -1);
    neko_fontcache_poollist_value v = plist.pool[plist.back].value;
    __neko_fontcache_poollist_erase(plist, plist.back);
    return v;
}

void __neko_fontcache_LRU_init(neko_fontcache_LRU& LRU, int capacity) {
    // Thanks to dfsbfs from leetcode! This code is eavily based on that with simplifications made on top.
    // ref: https://leetcode.com/problems/lru-cache/discuss/968703/c%2B%2B
    //      https://leetcode.com/submissions/detail/436667816/
    LRU.capacity = capacity;
    LRU.cache.reserve(capacity);
    __neko_fontcache_poollist_init(LRU.key_queue, capacity);
}

void __neko_fontcache_LRU_refresh(neko_fontcache_LRU& LRU, uint64_t key) {
    auto it = LRU.cache.find(key);
    __neko_fontcache_poollist_erase(LRU.key_queue, it->second.ptr);
    __neko_fontcache_poollist_push_front(LRU.key_queue, key);
    it->second.ptr = LRU.key_queue.front;
}

int __neko_fontcache_LRU_get(neko_fontcache_LRU& LRU, uint64_t key) {
    auto it = LRU.cache.find(key);
    if (it == LRU.cache.end()) {
        return -1;
    }
    __neko_fontcache_LRU_refresh(LRU, key);
    return it->second.value;
}

int __neko_fontcache_LRU_peek(neko_fontcache_LRU& LRU, uint64_t key) {
    auto it = LRU.cache.find(key);
    if (it == LRU.cache.end()) {
        return -1;
    }
    return it->second.value;
}

uint64_t __neko_fontcache_LRU_put(neko_fontcache_LRU& LRU, uint64_t key, int val) {
    auto it = LRU.cache.find(key);
    if (it != LRU.cache.end()) {
        __neko_fontcache_LRU_refresh(LRU, key);
        it->second.value = val;
        return key;
    }

    uint64_t evict = key;
    if (LRU.key_queue.size >= LRU.capacity) {
        evict = __neko_fontcache_poollist_pop_back(LRU.key_queue);
        LRU.cache.erase(evict);
    }

    __neko_fontcache_poollist_push_front(LRU.key_queue, key);
    LRU.cache[key].value = val;
    LRU.cache[key].ptr = LRU.key_queue.front;
    return evict;
}

uint64_t __neko_fontcache_LRU_get_next_evicted(neko_fontcache_LRU& LRU) {
    if (LRU.key_queue.size >= LRU.capacity) {
        uint64_t evict = __neko_fontcache_poollist_peek_back(LRU.key_queue);
        ;
        return evict;
    }
    return 0xFFFFFFFFFFFFFFFFULL;
}
