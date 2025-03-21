

#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/ecs/entitybase.hpp"
#include "engine/edit.h"

static char *atlas = NULL;

CEntityPool<CSprite> *Sprite__pool;

int type_sprite;

static Asset sprite_shader = {};
static GLuint sprite_vao;
static GLuint sprite_vbo;

static void _set_atlas(const char *filename, bool err) {
    vec2 atlas_size;

    GLuint sid = sprite_shader.shader.id;

    bool ok = asset_load(AssetLoadData{AssetKind_Image, true}, filename, NULL);

    if (!ok) {
        if (err) error("couldn't load atlas from path '%s', check path and format", filename);
        return;
    }

    mem_free(atlas);
    atlas = (char *)mem_alloc(strlen(filename) + 1);
    strcpy(atlas, filename);

    atlas_size = texture_get_size(atlas);
    glUseProgram(sid);
    glUniform2fv(glGetUniformLocation(sid, "atlas_size"), 1, (const GLfloat *)&atlas_size);
}

void sprite_set_atlas(const char *filename) { _set_atlas(filename, true); }

const char *sprite_get_atlas() { return atlas; }

void sprite_add(CEntity ent) {
    CSprite *sprite;

    if (Sprite__pool->Get(ent)) return;

    transform_add(ent);

    sprite = Sprite__pool->Add(ent);
    sprite->size = luavec2(1.0f, 1.0f);
    sprite->texcell = luavec2(32.0f, 32.0f);
    sprite->texsize = luavec2(32.0f, 32.0f);
    sprite->depth = 0;
}
void sprite_remove(CEntity ent) { Sprite__pool->Remove(ent); }
bool sprite_has(CEntity ent) { return Sprite__pool->Get(ent) != NULL; }

void sprite_set_size(CEntity ent, vec2 size) {
    CSprite *sprite = Sprite__pool->Get(ent);
    error_assert(sprite);
    sprite->size = size;
}
vec2 sprite_get_size(CEntity ent) {
    CSprite *sprite = Sprite__pool->Get(ent);
    error_assert(sprite);
    return sprite->size;
}

void sprite_set_texcell(CEntity ent, vec2 texcell) {
    CSprite *sprite = Sprite__pool->Get(ent);
    error_assert(sprite);
    sprite->texcell = texcell;
}
vec2 sprite_get_texcell(CEntity ent) {
    CSprite *sprite = Sprite__pool->Get(ent);
    error_assert(sprite);
    return sprite->texcell;
}
void sprite_set_texsize(CEntity ent, vec2 texsize) {
    CSprite *sprite = Sprite__pool->Get(ent);
    error_assert(sprite);
    sprite->texsize = texsize;
}
vec2 sprite_get_texsize(CEntity ent) {
    CSprite *sprite = Sprite__pool->Get(ent);
    error_assert(sprite);
    return sprite->texsize;
}

void sprite_set_depth(CEntity ent, int depth) {
    CSprite *sprite = Sprite__pool->Get(ent);
    error_assert(sprite);
    sprite->depth = depth;
}
int sprite_get_depth(CEntity ent) {
    CSprite *sprite = Sprite__pool->Get(ent);
    error_assert(sprite);
    return sprite->depth;
}

void sprite_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    type_sprite = EcsRegisterCType<CSprite>(L);

    Sprite__pool = EcsProtoGetCType<CSprite>(L);

    bool ok = asset_load_kind(AssetKind_Shader, "shader/sprite.glsl", &sprite_shader);
    error_assert(ok);

    GLuint sid = sprite_shader.shader.id;

    glUseProgram(sid);
    glUniform1i(glGetUniformLocation(sid, "tex0"), 0);
    sprite_set_atlas("assets/data/default.png");

    glGenVertexArrays(1, &sprite_vao);
    glBindVertexArray(sprite_vao);
    glGenBuffers(1, &sprite_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, sprite_vbo);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat1", CSprite, wmat.v[0]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat2", CSprite, wmat.v[3]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat3", CSprite, wmat.v[6]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "size", CSprite, size);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "texcell", CSprite, texcell);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "texsize", CSprite, texsize);
}

void sprite_fini() {

    glDeleteBuffers(1, &sprite_vbo);
    glDeleteVertexArrays(1, &sprite_vao);

    entitypool_free(Sprite__pool);

    mem_free(atlas);
}

int sprite_update_all(App *app, event_t evt) {

    static vec2 min = {-0.5, -0.5}, max = {0.5, 0.5};

    entitypool_remove_destroyed(Sprite__pool, sprite_remove);

    Sprite__pool->ForEach([](CSprite *sprite) { sprite->wmat = transform_get_world_matrix(sprite->ent); });

    if (edit_get_enabled()) {
        Sprite__pool->ForEach([](CSprite *sprite) { edit_bboxes_update(sprite->ent, bbox(vec2_mul(sprite->size, min), vec2_mul(sprite->size, max))); });
    }

    return 0;
}

static int _depth_compare(const void *a, const void *b) {
    const CSprite *sa = (CSprite *)a, *sb = (CSprite *)b;

    if (sb->depth == sa->depth) return ((int)sa->ent.id) - ((int)sb->ent.id);
    return sb->depth - sa->depth;
}

void sprite_draw_all() {
    unsigned int nsprites;

    entitypool_sort(Sprite__pool, _depth_compare);

    GLuint sid = sprite_shader.shader.id;

    glUseProgram(sid);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    glActiveTexture(GL_TEXTURE0);
    texture_bind(atlas);

    glBindVertexArray(sprite_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sprite_vbo);
    nsprites = entitypool_size(Sprite__pool);
    glBufferData(GL_ARRAY_BUFFER, nsprites * sizeof(CSprite), entitypool_begin(Sprite__pool), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nsprites);
}

void sprite_save_all(App *app) {
    // Store *t, *sprite_s;
    // CSprite *sprite;

    // if (store_child_save(&t, "sprite", s)) {
    //     string_save((const char **)&atlas, "atlas", t);

    //     entitypool_save_foreach(sprite, sprite_s, Sprite__pool, "pool", t) {
    //         vec2_save(&sprite->size, "size", sprite_s);
    //         vec2_save(&sprite->texcell, "texcell", sprite_s);
    //         vec2_save(&sprite->texsize, "texsize", sprite_s);
    //         int_save(&sprite->depth, "depth", sprite_s);
    //     }
    // }
}
void sprite_load_all(App *app) {
    // Store *t, *sprite_s;
    // CSprite *sprite;
    // char *tatlas;

    // if (store_child_load(&t, "sprite", s)) {

    //     if (string_load(&tatlas, "atlas", NULL, t)) {
    //         _set_atlas(tatlas, false);
    //         mem_free(tatlas);
    //     }

    //     entitypool_load_foreach(sprite, sprite_s, Sprite__pool, "pool", t) {
    //         vec2_load(&sprite->size, "size", luavec2(1, 1), sprite_s);
    //         vec2_load(&sprite->texcell, "texcell", luavec2(32, 32), sprite_s);
    //         vec2_load(&sprite->texsize, "texsize", luavec2(32, 32), sprite_s);
    //         int_load(&sprite->depth, "depth", 0, sprite_s);
    //     }
    // }
}