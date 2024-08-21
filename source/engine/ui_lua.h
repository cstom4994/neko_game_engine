#pragma once

#include "engine/base.h"
#include "engine/draw.h"
#include "engine/entity.h"
#include "engine/input.h"
#include "engine/luax.h"
#include "engine/math.h"
#include "engine/neko.hpp"
#include "engine/prelude.h"

#ifndef NEKO_UI
#define NEKO_UI

// lua binding
int open_ui(lua_State* L);
int open_mt_ui_container(lua_State* L);
int open_mt_ui_ref(lua_State* L);
int open_mt_ui_style(lua_State* L);

enum MUIRefKind : i32 {
    MUIRefKind_Nil,
    MUIRefKind_Boolean,
    MUIRefKind_Real,
    MUIRefKind_String,
};

struct MUIRef {
    MUIRefKind kind;
    union {
        int boolean;
        f32 real;
        char string[512];
    };
};

void lua_ui_set_ref(lua_State* L, MUIRef* ref, i32 arg);
MUIRef* lua_ui_check_ref(lua_State* L, i32 arg, MUIRefKind kind);

#endif
