#include "engine/sprite.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>

#include "edit.h"
#include "engine/base.h"
#include "engine/camera.h"
#include "engine/ecs.h"
#include "engine/game.h"
#include "engine/prelude.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "gfx.h"

// deps
#include <cute_aseprite.h>

typedef struct Sprite Sprite;
struct Sprite {
    EntityPoolElem pool_elem;

    CMat3 wmat;  // 要发送到着色器的世界变换矩阵

    CVec2 size;
    CVec2 texcell;
    CVec2 texsize;

    int depth;
};

static EntityPool *pool;

static char *atlas = NULL;

static GLuint sprite_program;
static GLuint vao;
static GLuint vbo;

static void _set_atlas(const char *filename, bool err) {
    CVec2 atlas_size;

    bool ok = asset_load(AssetLoadData{AssetKind_Image, true}, filename, NULL);

    if (!ok) {
        if (err) error("couldn't load atlas from path '%s', check path and format", filename);
        return;
    }

    mem_free(atlas);
    atlas = (char *)mem_alloc(strlen(filename) + 1);
    strcpy(atlas, filename);

    atlas_size = texture_get_size(atlas);
    glUseProgram(sprite_program);
    glUniform2fv(glGetUniformLocation(sprite_program, "atlas_size"), 1, (const GLfloat *)&atlas_size);
}

void sprite_set_atlas(const char *filename) { _set_atlas(filename, true); }

const char *sprite_get_atlas() { return atlas; }

void sprite_add(Entity ent) {
    Sprite *sprite;

    if (entitypool_get(pool, ent)) return;

    transform_add(ent);

    sprite = (Sprite *)entitypool_add(pool, ent);
    sprite->size = vec2(1.0f, 1.0f);
    sprite->texcell = vec2(32.0f, 32.0f);
    sprite->texsize = vec2(32.0f, 32.0f);
    sprite->depth = 0;
}
void sprite_remove(Entity ent) { entitypool_remove(pool, ent); }
bool sprite_has(Entity ent) { return entitypool_get(pool, ent) != NULL; }

void sprite_set_size(Entity ent, CVec2 size) {
    Sprite *sprite = (Sprite *)entitypool_get(pool, ent);
    error_assert(sprite);
    sprite->size = size;
}
CVec2 sprite_get_size(Entity ent) {
    Sprite *sprite = (Sprite *)entitypool_get(pool, ent);
    error_assert(sprite);
    return sprite->size;
}

void sprite_set_texcell(Entity ent, CVec2 texcell) {
    Sprite *sprite = (Sprite *)entitypool_get(pool, ent);
    error_assert(sprite);
    sprite->texcell = texcell;
}
CVec2 sprite_get_texcell(Entity ent) {
    Sprite *sprite = (Sprite *)entitypool_get(pool, ent);
    error_assert(sprite);
    return sprite->texcell;
}
void sprite_set_texsize(Entity ent, CVec2 texsize) {
    Sprite *sprite = (Sprite *)entitypool_get(pool, ent);
    error_assert(sprite);
    sprite->texsize = texsize;
}
CVec2 sprite_get_texsize(Entity ent) {
    Sprite *sprite = (Sprite *)entitypool_get(pool, ent);
    error_assert(sprite);
    return sprite->texsize;
}

void sprite_set_depth(Entity ent, int depth) {
    Sprite *sprite = (Sprite *)entitypool_get(pool, ent);
    error_assert(sprite);
    sprite->depth = depth;
}
int sprite_get_depth(Entity ent) {
    Sprite *sprite = (Sprite *)entitypool_get(pool, ent);
    error_assert(sprite);
    return sprite->depth;
}

