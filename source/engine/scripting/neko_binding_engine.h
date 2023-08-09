
#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/base/neko_engine.h"
#include "engine/platform/neko_platform.h"
#include "engine/scripting/neko_lua_base.h"

namespace neko {

neko_global lua_State* g_lua_bind;

static bool __neke_bind_platform_key_pressed(const_str key) {

    neko_platform_keycode cval;

    lua_pushstring(g_lua_bind, key);
    neko_lua_auto_to(g_lua_bind, neko_platform_keycode, &cval, -1);
    lua_pop(g_lua_bind, 1);

    neko_engine* engine = neko_engine_instance();
    return engine->ctx.platform->key_pressed(cval);
}

neko_inline void neko_register_platform(lua_State* L) {

    neko_lua_auto_enum(L, neko_platform_keycode);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_a);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_b);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_c);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_d);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_e);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_g);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_h);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_i);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_j);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_k);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_l);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_m);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_n);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_o);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_p);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_q);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_r);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_s);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_t);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_u);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_v);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_w);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_x);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_y);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_z);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_lshift);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_rshift);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_lalt);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_ralt);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_lctrl);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_rctrl);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_bspace);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_bslash);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_qmark);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_tilde);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_comma);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_period);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_esc);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_space);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_left);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_up);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_right);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_down);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_zero);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_one);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_two);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_three);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_four);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_five);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_six);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_seven);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_eight);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_nine);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npzero);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npone);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_nptwo);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npthree);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npfour);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npfive);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npsix);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npseven);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npeight);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npnine);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_caps);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_delete);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_end);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f1);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f2);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f3);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f4);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f5);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f6);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f7);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f8);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f9);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f10);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f11);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_f12);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_home);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_plus);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_minus);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_lbracket);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_rbracket);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_semi_colon);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_enter);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_insert);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_pgup);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_pgdown);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_numlock);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_tab);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npmult);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npdiv);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npplus);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npminus);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npenter);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_npdel);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_mute);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_volup);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_voldown);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_pause);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_print);
    neko_lua_auto_enum_value(L, neko_platform_keycode, neko_keycode_count);

    neko_lua_register_t<>(L).def(&__neke_bind_platform_key_pressed, "neko_key_pressed");
}

neko_inline void neko_register(lua_State* L) {
    g_lua_bind = L;
    neko_register_platform(L);
}

}  // namespace neko

#endif  // !NEKO_BINDING_ENGINE_H
