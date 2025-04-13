
#include "sprite.h"

#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/editor.h"
#include "engine/scripting/lua_util.h"
#include "engine/components/transform.h"
#include "engine/components/camera.h"

static char *atlas = NULL;

static Asset sprite_shader = {};
static GLuint sprite_vao;
static GLuint sprite_vbo;

static void _set_atlas(const char *filename, bool err) {
    vec2 atlas_size;

    GLuint sid = assets_get<AssetShader>(sprite_shader).id;

    bool ok = asset_load(AssetLoadData{AssetKind_Image, true}, filename, NULL);

    if (!ok) {
        if (err) errorf_marco("couldn't load atlas from path '%s', check path and format", filename);
        return;
    }

    mem_free(atlas);
    atlas = (char *)mem_alloc(strlen(filename) + 1);
    strcpy(atlas, filename);

    atlas_size = texture_get_size(atlas);
    glUseProgram(sid);
    glUniform2fv(glGetUniformLocation(sid, "atlas_size"), 1, (const GLfloat *)&atlas_size);
}

void Sprite::sprite_set_atlas(const char *filename) { _set_atlas(filename, true); }

const char *Sprite::sprite_get_atlas() { return atlas; }

CSprite *Sprite::sprite_add(CEntity ent) {
    CSprite *sprite;

    if (ComponentTypeBase::EntityPool->GetPtr(ent)) return nullptr;

    the<Transform>().transform_add(ent);

    sprite = ComponentTypeBase::EntityPool->Add(ent);
    sprite->size = luavec2(1.0f, 1.0f);
    sprite->texcell = luavec2(32.0f, 32.0f);
    sprite->texsize = luavec2(32.0f, 32.0f);
    sprite->depth = 0;

    return sprite;
}

CSprite *Sprite::wrap_sprite_add(CEntity ent) {
    CSprite *ptr = sprite_add(ent);

    auto L = ENGINE_LUA();

    EcsWorld *world = ENGINE_ECS();
    EntityData *e = EcsGetEnt(L, world, ent.id);
    LuaRef tb = LuaRef::NewTable(L);
    tb["__ud"] = ptr;
    int cid1 = EcsComponentSet(L, e, ComponentTypeBase::Tid, tb);

    return ptr;
}

void Sprite::sprite_remove(CEntity ent) { ComponentTypeBase::EntityPool->Remove(ent); }

bool Sprite::sprite_has(CEntity ent) { return ComponentTypeBase::EntityPool->GetPtr(ent) != NULL; }

void Sprite::sprite_set_size(CEntity ent, vec2 size) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);
    sprite->size = size;
}
vec2 Sprite::sprite_get_size(CEntity ent) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);
    return sprite->size;
}

void Sprite::sprite_set_texcell(CEntity ent, vec2 texcell) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);
    sprite->texcell = texcell;
}
vec2 Sprite::sprite_get_texcell(CEntity ent) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);
    return sprite->texcell;
}
void Sprite::sprite_set_texsize(CEntity ent, vec2 texsize) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);
    sprite->texsize = texsize;
}
vec2 Sprite::sprite_get_texsize(CEntity ent) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);
    return sprite->texsize;
}

void Sprite::sprite_set_depth(CEntity ent, int depth) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);
    sprite->depth = depth;
}
int Sprite::sprite_get_depth(CEntity ent) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);
    return sprite->depth;
}

void Sprite::sprite_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    ComponentTypeBase::Tid = EcsRegisterCType<CSprite>(L);
    ComponentTypeBase::EntityPool = EcsProtoGetCType<CSprite>(L);

    bool ok = asset_load_kind(AssetKind_Shader, "@code/game/shader/sprite.glsl", &sprite_shader);
    error_assert(ok);

    GLuint sid = assets_get<AssetShader>(sprite_shader).id;

    glUseProgram(sid);
    glUniform1i(glGetUniformLocation(sid, "tex0"), 0);
    sprite_set_atlas("@gamedata/assets/data/default.png");

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

    // clang-format off

    auto type = BUILD_TYPE(Sprite)
        .MemberMethod("sprite_set_atlas", this, &Sprite::sprite_set_atlas)
        .MemberMethod("sprite_get_atlas", this, &Sprite::sprite_get_atlas)
        .MemberMethod("sprite_add", this, &Sprite::wrap_sprite_add)
        .MemberMethod("sprite_remove", this, &Sprite::sprite_remove)
        .MemberMethod("sprite_has", this, &Sprite::sprite_has)
        .MemberMethod("sprite_set_size", this, &Sprite::sprite_set_size)
        .MemberMethod("sprite_get_size", this, &Sprite::sprite_get_size)
        .MemberMethod("sprite_set_texcell", this, &Sprite::sprite_set_texcell)
        .MemberMethod("sprite_get_texcell", this, &Sprite::sprite_get_texcell)
        .MemberMethod("sprite_set_texsize", this, &Sprite::sprite_set_texsize)
        .MemberMethod("sprite_get_texsize", this, &Sprite::sprite_get_texsize)
        .MemberMethod("sprite_set_depth", this, &Sprite::sprite_set_depth)
        .MemberMethod("sprite_get_depth", this, &Sprite::sprite_get_depth)
        .Build();

    // clang-format on
}