void sprite_init() {
    PROFILE_FUNC();

    pool = entitypool_new(Sprite);

    sprite_program = gfx_create_program("sprite_program", "shader/sprite.vert", "shader/sprite.geom", "shader/sprite.frag");
    glUseProgram(sprite_program);
    glUniform1i(glGetUniformLocation(sprite_program, "tex0"), 0);
    sprite_set_atlas("assets/data/default.png");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gfx_bind_vertex_attrib(sprite_program, GL_FLOAT, 3, "wmat1", Sprite, wmat.m[0]);
    gfx_bind_vertex_attrib(sprite_program, GL_FLOAT, 3, "wmat2", Sprite, wmat.m[1]);
    gfx_bind_vertex_attrib(sprite_program, GL_FLOAT, 3, "wmat3", Sprite, wmat.m[2]);
    gfx_bind_vertex_attrib(sprite_program, GL_FLOAT, 2, "size", Sprite, size);
    gfx_bind_vertex_attrib(sprite_program, GL_FLOAT, 2, "texcell", Sprite, texcell);
    gfx_bind_vertex_attrib(sprite_program, GL_FLOAT, 2, "texsize", Sprite, texsize);
}

void sprite_fini() {

    glDeleteProgram(sprite_program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    entitypool_free(pool);

    mem_free(atlas);
}

void sprite_update_all() {
    Sprite *sprite;
    static CVec2 min = {-0.5, -0.5}, max = {0.5, 0.5};

    entitypool_remove_destroyed(pool, sprite_remove);

    entitypool_foreach(sprite, pool) { sprite->wmat = transform_get_world_matrix(sprite->pool_elem.ent); }

    if (edit_get_enabled()) {
        entitypool_foreach(sprite, pool) { edit_bboxes_update(sprite->pool_elem.ent, bbox(vec2_mul(sprite->size, min), vec2_mul(sprite->size, max))); }
    }
}

static int _depth_compare(const void *a, const void *b) {
    const Sprite *sa = (Sprite *)a, *sb = (Sprite *)b;

    if (sb->depth == sa->depth) return ((int)sa->pool_elem.ent.id) - ((int)sb->pool_elem.ent.id);
    return sb->depth - sa->depth;
}

void sprite_draw_all() {
    unsigned int nsprites;

    entitypool_sort(pool, _depth_compare);

    glUseProgram(sprite_program);
    glUniformMatrix3fv(glGetUniformLocation(sprite_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    glActiveTexture(GL_TEXTURE0);
    texture_bind(atlas);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    nsprites = entitypool_size(pool);
    glBufferData(GL_ARRAY_BUFFER, nsprites * sizeof(Sprite), entitypool_begin(pool), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nsprites);
}

void sprite_save_all(Store *s) {
    Store *t, *sprite_s;
    Sprite *sprite;

    if (store_child_save(&t, "sprite", s)) {
        string_save((const char **)&atlas, "atlas", t);

        entitypool_save_foreach(sprite, sprite_s, pool, "pool", t) {
            vec2_save(&sprite->size, "size", sprite_s);
            vec2_save(&sprite->texcell, "texcell", sprite_s);
            vec2_save(&sprite->texsize, "texsize", sprite_s);
            int_save(&sprite->depth, "depth", sprite_s);
        }
    }
}
void sprite_load_all(Store *s) {
    Store *t, *sprite_s;
    Sprite *sprite;
    char *tatlas;

    if (store_child_load(&t, "sprite", s)) {

        if (string_load(&tatlas, "atlas", NULL, t)) {
            _set_atlas(tatlas, false);
            mem_free(tatlas);
        }

        entitypool_load_foreach(sprite, sprite_s, pool, "pool", t) {
            vec2_load(&sprite->size, "size", vec2(1, 1), sprite_s);
            vec2_load(&sprite->texcell, "texcell", vec2(32, 32), sprite_s);
            vec2_load(&sprite->texsize, "texsize", vec2(32, 32), sprite_s);
            int_load(&sprite->depth, "depth", 0, sprite_s);
        }
    }
}

bool AseSpriteData::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    ase_t *ase = nullptr;
    {
        PROFILE_BLOCK("aseprite load");
        ase = cute_aseprite_load_from_memory(contents.data, (i32)contents.len, nullptr);
    }
    neko_defer(cute_aseprite_free(ase));

    Arena arena = {};

    i32 rect = ase->w * ase->h * 4;

    Slice<AseSpriteFrame> frames = {};
    frames.resize(&arena, ase->frame_count);

    Array<char> pixels = {};
    pixels.reserve(ase->frame_count * rect);
    neko_defer(pixels.trash());

    for (i32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t &frame = ase->frames[i];

        AseSpriteFrame sf = {};
        sf.duration = frame.duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (float)i / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (float)(i + 1) / ase->frame_count;

        frames[i] = sf;
        memcpy(pixels.data + (i * rect), &frame.pixels[0].r, rect);
    }

    // sg_image_desc desc = {};
    int ase_width = ase->w;
    int ase_height = ase->h * ase->frame_count;
    // desc.data.subimage[0][0].ptr = pixels.data;
    // desc.data.subimage[0][0].size = ase->frame_count * rect;

    u32 id = 0;
    {
        PROFILE_BLOCK("make image");
        LockGuard lock{&g_app->gpu_mtx};

        // 如果存在 则释放旧的 GL 纹理
        // if (tex->id != 0) glDeleteTextures(1, &tex->id);

        // 生成 GL 纹理
        glGenTextures(1, &id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // if (flip_image_vertical) {
        //     _flip_image_vertical(pixels.data, ase_width, ase_height);
        // }

        // 将纹理数据复制到 GL

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ase_width, ase_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture img = {};
    img.id = id;
    img.width = ase_width;
    img.height = ase_height;

    HashMap<AseSpriteLoop> by_tag = {};
    by_tag.reserve(ase->tag_count);

    for (i32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t &tag = ase->tags[i];

        u64 len = (u64)((tag.to_frame + 1) - tag.from_frame);

        AseSpriteLoop loop = {};

        loop.indices.resize(&arena, len);
        for (i32 j = 0; j < len; j++) {
            loop.indices[j] = j + tag.from_frame;
        }

        by_tag[fnv1a(tag.name)] = loop;
    }

    console_log("created sprite with image id: %d and %llu frames", img.id, (unsigned long long)frames.len);

    AseSpriteData s = {};
    s.arena = arena;
    s.img = img;
    s.frames = frames;
    s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *this = s;
    return true;
}

void AseSpriteData::trash() {
    by_tag.trash();
    arena.trash();
}

bool AseSprite::play(String tag) {
    u64 key = fnv1a(tag);
    bool same = loop == key;
    loop = key;
    return same;
}

void AseSprite::update(float dt) {
    AseSpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    i32 index = view.frame();
    AseSpriteFrame frame = view.data.frames[index];

    elapsed += dt * 1000;
    if (elapsed > frame.duration) {
        if (current_frame == view.len() - 1) {
            current_frame = 0;
        } else {
            current_frame++;
        }

        elapsed -= frame.duration;
    }
}

void AseSprite::set_frame(i32 frame) {
    AseSpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    if (0 <= frame && frame < view.len()) {
        current_frame = frame;
        elapsed = 0;
    }
}

bool AseSpriteView::make(AseSprite *spr) {
    Asset a = {};
    bool ok = asset_read(spr->sprite, &a);
    if (!ok) {
        return false;
    }

    AseSpriteData data = a.sprite;
    const AseSpriteLoop *res = data.by_tag.get(spr->loop);

    AseSpriteView view = {};
    view.sprite = spr;
    view.data = data;

    if (res != nullptr) {
        view.loop = *res;
    }

    *this = view;
    return true;
}

i32 AseSpriteView::frame() {
    if (loop.indices.data != nullptr) {
        return loop.indices[sprite->current_frame];
    } else {
        return sprite->current_frame;
    }
}

u64 AseSpriteView::len() {
    if (loop.indices.data != nullptr) {
        return loop.indices.len;
    } else {
        return data.frames.len;
    }
}