void Sprite::sprite_fini() {

    glDeleteBuffers(1, &sprite_vbo);
    glDeleteVertexArrays(1, &sprite_vao);

    entitypool_free(ComponentTypeBase::EntityPool);

    mem_free(atlas);
}

int Sprite::sprite_update_all(Event evt) {

    static vec2 min = {-0.5, -0.5}, max = {0.5, 0.5};

    entitypool_remove_destroyed(ComponentTypeBase::EntityPool, [this](CEntity ent) { sprite_remove(ent); });

    ComponentTypeBase::EntityPool->ForEach([](CSprite *sprite) { sprite->wmat = the<Transform>().transform_get_world_matrix(sprite->ent); });

    if (edit_get_enabled()) {
        ComponentTypeBase::EntityPool->ForEach([](CSprite *sprite) { edit_bboxes_update(sprite->ent, bbox(vec2_mul(sprite->size, min), vec2_mul(sprite->size, max))); });
    }

    return 0;
}

static int _depth_compare(const void *a, const void *b) {
    const CSprite *sa = (CSprite *)a, *sb = (CSprite *)b;

    if (sb->depth == sa->depth) return ((int)sa->ent.id) - ((int)sb->ent.id);
    return sb->depth - sa->depth;
}

void Sprite::sprite_draw_all() {
    unsigned int nsprites;

    entitypool_sort(ComponentTypeBase::EntityPool, _depth_compare);

    GLuint sid = assets_get<AssetShader>(sprite_shader).id;

    glUseProgram(sid);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)the<Camera>().GetInverseViewMatrixPtr());

    texture_bind_byname(atlas, 0);

    glBindVertexArray(sprite_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sprite_vbo);
    nsprites = entitypool_size(ComponentTypeBase::EntityPool);
    glBufferData(GL_ARRAY_BUFFER, nsprites * sizeof(CSprite), entitypool_begin(ComponentTypeBase::EntityPool), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nsprites);
}

DEFINE_IMGUI_BEGIN(template <>, CSprite) {
    ImGuiWrap::Auto(var.wmat, "wmat");
    ImGuiWrap::Auto(var.size, "size");
    ImGuiWrap::Auto(var.texcell, "texcell");
    ImGuiWrap::Auto(var.texsize, "texsize");
    ImGuiWrap::Auto(var.depth, "depth");
}
DEFINE_IMGUI_END()

int Sprite::Inspect(CEntity ent) {
    CSprite *sprite = ComponentTypeBase::EntityPool->GetPtr(ent);
    error_assert(sprite);

    ImGuiWrap::Auto(sprite, "CSprite");

    return 0;
}

void sprite_save_all(CL *app) {
    // Store *t, *sprite_s;
    // CSprite *sprite;

    // if (store_child_save(&t, "sprite", s)) {
    //     string_save((const char **)&atlas, "atlas", t);

    //     entitypool_save_foreach(sprite, sprite_s, ComponentTypeBase::EntityPool, "pool", t) {
    //         vec2_save(&sprite->size, "size", sprite_s);
    //         vec2_save(&sprite->texcell, "texcell", sprite_s);
    //         vec2_save(&sprite->texsize, "texsize", sprite_s);
    //         int_save(&sprite->depth, "depth", sprite_s);
    //     }
    // }
}
void sprite_load_all(CL *app) {
    // Store *t, *sprite_s;
    // CSprite *sprite;
    // char *tatlas;

    // if (store_child_load(&t, "sprite", s)) {

    //     if (string_load(&tatlas, "atlas", NULL, t)) {
    //         _set_atlas(tatlas, false);
    //         mem_free(tatlas);
    //     }

    //     entitypool_load_foreach(sprite, sprite_s, ComponentTypeBase::EntityPool, "pool", t) {
    //         vec2_load(&sprite->size, "size", luavec2(1, 1), sprite_s);
    //         vec2_load(&sprite->texcell, "texcell", luavec2(32, 32), sprite_s);
    //         vec2_load(&sprite->texsize, "texsize", luavec2(32, 32), sprite_s);
    //         int_load(&sprite->depth, "depth", 0, sprite_s);
    //     }
    // }
}