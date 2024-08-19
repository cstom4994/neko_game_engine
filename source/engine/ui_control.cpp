#include "engine/ui.h"

i32 ui_text_ex(ui_context_t* ctx, const char* text, i32 wrap, Color256* col, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0, elementid = UI_ELEMENT_TEXT;
    ui_id id = ui_get_id(ctx, text, strlen(text));
    idraw_t* dl = &ctx->overlay_draw_list;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    const char *start, *end, *p = text;
    i32 width = -1;

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    ui_style_t* save = ui_push_style(ctx, &style);
    FontFamily* font = ctx->style->font;
    Color256* color = col ? col : &ctx->style->colors[UI_COLOR_CONTENT];
    i32 sx = ctx->style->shadow_x;
    i32 sy = ctx->style->shadow_y;
    if (opt & UI_OPT_NOSTYLESHADOW) {
        sx = 0;
        sy = 0;
    }
    Color256* sc = &ctx->style->colors[UI_COLOR_SHADOW];
    i32 th = ui_font_height(font);
    ui_layout_column_begin(ctx);
    ui_layout_row(ctx, 1, &width, th);

    ui_rect_t tr = ui_layout_next(ctx);
    ui_layout_set_next(ctx, tr, 0);
    ui_rect_t r = ui_layout_next(ctx);
    ui_rect_t bg = r;
    do {
        i32 w = 0;
        start = end = p;
        do {
            const char* word = p;
            while (*p && *p != ' ' && *p != '\n') {
                p++;
            }

            if (wrap) w += ui_text_width(font, word, p - word);
            if (w > r.w && end != start) {
                break;
            }

            if (wrap) w += ui_text_width(font, p, 1);
            end = p++;

        } while (*end && *end != '\n');

        if (r.w > tr.w) tr.w = r.w;
        tr.h = (r.y + r.h) - tr.y;

        ui_rect_t txtrct = r;
        bg = r;
        if (*end) {
            r = ui_layout_next(ctx);
            bg.h = r.y - bg.y;
        } else {
            i32 th = ui_text_height(font, start, end - start);
            bg.h = r.h + (float)th / 2.f;
        }

        // Draw frame here for background if applicable (need to do this to account for space between wrap)
        if (ctx->style->colors[UI_COLOR_BACKGROUND].a && ~opt & UI_OPT_NOSTYLEBACKGROUND) {
            ui_draw_rect(ctx, bg, style.colors[UI_COLOR_BACKGROUND]);
        }

        // Draw text
        ui_draw_text(ctx, font, start, end - start, neko_v2(txtrct.x, txtrct.y), *color, sx, sy, *sc);
        p = end + 1;

    } while (*end);

    // draw border
    if (style.colors[UI_COLOR_BORDER].a && ~opt & UI_OPT_NOSTYLEBORDER) {
        ui_draw_box(ctx, ui_expand_rect(tr, (i16*)style.border_width), (i16*)style.border_width, style.colors[UI_COLOR_BORDER]);
    }

    ui_update_control(ctx, id, tr, 0x00);

    // handle click
    if (ctx->mouse_down != UI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == UI_ELEMENT_STATE_OFF_FOCUS) {
        res |= UI_RES_SUBMIT;
    }

    ui_layout_column_end(ctx);
    ui_pop_style(ctx, save);

    return res;
}

//  i32 ui_text_fc_ex(ui_context_t* ctx, const char* text, neko_font_index fontindex) {
//     i32 width = -1;
//     i32 th = 20;
//     ui_layout_column_begin(ctx);
//     ui_layout_row(ctx, 1, &width, th);
//     ui_layout_t* layout = ui_get_layout(ctx);
//     if (fontindex == -1) fontindex = ctx->gui_idraw.data->font_fc_default;
//     gfx_fc_text(text, fontindex, layout->body.x, layout->body.y + layout->body.h / 2);
//     ui_layout_column_end(ctx);
//     return 0;
// }

i32 ui_label_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) {
    // Want to push animations here for styles
    i32 res = 0;
    i32 elementid = UI_ELEMENT_LABEL;
    ui_id id = ui_get_id(ctx, label, neko_strlen(label));

    char id_tag[256] = NEKO_DEFAULT_VAL();
    char label_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag), opt);
    ui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    if (id_tag) ui_push_id(ctx, id_tag, strlen(id_tag));

    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, 0x00);
    ui_draw_control_text(ctx, label_tag, r, &style, 0x00);
    ui_pop_style(ctx, save);
    if (id_tag) ui_pop_id(ctx);

    // handle click
    if (ctx->mouse_down != UI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == UI_ELEMENT_STATE_OFF_FOCUS) {
        res |= UI_RES_SUBMIT;
    }

    return res;
}

i32 ui_image_ex(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, vec2 uv0, vec2 uv1, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0;
    ui_id id = ui_get_id(ctx, &hndl, sizeof(hndl));
    const i32 elementid = UI_ELEMENT_IMAGE;

    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Temporary copy of style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, opt);

    // handle click
    if (ctx->mouse_down != UI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == UI_ELEMENT_STATE_OFF_FOCUS) {
        res |= UI_RES_SUBMIT;
    }

    // draw border
    if (style.colors[UI_COLOR_BORDER].a) {
        ui_draw_box(ctx, ui_expand_rect(r, (i16*)style.border_width), (i16*)style.border_width, style.colors[UI_COLOR_BORDER]);
    }

    ui_draw_image(ctx, hndl, r, uv0, uv1, style.colors[UI_COLOR_CONTENT]);

    ui_pop_style(ctx, save);

    return res;
}

i32 ui_combo_begin_ex(ui_context_t* ctx, const char* id, const char* current_item, i32 max_items, ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0;
    opt = UI_OPT_NOMOVE | UI_OPT_NORESIZE | UI_OPT_NOTITLE | UI_OPT_FORCESETRECT;

    if (ui_button(ctx, current_item)) {
        ui_popup_open(ctx, id);
    }

    i32 ct = max_items > 0 ? max_items : 0;
    ui_rect_t rect = ctx->last_rect;
    rect.y += rect.h;
    rect.h = ct ? (ct + 1) * ctx->style_sheet->styles[UI_ELEMENT_BUTTON][0x00].size[1] : rect.h;
    return ui_popup_begin_ex(ctx, id, rect, NULL, opt);
}

i32 ui_combo_item_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = ui_button_ex(ctx, label, desc, opt);
    if (res) {
        ui_current_container_close(ctx);
    }
    return res;
}

void ui_combo_end(ui_context_t* ctx) { ui_popup_end(ctx); }

void ui_parse_label_tag(ui_context_t* ctx, const char* str, char* buffer, size_t sz) {
    // neko_lexer_t lex = neko_lexer_c_ctor(str);
    // while (neko_lexer_can_lex(&lex)) {
    //     neko_token_t token = neko_lexer_next_token(&lex);
    //     switch (token.type) {
    //         case NEKO_TOKEN_HASH: {
    //             if (neko_lexer_peek(&lex).type == NEKO_TOKEN_HASH) {
    //                 neko_token_t end = neko_lexer_current_token(&lex);

    //                 // Determine len
    //                 size_t len = NEKO_MIN(end.text - str, sz);

    //                 memcpy(buffer, str, len);
    //                 return;
    //             }
    //         } break;
    //     }
    // }

    // Reached end, so just memcpy
    memcpy(buffer, str, NEKO_MIN(sz, strlen(str) + 1));
}

void ui_parse_id_tag(ui_context_t* ctx, const char* str, char* buffer, size_t sz, u64 opt) {
    if (opt & UI_OPT_PARSEIDTAGONLY) {
        // neko_lexer_t lex = neko_lexer_c_ctor(str);
        // while (neko_lexer_can_lex(&lex)) {
        //     neko_token_t token = neko_lexer_next_token(&lex);
        //     switch (token.type) {
        //         case NEKO_TOKEN_HASH: {
        //             if (neko_lexer_peek(&lex).type == NEKO_TOKEN_HASH) {
        //                 neko_token_t end = neko_lexer_next_token(&lex);
        //                 end = neko_lexer_next_token(&lex);
        //                 // 确定长度
        //                 size_t len = NEKO_MIN((str + strlen(str)) - end.text, sz);
        //                 memcpy(buffer, end.text, len);
        //                 return;
        //             }
        //         } break;
        //     }
        // }
    } else {
        size_t str_sz = strlen(str);
        size_t actual_sz = NEKO_MIN(str_sz, sz - 1);
        memcpy(buffer, str, actual_sz);
        buffer[actual_sz] = 0;
    }
}

i32 ui_button_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) {
    // Note: clip out early here for performance

    i32 res = 0;
    ui_id id = ui_get_id(ctx, label, strlen(label));
    idraw_t* dl = &ctx->overlay_draw_list;

    char id_tag[256] = NEKO_DEFAULT_VAL();
    char label_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag), opt);
    ui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, UI_ELEMENT_BUTTON);

    // Push id if tag available
    if (id_tag) {
        ui_push_id(ctx, id_tag, strlen(id_tag));
    }

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, UI_ELEMENT_BUTTON);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, UI_ELEMENT_BUTTON, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, UI_ELEMENT_BUTTON, 0x01)
                                   : ui_get_current_element_style(ctx, desc, UI_ELEMENT_BUTTON, 0x00);
    }

    // Temporary copy of style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, opt);

    // handle click or button press for submission
    if (ctx->mouse_down != UI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == UI_ELEMENT_STATE_OFF_FOCUS) {
        res |= UI_RES_SUBMIT;
    }

    // draw border
    if (style.colors[UI_COLOR_BORDER].a) {
        ui_draw_box(ctx, ui_expand_rect(r, (i16*)style.border_width), (i16*)style.border_width, style.colors[UI_COLOR_BORDER]);
    }

    opt |= UI_OPT_ISCONTENT;
    ui_draw_rect(ctx, r, style.colors[UI_COLOR_BACKGROUND]);
    if (label) {
        ui_draw_control_text(ctx, label_tag, r, &style, opt);
    }

    ui_pop_style(ctx, save);

    if (id_tag) ui_pop_id(ctx);

    return res;
}

i32 ui_checkbox_ex(ui_context_t* ctx, const char* label, i32* state, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0;
    ui_id id = ui_get_id(ctx, &state, sizeof(state));
    ui_rect_t r = ui_layout_next(ctx);
    ui_rect_t box = ui_rect(r.x, r.y, r.h, r.h);
    i32 ox = (i32)(box.w * 0.2f), oy = (i32)(box.h * 0.2f);
    ui_rect_t inner_box = ui_rect(box.x + ox, box.y + oy, box.w - 2 * ox, box.h - 2 * oy);
    ui_update_control(ctx, id, r, 0);

    i32 elementid = UI_ELEMENT_BUTTON;
    ui_style_t style = NEKO_DEFAULT_VAL();
    style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
            : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                               : ui_get_current_element_style(ctx, desc, elementid, 0x00);

    // handle click
    if ((ctx->mouse_pressed == UI_MOUSE_LEFT || (ctx->mouse_pressed && ~opt & UI_OPT_LEFTCLICKONLY)) && ctx->focus == id) {
        res |= UI_RES_CHANGE;
        *state = !*state;
    }

    // draw
    ui_draw_control_frame(ctx, id, box, UI_ELEMENT_INPUT, 0);
    if (*state) {
        // Draw in a filled rect
        ui_draw_rect(ctx, inner_box, style.colors[UI_COLOR_BACKGROUND]);
    }

    r = ui_rect(r.x + box.w, r.y, r.w - box.w, r.h);
    ui_draw_control_text(ctx, label, r, &ctx->style_sheet->styles[UI_ELEMENT_TEXT][0], 0);
    return res;
}

i32 ui_textbox_raw(ui_context_t* ctx, char* buf, i32 bufsz, ui_id id, ui_rect_t rect, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0;

    i32 elementid = UI_ELEMENT_INPUT;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        // Need to check that I haven't updated more than once this frame
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Push temp style
    ui_style_t* save = ui_push_style(ctx, &style);

    ui_update_control(ctx, id, rect, opt | UI_OPT_HOLDFOCUS);

    if (ctx->focus == id) {
        // handle text input
        i32 len = strlen(buf);
        i32 n = NEKO_MIN(bufsz - len - 1, (i32)strlen(ctx->input_text));
        if (n > 0) {
            memcpy(buf + len, ctx->input_text, n);
            len += n;
            buf[len] = '\0';
            res |= UI_RES_CHANGE;
        }

        // handle backspace
        if (ctx->key_pressed & UI_KEY_BACKSPACE && len > 0) {
            if (ctx->key_down & UI_KEY_CTRL) {
                for (--len; len > 0; len--) {
                    // 跳过 utf-8 连续字节
                    if ((buf[len - 1] & 0xc0) == 0x80) continue;
                    // 查找直到分隔符
                    if (strchr(" ()[]{},.-+*=/\\^~|\"'&%#@!<>;:", buf[len - 1])) break;
                }
            } else {
                // 跳过 utf-8 连续字节
                while ((buf[--len] & 0xc0) == 0x80 && len > 0);
            }
            buf[len] = '\0';
            res |= UI_RES_CHANGE;
        }

        // TODO: 处理粘贴
        // if (neko_os_key_pressed(NEKO_KEYCODE_V) && ctx->key_down & UI_KEY_CTRL) {
        //     const_str clipboard = neko_pf_window_get_clipboard(ctx->window_hndl);
        //     printf("%s --\n", clipboard);
        //     i32 n = NEKO_MIN(bufsz - len - 1, (i32)strlen(clipboard));
        //     if (n > 0) {
        //         memcpy(buf + len, clipboard, n);
        //         len += n;
        //         buf[len] = '\0';
        //         res |= UI_RES_CHANGE;
        //     }
        // }

        // handle return
        if (ctx->key_pressed & UI_KEY_RETURN) {
            ui_set_focus(ctx, 0);
            res |= UI_RES_SUBMIT;
        }
    }

    // draw

    // Textbox border
    ui_draw_box(ctx, ui_expand_rect(rect, (i16*)style.border_width), (i16*)style.border_width, style.colors[UI_COLOR_BORDER]);

    // Textbox bg
    ui_draw_control_frame(ctx, id, rect, UI_ELEMENT_INPUT, opt);

    // Text and carret
    if (ctx->focus == id) {
        ui_style_t* sp = &style;
        Color256* color = &sp->colors[UI_COLOR_CONTENT];
        i32 sx = sp->shadow_x;
        i32 sy = sp->shadow_y;
        Color256* sc = &sp->colors[UI_COLOR_SHADOW];
        FontFamily* font = sp->font;
        i32 textw = ui_text_width(font, buf, -1);
        i32 texth = ui_font_height(font);
        i32 ofx = (i32)(rect.w - sp->padding[UI_PADDING_RIGHT] - textw - 1);
        i32 textx = (i32)(rect.x + NEKO_MIN(ofx, sp->padding[UI_PADDING_LEFT]));
        i32 texty = (i32)(rect.y + (rect.h - texth) / 2);
        i32 cary = (i32)(rect.y + 1);
        ui_push_clip_rect(ctx, rect);

        // Draw text
        ui_draw_control_text(ctx, buf, rect, &style, opt);

        // Draw caret (control alpha based on frame)
        static bool on = true;
        static float ct = 0.f;
        if (~opt & UI_OPT_NOCARET) {
            vec2 pos = neko_v2(rect.x, rect.y);

            // Grab stylings
            const i32 padding_left = sp->padding[UI_PADDING_LEFT];
            const i32 padding_top = sp->padding[UI_PADDING_TOP];
            const i32 padding_right = sp->padding[UI_PADDING_RIGHT];
            const i32 padding_bottom = sp->padding[UI_PADDING_BOTTOM];
            const i32 align = sp->align_content;
            const i32 justify = sp->justify_content;

            // Determine x placement based on justification
            switch (justify) {
                default:
                case UI_JUSTIFY_START: {
                    pos.x = rect.x + padding_left;
                } break;

                case UI_JUSTIFY_CENTER: {
                    pos.x = rect.x + (rect.w - textw) * 0.5f;
                } break;

                case UI_JUSTIFY_END: {
                    pos.x = rect.x + (rect.w - textw) - padding_right;
                } break;
            }

            // Determine caret position based on style justification
            ui_rect_t cr = ui_rect(pos.x + textw + padding_right, (f32)rect.y + 5.f, 1.f, (f32)rect.h - 10.f);

            if (ctx->last_focus_state == UI_ELEMENT_STATE_ON_FOCUS) {
                on = true;
                ct = 0.f;
            }
            ct += 0.1f;
            if (ct >= 3.f) {
                on = !on;
                ct = 0.f;
            }
            Color256 col = *color;
            col.a = on ? col.a : 0;
            ui_draw_rect(ctx, cr, col);
        }

        ui_pop_clip_rect(ctx);
    } else {
        ui_style_t* sp = &style;
        Color256* color = &sp->colors[UI_COLOR_CONTENT];
        FontFamily* font = sp->font;
        i32 sx = sp->shadow_x;
        i32 sy = sp->shadow_y;
        Color256* sc = &sp->colors[UI_COLOR_SHADOW];
        i32 textw = ui_text_width(font, buf, -1);
        i32 texth = ui_text_height(font, buf, -1);
        i32 textx = (i32)(rect.x + sp->padding[UI_PADDING_LEFT]);
        i32 texty = (i32)(rect.y + (rect.h - texth) / 2);
        ui_push_clip_rect(ctx, rect);
        ui_draw_control_text(ctx, buf, rect, &style, opt);
        ui_pop_clip_rect(ctx);
    }

    ui_pop_style(ctx, save);

    return res;
}

static i32 ui_number_textbox(ui_context_t* ctx, ui_real* value, ui_rect_t r, ui_id id, const ui_selector_desc_t* desc) {
    if (ctx->mouse_pressed == UI_MOUSE_LEFT && ctx->key_down & UI_KEY_SHIFT && ctx->hover == id) {
        ctx->number_edit = id;
        neko_snprintf(ctx->number_edit_buf, UI_MAX_FMT, UI_REAL_FMT, *value);
    }
    if (ctx->number_edit == id) {
        // This is broken for some reason...
        i32 res = ui_textbox_raw(ctx, ctx->number_edit_buf, sizeof(ctx->number_edit_buf), id, r, desc, 0);

        if (res & UI_RES_SUBMIT || ctx->focus != id) {
            *value = strtod(ctx->number_edit_buf, NULL);
            ctx->number_edit = 0;
        } else {
            return 1;
        }
    }
    return 0;
}

i32 ui_textbox_ex(ui_context_t* ctx, char* buf, i32 bufsz, const ui_selector_desc_t* desc, u64 opt) {
    // Handle animation here...
    i32 res = 0;
    ui_id id = ui_get_id(ctx, &buf, sizeof(buf));
    i32 elementid = UI_ELEMENT_INPUT;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        // Need to check that I haven't updated more than once this frame
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Push temp style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, opt | UI_OPT_HOLDFOCUS);
    res |= ui_textbox_raw(ctx, buf, bufsz, id, r, desc, opt);
    ui_pop_style(ctx, save);

    return res;
}

i32 ui_slider_ex(ui_context_t* ctx, ui_real* value, ui_real low, ui_real high, ui_real step, const char* fmt, const ui_selector_desc_t* desc, u64 opt) {
    char buf[UI_MAX_FMT + 1];
    ui_rect_t thumb;
    i32 x, w, res = 0;
    ui_real last = *value, v = last;
    ui_id id = ui_get_id(ctx, &value, sizeof(value));
    i32 elementid = UI_ELEMENT_INPUT;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);
    i32 state = ctx->focus == id ? UI_ELEMENT_STATE_FOCUS : ctx->hover == id ? UI_ELEMENT_STATE_HOVER : UI_ELEMENT_STATE_DEFAULT;

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ui_get_current_element_style(ctx, desc, elementid, state);
    }

    // Temporary copy of style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t base = ui_layout_next(ctx);

    // handle text input mode
    if (ui_number_textbox(ctx, &v, base, id, desc)) {
        return res;
    }

    // handle normal mode
    ui_update_control(ctx, id, base, opt);

    // handle input
    if (ctx->focus == id && (ctx->mouse_down | ctx->mouse_pressed) == UI_MOUSE_LEFT) {
        v = low + (ctx->mouse_pos.x - base.x) * (high - low) / base.w;
        if (step) {
            v = (((v + step / 2) / step)) * step;
        }
    }

    // clamp and store value, update res
    *value = v = NEKO_CLAMP(v, low, high);
    if (last != v) {
        res |= UI_RES_CHANGE;
    }

    // draw base
    ui_draw_control_frame(ctx, id, base, UI_ELEMENT_INPUT, opt);

    // draw control
    w = style.thumb_size;  // Don't like this...
    x = (i32)((v - low) * (base.w - w) / (high - low));
    thumb = ui_rect((f32)base.x + (f32)x, base.y, (f32)w, base.h);
    ui_draw_control_frame(ctx, id, thumb, UI_ELEMENT_BUTTON, opt);

    // draw text
    style.colors[UI_COLOR_BACKGROUND] = ctx->style_sheet->styles[UI_ELEMENT_TEXT][state].colors[UI_COLOR_BACKGROUND];
    neko_snprintf(buf, UI_MAX_FMT, fmt, v);
    ui_draw_control_text(ctx, buf, base, &style, opt);  // oh...bg

    // Pop style
    ui_pop_style(ctx, save);

    return res;
}

i32 ui_number_ex(ui_context_t* ctx, ui_real* value, ui_real step, const char* fmt, const ui_selector_desc_t* desc, u64 opt) {
    char buf[UI_MAX_FMT + 1];
    i32 res = 0;
    ui_id id = ui_get_id(ctx, &value, sizeof(value));
    i32 elementid = UI_ELEMENT_INPUT;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Temporary copy of style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t base = ui_layout_next(ctx);
    ui_real last = *value;

    // handle text input mode
    if (ui_number_textbox(ctx, value, base, id, desc)) {
        ui_pop_style(ctx, save);
        return res;
    }

    // handle normal mode
    ui_update_control(ctx, id, base, opt);

    // handle input
    if (ctx->focus == id && ctx->mouse_down == UI_MOUSE_LEFT) {
        *value += ctx->mouse_delta.x * step;
    }

    // set flag if value changed
    if (*value != last) {
        res |= UI_RES_CHANGE;
    }

    // draw base
    ui_draw_control_frame(ctx, id, base, UI_ELEMENT_INPUT, opt);

    // draw text
    neko_snprintf(buf, UI_MAX_FMT, fmt, *value);
    ui_draw_control_text(ctx, buf, base, &ctx->style_sheet->styles[UI_ELEMENT_TEXT][0], opt);

    ui_pop_style(ctx, save);

    return res;
}

static i32 __ui_header(ui_context_t* ctx, const char* label, i32 istreenode, const ui_selector_desc_t* desc, u64 opt) {
    ui_rect_t r;
    i32 active, expanded;
    i32 width = -1;
    ui_layout_row(ctx, 1, &width, 0);

    char id_tag[256] = NEKO_DEFAULT_VAL();
    char label_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag), opt);
    ui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    ui_id id = ui_get_id(ctx, id_tag, strlen(id_tag));
    i32 idx = ui_pool_get(ctx, ctx->treenode_pool, UI_TREENODEPOOL_SIZE, id);

    if (id_tag) ui_push_id(ctx, id_tag, strlen(id_tag));

    active = (idx >= 0);
    expanded = (opt & UI_OPT_EXPANDED) ? !active : active;
    r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, 0);

    // handle click
    active ^= (ctx->mouse_pressed == UI_MOUSE_LEFT && ctx->focus == id);

    // update pool ref
    if (idx >= 0) {
        if (active) {
            ui_pool_update(ctx, ctx->treenode_pool, idx);
        } else {
            memset(&ctx->treenode_pool[idx], 0, sizeof(ui_pool_item_t));
        }

    } else if (active) {
        ui_pool_init(ctx, ctx->treenode_pool, UI_TREENODEPOOL_SIZE, id);
    }

    // draw
    if (istreenode) {
        if (ctx->hover == id) {
            ui_draw_frame(ctx, r, &ctx->style_sheet->styles[UI_ELEMENT_BUTTON][UI_ELEMENT_STATE_HOVER]);
        }
    } else {
        ui_draw_control_frame(ctx, id, r, UI_ELEMENT_BUTTON, 0);
    }

    const float sz = 6.f;
    if (expanded) {
        vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        vec2 b = vec2_add(a, neko_v2(sz, 0.f));
        vec2 c = vec2_add(a, neko_v2(sz / 2.f, sz));
        ui_draw_triangle(ctx, a, b, c, ctx->style_sheet->styles[UI_ELEMENT_TEXT][0x00].colors[UI_COLOR_CONTENT]);
    } else {
        vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        vec2 b = vec2_add(a, neko_v2(sz, sz / 2.f));
        vec2 c = vec2_add(a, neko_v2(0.f, sz));
        ui_draw_triangle(ctx, a, b, c, ctx->style_sheet->styles[UI_ELEMENT_TEXT][0x00].colors[UI_COLOR_CONTENT]);
    }

    // Draw text for treenode
    r.x += r.h - ctx->style->padding[UI_PADDING_TOP];
    r.w -= r.h - ctx->style->padding[UI_PADDING_BOTTOM];
    ui_draw_control_text(ctx, label_tag, r, &ctx->style_sheet->styles[UI_ELEMENT_TEXT][0x00], 0);

    if (id_tag) ui_pop_id(ctx);

    return expanded ? UI_RES_ACTIVE : 0;
}

i32 ui_header_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) { return __ui_header(ctx, label, 0, desc, opt); }

i32 ui_treenode_begin_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = __ui_header(ctx, label, 1, desc, opt);
    if (res & UI_RES_ACTIVE) {
        ui_get_layout(ctx)->indent += ctx->style->indent;
        ui_stack_push(ctx->id_stack, ctx->last_id);
    }

    return res;
}

void ui_treenode_end(ui_context_t* ctx) {
    ui_get_layout(ctx)->indent -= ctx->style->indent;
    ui_pop_id(ctx);
}

// -1 for left, + 1 for right
void ui_tab_item_swap(ui_context_t* ctx, ui_container_t* cnt, i32 direction) {
    ui_tab_bar_t* tab_bar = ui_get_tab_bar(ctx, cnt);
    if (!tab_bar) return;

    i32 item = (i32)cnt->tab_item;
    i32 idx = NEKO_CLAMP(item + direction, 0, (i32)tab_bar->size - 1);

    ui_container_t* scnt = (ui_container_t*)tab_bar->items[idx].data;

    ui_tab_item_t* cti = &tab_bar->items[cnt->tab_item];
    ui_tab_item_t* sti = &tab_bar->items[idx];
    ui_tab_item_t tmp = *cti;

    // Swap cti
    sti->data = cnt;
    cnt->tab_item = sti->idx;

    // Swap sti
    cti->data = scnt;
    scnt->tab_item = cti->idx;

    tab_bar->focus = sti->idx;
}

i32 ui_window_begin_ex(ui_context_t* ctx, const char* title, ui_rect_t rect, bool* open, const ui_selector_desc_t* desc, u64 opt) {
    ui_rect_t body;
    ui_id id = ui_get_id(ctx, title, strlen(title));
    ui_container_t* cnt = ui_get_container_ex(ctx, id, opt);

    char id_tag[256] = NEKO_DEFAULT_VAL();
    char label_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, title, id_tag, sizeof(id_tag), opt);
    ui_parse_label_tag(ctx, title, label_tag, sizeof(label_tag));

    if (cnt && open) {
        cnt->open = *open;
    }

    if (!cnt || !cnt->open) {
        return 0;
    }

    memcpy(cnt->name, label_tag, 256);

    const i32 title_max_size = 100;

    bool new_frame = cnt->frame != ctx->frame;

    i32 state = ctx->active_root == cnt ? UI_ELEMENT_STATE_FOCUS : ctx->hover_root == cnt ? UI_ELEMENT_STATE_HOVER : UI_ELEMENT_STATE_DEFAULT;

    const float split_size = UI_SPLIT_SIZE;

    ui_stack_push(ctx->id_stack, id);

    // Get splits
    ui_split_t* split = ui_get_split(ctx, cnt);
    ui_split_t* root_split = ui_get_root_split(ctx, cnt);

    // Get root container
    ui_container_t* root_cnt = ui_get_root_container(ctx, cnt);

    // Cache rect
    if ((cnt->rect.w == 0.f || opt & UI_OPT_FORCESETRECT || opt & UI_OPT_FULLSCREEN || cnt->flags & UI_WINDOW_FLAGS_FIRST_INIT) && new_frame) {
        if (opt & UI_OPT_FULLSCREEN) {
            vec2 fb = ctx->framebuffer_size;
            cnt->rect = ui_rect(0, 0, fb.x, fb.y);

            // Set root split rect size
            if (root_split) {
                root_split->rect = cnt->rect;
                ui_update_split(ctx, root_split);
            }
        } else {
            // Set root split rect size
            if (root_split && root_cnt == cnt) {
                root_split->rect = rect;
                ui_update_split(ctx, root_split);
            } else {
                cnt->rect = rect;
            }
        }
        cnt->flags = cnt->flags & ~UI_WINDOW_FLAGS_FIRST_INIT;
    }
    ui_begin_root_container(ctx, cnt, opt);
    rect = body = cnt->rect;
    cnt->opt = opt;

    if (opt & UI_OPT_DOCKSPACE) {
        cnt->zindex = 0;
    }

    // If parent cannot move/resize, set to this opt as well
    if (root_cnt->opt & UI_OPT_NOMOVE) {
        cnt->opt |= UI_OPT_NOMOVE;
    }

    if (root_cnt->opt & UI_OPT_NORESIZE) {
        cnt->opt |= UI_OPT_NORESIZE;
    }

    // If in a tab view, then title has to be handled differently...
    ui_tab_bar_t* tab_bar = cnt->tab_bar ? neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar) : NULL;
    ui_tab_item_t* tab_item = tab_bar ? &tab_bar->items[cnt->tab_item] : NULL;

    if (tab_item && tab_item) {
        if (tab_bar->focus == tab_item->idx) {
            cnt->flags |= UI_WINDOW_FLAGS_VISIBLE;
            cnt->opt &= !UI_OPT_NOINTERACT;
            cnt->opt &= !UI_OPT_NOHOVER;
        } else {
            cnt->flags &= ~UI_WINDOW_FLAGS_VISIBLE;
            cnt->opt |= UI_OPT_NOINTERACT;
            cnt->opt |= UI_OPT_NOHOVER;
        }
    }

    bool in_root = false;

    // If hovered root is in the tab group and moused over, then is hovered
    if (tab_bar) {
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (ctx->hover_root == (ui_container_t*)tab_bar->items[i].data) {
                in_root = true;
                break;
            }
        }
    }

    ui_container_t* s_cnt = cnt;
    if (tab_bar && split) {
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((ui_container_t*)tab_bar->items[i].data)->split) {
                s_cnt = (ui_container_t*)tab_bar->items[i].data;
            }
        }
    }

    // Do split size/position
    if (split) {
        const ui_style_t* cstyle = &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][state];
        const ui_rect_t* sr = &split->rect;
        const float ratio = split->ratio;
        float shsz = split_size;
        const float omr = (1.f - ratio);

        switch (split->type) {
            case UI_SPLIT_LEFT: {
                if (split->children[UI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + shsz, sr->w * ratio - 2.f * shsz, sr->h - 2.f * shsz);
                } else {
                    cnt->rect = ui_rect(sr->x + sr->w * ratio + shsz, sr->y + shsz, sr->w * (1.f - ratio) - 2.f * shsz, sr->h - 2.f * shsz);
                }

            } break;

            case UI_SPLIT_RIGHT: {
                if (split->children[UI_SPLIT_NODE_PARENT].container == s_cnt) {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + shsz, sr->w * (1.f - ratio) - 2.f * shsz, sr->h - 2.f * shsz);
                } else {
                    cnt->rect = ui_rect(sr->x + sr->w * (1.f - ratio) + shsz, sr->y + shsz, sr->w * ratio - 2.f * shsz, sr->h - 2.f * shsz);
                }
            } break;

            case UI_SPLIT_TOP: {
                if (split->children[UI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + shsz, sr->w - 2.f * shsz, sr->h * ratio - 2.f * shsz);
                } else {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + sr->h * ratio + shsz, sr->w - 2.f * shsz, sr->h * (1.f - ratio) - 2.f * shsz);
                }
            } break;

            case UI_SPLIT_BOTTOM: {
                if (split->children[UI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + sr->h * (1.f - ratio) + shsz, sr->w - 2.f * shsz, sr->h * (ratio)-2.f * shsz);
                } else {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + shsz, sr->w - 2.f * shsz, sr->h * (1.f - ratio) - 2.f * shsz);
                }
            } break;
        }
    }

    // Calculate movement
    if (~cnt->opt & UI_OPT_NOTITLE && new_frame) {
        ui_rect_t* rp = root_split ? &root_split->rect : &cnt->rect;

        // Cache rect
        ui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        tr.x += split_size;
        ui_id id = ui_get_id(ctx, "!title", 6);
        ui_update_control(ctx, id, tr, opt);

        // Need to move the entire thing
        if ((id == ctx->focus || id == ctx->hover) && ctx->mouse_down == UI_MOUSE_LEFT) {
            // This log_lock id is what I need...

            ctx->active_root = cnt;

            if (tab_bar) {
                ctx->next_focus_root = (ui_container_t*)(tab_bar->items[tab_bar->focus].data);
                ui_bring_to_front(ctx, (ui_container_t*)tab_bar->items[tab_bar->focus].data);
                if (id == ctx->focus && tab_bar->focus != tab_item->idx) ctx->lock_focus = id;
            } else {
                ctx->next_focus_root = cnt;
            }

            if (root_split) {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = UI_SPLIT_MOVE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = UI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            }
        }

        // Tab view
        i32 tw = title_max_size;
        id = ui_get_id(ctx, "!split_tab", 10);
        const float hp = 0.8f;
        tr.x += split_size;
        float h = tr.h * hp;
        float y = tr.y + tr.h * (1.f - hp);

        // Will update tab bar rect size with parent window rect
        if (tab_item) {
            // Get tab bar
            ui_rect_t* r = &tab_bar->rect;

            // Determine width
            i32 tab_width = (i32)NEKO_MIN(r->w / (float)tab_bar->size, title_max_size);
            tw = tab_item->zindex ? (i32)tab_width : (i32)(tab_width + 1.f);

            // Determine position (based on zindex and total width)
            float xoff = 0.f;  // tab_item->zindex ? 2.f : 0.f;
            tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff;
        }

        ui_rect_t r = ui_rect(tr.x + split_size, y, (f32)tw, h);

        ui_update_control(ctx, id, r, opt);

        // Need to move the entire thing
        if ((id == ctx->hover || id == ctx->focus) && ctx->mouse_down == UI_MOUSE_LEFT) {
            ui_set_focus(ctx, id);
            ctx->next_focus_root = cnt;
            ctx->active_root = cnt;

            // Don't move from tab bar
            if (tab_item) {
                // Handle break out
                if (ctx->mouse_pos.y < tr.y || ctx->mouse_pos.y > tr.y + tr.h) {
                    ctx->undock_root = cnt;
                }

                if (tab_bar->focus != tab_item->idx) {
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = UI_CNT_FOCUS;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
            }

            else if (root_split) {
                // Handle break out
                if (ctx->mouse_pos.y < tr.y || ctx->mouse_pos.y > tr.y + tr.h) {
                    ctx->undock_root = cnt;
                }
            } else {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = UI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            }
        }
    }

    // Control frame for body movement
    if (~root_cnt->opt & UI_OPT_NOMOVE && ~cnt->opt & UI_OPT_NOMOVE && ~cnt->opt & UI_OPT_NOINTERACT && ~cnt->opt & UI_OPT_NOHOVER && new_frame && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        // Cache rect
        ui_rect_t br = cnt->rect;

        if (~cnt->opt & UI_OPT_NOTITLE) {
            br.y += ctx->style->title_height;
            br.h -= ctx->style->title_height;
        }
        ui_id id = ui_get_id(ctx, "!body", 5);
        // ui_update_control(ctx, id, br, (opt | UI_OPT_NOSWITCHSTATE));

        // Need to move the entire thing
        if (ctx->hover_root == cnt && !ctx->focus_split && !ctx->focus && !ctx->lock_focus && !ctx->hover && ctx->mouse_down == UI_MOUSE_LEFT) {
            ctx->active_root = cnt;
            ctx->next_focus_root = cnt;
            if (root_split) {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = UI_SPLIT_MOVE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else if (tab_bar) {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = UI_CNT_FOCUS;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);

                req.type = UI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = UI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            }
        }
    }

    // Get parent window if in tab view, then set rect to it (will be a frame off though...)
    if (tab_item && tab_bar) {
        if (tab_bar->focus == tab_item->idx || split) {
            tab_bar->rect = cnt->rect;
        } else {
            cnt->rect = tab_bar->rect;
        }
    }

    // Cache body
    body = cnt->rect;

    if (split) {
        const float sh = split_size * 0.5f;
    }

    if (~opt & UI_OPT_NOTITLE) {
        ui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        if (split) {
            const float sh = split_size * 0.5f;
        }
        body.y += tr.h;
        body.h -= tr.h;
    }

    i32 zindex = INT32_MAX - 1;
    if (root_split) {
        ui_get_split_lowest_zindex(ctx, root_split, &zindex);
        if (zindex == cnt->zindex) {
            ui_style_t* style = &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][state];
            ui_draw_rect(ctx, root_split->rect, style->colors[UI_COLOR_BACKGROUND]);
            ui_draw_splits(ctx, root_split);
        }
    }

    // draw body frame
    if (~opt & UI_OPT_NOFRAME && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        ui_style_t* style = &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][state];

        if (ctx->active_root == root_cnt) {
            i32 f = 0;
        }

        ui_draw_rect(ctx, body, style->colors[UI_COLOR_BACKGROUND]);

        // draw border (get root cnt and check state of that)
        if (split) {
            i32 root_state = ctx->active_root == root_cnt ? UI_ELEMENT_STATE_FOCUS : ctx->hover_root == root_cnt ? UI_ELEMENT_STATE_HOVER : UI_ELEMENT_STATE_DEFAULT;

            bool share_split = ctx->active_root && ui_get_root_container(ctx, ctx->active_root) == root_cnt ? true : false;

            // Have to look and see if hovered root shares split...
            ui_style_t* root_style = style;
            if (share_split) {
                root_style = &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][UI_ELEMENT_STATE_FOCUS];
            } else {
                root_style = state == UI_ELEMENT_STATE_FOCUS        ? style
                             : root_state == UI_ELEMENT_STATE_FOCUS ? &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][root_state]
                             : root_state == UI_ELEMENT_STATE_HOVER ? &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][root_state]
                                                                    : style;
            }
            if (~opt & UI_OPT_NOBORDER && root_style->colors[UI_COLOR_BORDER].a) {
                ui_draw_box(ctx, ui_expand_rect(split->rect, (i16*)root_style->border_width), (i16*)root_style->border_width, root_style->colors[UI_COLOR_BORDER]);
            }
        } else {
            if (~opt & UI_OPT_NOBORDER && style->colors[UI_COLOR_BORDER].a) {
                ui_draw_box(ctx, ui_expand_rect(cnt->rect, (i16*)style->border_width), (i16*)style->border_width, style->colors[UI_COLOR_BORDER]);
            }
        }
    }

    if (split && ~opt & UI_OPT_NOCLIP && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        i16 exp[] = {1, 1, 1, 1};
        ui_push_clip_rect(ctx, ui_expand_rect(cnt->rect, exp));
    }

    if (split) {
        const float sh = split_size * 0.5f;
        body.x += sh;
        body.w -= split_size;
    }

    // do title bar
    if (~opt & UI_OPT_NOTITLE) {
        ui_style_t* cstyle = &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][state];
        ui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        if (split) {
            const float sh = split_size * 0.5f;
        }

        // Don't draw this unless you're the bottom window or first frame in a tab group (if in editor_dockspace)
        if (tab_bar) {
            bool lowest = true;
            {
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    if (cnt->zindex > ((ui_container_t*)(tab_bar->items[i].data))->zindex) {
                        lowest = false;
                        break;
                    }
                }
                if (lowest) {
                    ui_draw_frame(ctx, tr, &ctx->style_sheet->styles[UI_ELEMENT_PANEL][0x00]);
                    // ui_draw_box(ctx, ui_expand_rect(tr, (i16*)cstyle->border_width), (i16*)cstyle->border_width, cstyle->colors[UI_COLOR_BORDER]);
                }
            }
        }

        else {
            ui_draw_frame(ctx, tr, &ctx->style_sheet->styles[UI_ELEMENT_PANEL][0x00]);
            // ui_draw_box(ctx, ui_expand_rect(tr, (i16*)cstyle->border_width), cstyle->border_width, cstyle->colors[UI_COLOR_BORDER]);
        }

        // Draw tab control
        {

            // Tab view
            i32 tw = title_max_size;
            id = ui_get_id(ctx, "!split_tab", 10);
            const float hp = 0.8f;
            tr.x += split_size;
            float h = tr.h * hp;
            float y = tr.y + tr.h * (1.f - hp);

            // Will update tab bar rect size with parent window rect
            if (tab_item) {
                // Get tab bar
                ui_rect_t* r = &tab_bar->rect;

                // Determine width
                i32 tab_width = (i32)NEKO_MIN(r->w / (float)tab_bar->size, title_max_size);
                tw = (i32)(tab_width - 2.f);

                // Determine position (based on zindex and total width)
                float xoff = !tab_item->zindex ? split_size : 2.f;  // tab_item->zindex ? 2.f : 0.f;
                tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff;
            }

            ui_rect_t r = ui_rect(tr.x + split_size, y, (f32)tw, h);

            bool hovered = false;

            if (in_root && ui_rect_overlaps_vec2(r, ctx->mouse_pos)) {
                idraw_t* dl = &ctx->overlay_draw_list;
                // neko_idraw_rectvd(dl, neko_v2(r.x, r.y), neko_v2(r.w, r.h), neko_v2s(0.f), neko_v2s(1.f), NEKO_COLOR_WHITE, R_PRIMITIVE_LINES);
                hovered = true;
            }

            bool other_root_active = ctx->focus_root != cnt;
            if (tab_bar) {
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    if (tab_bar->items[i].data == ctx->focus_root) {
                        other_root_active = false;
                    }
                }
            }

            if (!other_root_active && hovered && ctx->mouse_down == UI_MOUSE_LEFT && !ctx->lock_focus) {
                // This is an issue...
                ui_set_focus(ctx, id);
                ctx->lock_focus = id;

                if (tab_item && tab_bar->focus != tab_item->idx) {
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = UI_CNT_FOCUS;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
            }

            if (!other_root_active && ctx->mouse_down == UI_MOUSE_LEFT && ctx->focus == id) {
                if (ctx->mouse_pos.x < r.x) {
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = UI_TAB_SWAP_LEFT;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
                if (ctx->mouse_pos.x > r.x + r.w) {
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = UI_TAB_SWAP_RIGHT;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
            }

            bool tab_focus = (!tab_bar || (tab_bar && tab_item && tab_bar->focus == tab_item->idx));

            Color256 def = ctx->style_sheet->styles[UI_ELEMENT_BUTTON][0x00].colors[UI_COLOR_BACKGROUND];
            Color256 hov = ctx->style_sheet->styles[UI_ELEMENT_BUTTON][0x01].colors[UI_COLOR_BACKGROUND];
            Color256 act = ctx->style_sheet->styles[UI_ELEMENT_BUTTON][0x02].colors[UI_COLOR_BACKGROUND];
            Color256 inactive = color256(10, 10, 10, 50);

            i16 exp[] = {1, 1, 1, 1};
            ui_push_clip_rect(ctx, ui_expand_rect(cnt->rect, exp));

            ui_push_clip_rect(ctx, r);

            ui_draw_rect(ctx, r, id == ctx->focus ? act : hovered ? hov : tab_focus ? def : inactive);
            ui_draw_control_text(ctx, label_tag, r, &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][state], opt);

            ui_pop_clip_rect(ctx);
            ui_pop_clip_rect(ctx);
        }

        // do `close` button
        /*
        if (~opt & UI_OPT_NOCLOSE && false)
        {
            ui_id id = ui_get_id(ctx, "!close", 6);
            ui_rect_t r = ui_rect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
            tr.w -= r.w;
            ui_draw_icon(ctx, UI_ICON_CLOSE, r, ctx->style->colors[UI_COLOR_TITLETEXT]);
            ui_update_control(ctx, id, r, opt);
            if (ctx->mouse_pressed == UI_MOUSE_LEFT && id == ctx->focus)
            {
                cnt->open = 0;
            }
        }
        */
    }

    // resize to content size
    if (opt & UI_OPT_AUTOSIZE && !split) {
        /*
        ui_rect_t r = ui_get_layout(ctx)->body;
        cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
        cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
        */
    }

    if (split && ~opt & UI_OPT_NOCLIP && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        ui_pop_clip_rect(ctx);
    }

    // Draw border
    if (~opt & UI_OPT_NOFRAME && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        const int* w = (int*)ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][0x00].border_width;
        const Color256* bc = &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][0x00].colors[UI_COLOR_BORDER];
        // ui_draw_box(ctx, ui_expand_rect(cnt->rect, w), w, *bc);
    }

    ui_push_container_body(ctx, cnt, body, desc, opt);

    // close if this is a popup window and elsewhere was clicked
    if (opt & UI_OPT_POPUP && ctx->mouse_pressed && ctx->hover_root != cnt) {
        cnt->open = 0;
    }

    if (~opt & UI_OPT_NOCLIP) {
        if (cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
            ui_push_clip_rect(ctx, cnt->body);
        } else {
            ui_push_clip_rect(ctx, ui_rect(0, 0, 0, 0));
        }
    }

    return UI_RES_ACTIVE;
}

void ui_window_end(ui_context_t* ctx) {
    ui_container_t* cnt = ui_get_current_container(ctx);

    // Get root container
    ui_container_t* root_cnt = ui_get_root_container(ctx, cnt);

    // Get splits
    ui_split_t* split = ui_get_split(ctx, cnt);
    ui_split_t* root_split = ui_get_root_split(ctx, cnt);

    const bool new_frame = cnt->frame != ctx->frame;

    // Cache opt
    const u64 opt = cnt->opt;

    // Pop clip for rect
    if (~cnt->opt & UI_OPT_NOCLIP) {
        ui_pop_clip_rect(ctx);
    }

    if (~cnt->opt & UI_OPT_NOCLIP) {
        ui_push_clip_rect(ctx, cnt->rect);
    }

    // do `resize` handle
    if (~cnt->opt & UI_OPT_NORESIZE && ~root_cnt->opt & UI_OPT_NORESIZE && new_frame && ~cnt->opt & UI_OPT_DOCKSPACE) {
        i32 sz = ctx->style->title_height;
        ui_id id = ui_get_id(ctx, "!resize", 7);
        ui_rect_t r = ui_rect(cnt->rect.x + cnt->rect.w - (f32)sz, cnt->rect.y + cnt->rect.h - (f32)sz, (f32)sz, (f32)sz);
        ui_update_control(ctx, id, r, opt);
        if (id == ctx->focus && ctx->mouse_down == UI_MOUSE_LEFT) {
            ctx->active_root = cnt;
            if (root_split) {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = UI_SPLIT_RESIZE_SE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                cnt->rect.w = NEKO_MAX(96, cnt->rect.w + ctx->mouse_delta.x);
                cnt->rect.h = NEKO_MAX(64, cnt->rect.h + ctx->mouse_delta.y);
            }
        }

        // Draw resize icon (this will also be a callback)
        const u32 grid = 5;
        const float w = r.w / (float)grid;
        const float h = r.h / (float)grid;
        const float m = 2.f;
        const float o = 5.f;

        Color256 col = ctx->focus == id   ? ctx->style_sheet->styles[UI_ELEMENT_BUTTON][0x02].colors[UI_COLOR_BACKGROUND]
                       : ctx->hover == id ? ctx->style_sheet->styles[UI_ELEMENT_BUTTON][0x01].colors[UI_COLOR_BACKGROUND]
                                          : ctx->style_sheet->styles[UI_ELEMENT_BUTTON][0x00].colors[UI_COLOR_BACKGROUND];

        ui_draw_rect(ctx, ui_rect(r.x + w * grid - o, r.y + h * (grid - 2) - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * grid - o, r.y + h * (grid - 1) - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * (grid - 1) - o, r.y + h * (grid - 1) - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * grid - o, r.y + h * grid - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * (grid - 1) - o, r.y + h * grid - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * (grid - 2) - o, r.y + h * grid - o, w - m, h - m), col);
    }

    if (~cnt->opt & UI_OPT_NOCLIP) {
        ui_pop_clip_rect(ctx);
    }

    // draw shadow
    if (~opt & UI_OPT_NOFRAME && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        ui_rect_t* r = &cnt->rect;
        u32 ssz = (u32)(split ? UI_SPLIT_SIZE : 5);

        ui_draw_rect(ctx, ui_rect(r->x, r->y + r->h, r->w + 1, 1), ctx->style->colors[UI_COLOR_SHADOW]);
        ui_draw_rect(ctx, ui_rect(r->x, r->y + r->h, r->w + (f32)ssz, (f32)ssz), ctx->style->colors[UI_COLOR_SHADOW]);
        ui_draw_rect(ctx, ui_rect(r->x + r->w, r->y, 1, r->h), ctx->style->colors[UI_COLOR_SHADOW]);
        ui_draw_rect(ctx, ui_rect(r->x + r->w, r->y, (f32)ssz, r->h), ctx->style->colors[UI_COLOR_SHADOW]);
    }

#define _gui_window_resize_ctrl(ID, RECT, MOUSE, SPLIT_TYPE, MOD_KEY, ...) \
    do {                                                                   \
        if (ctx->key_down == (MOD_KEY)) {                                  \
            ui_id _ID = (ID);                                              \
            ui_rect_t _R = (RECT);                                         \
            ui_update_control(ctx, (ID), _R, opt);                         \
                                                                           \
            if (_ID == ctx->hover || _ID == ctx->focus) {                  \
                ui_draw_rect(ctx, _R, NEKO_COLOR_WHITE);                   \
            }                                                              \
                                                                           \
            if (_ID == ctx->focus && ctx->mouse_down == (MOUSE)) {         \
                ui_draw_rect(ctx, _R, NEKO_COLOR_WHITE);                   \
                if (root_split) {                                          \
                    ui_request_t req = NEKO_DEFAULT_VAL();                 \
                    req.type = (SPLIT_TYPE);                               \
                    req.split = root_split;                                \
                    neko_dyn_array_push(ctx->requests, req);               \
                } else if (new_frame) {                                    \
                    __VA_ARGS__                                            \
                }                                                          \
            }                                                              \
        }                                                                  \
    } while (0)

    // Control frame for body resize
    if (~opt & UI_OPT_NORESIZE && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        // Cache main rect
        ui_rect_t* r = root_split ? &root_split->rect : &cnt->rect;
        ui_rect_t* cr = &cnt->rect;

        const float border_ratio = 0.333f;

        if (split) {
            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_w", 8), ui_rect(cr->x, cr->y + cr->h * border_ratio, cr->w * border_ratio, cr->h * (1.f - 2.f * border_ratio)), UI_MOUSE_RIGHT,
                                    UI_SPLIT_RESIZE_INVALID, UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_e", 8),
                                    ui_rect(cr->x + cr->w * (1.f - border_ratio), cr->y + cr->h * border_ratio, cr->w * border_ratio, cr->h * (1.f - 2.f * border_ratio)), UI_MOUSE_LEFT,
                                    UI_SPLIT_RESIZE_INVALID, UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_n", 8), ui_rect(cr->x + cr->w * border_ratio, cr->y, cr->w * (1.f - 2.f * border_ratio), cr->h * border_ratio), UI_MOUSE_LEFT,
                                    UI_SPLIT_RESIZE_INVALID, UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_s", 8),
                                    ui_rect(cr->x + cr->w * border_ratio, cr->y + cr->h * (1.f - border_ratio), cr->w * (1.f - 2.f * border_ratio), cr->h * border_ratio), UI_MOUSE_LEFT,
                                    UI_SPLIT_RESIZE_INVALID, UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_se", 9), ui_rect(cr->x + cr->w - cr->w * border_ratio, cr->y + cr->h * (1.f - border_ratio), cr->w * border_ratio, cr->h * border_ratio),
                                    UI_MOUSE_LEFT, UI_SPLIT_RESIZE_INVALID, UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_ne", 9), ui_rect(cr->x + cr->w - cr->w * border_ratio, cr->y, cr->w * border_ratio, cr->h * border_ratio), UI_MOUSE_LEFT,
                                    UI_SPLIT_RESIZE_INVALID, UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_nw", 9), ui_rect(cr->x, cr->y, cr->w * border_ratio, cr->h * border_ratio), UI_MOUSE_LEFT, UI_SPLIT_RESIZE_INVALID, UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_sw", 9), ui_rect(cr->x, cr->y + cr->h - cr->h * border_ratio, cr->w * border_ratio, cr->h * border_ratio), UI_MOUSE_LEFT,
                                    UI_SPLIT_RESIZE_INVALID, UI_KEY_CTRL, {});
        }

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_w", 6), ui_rect(r->x, r->y + r->h * border_ratio, r->w * border_ratio, r->h * (1.f - 2.f * border_ratio)), UI_MOUSE_LEFT, UI_SPLIT_RESIZE_W,
                                UI_KEY_ALT, {
                                    float w = r->w;
                                    float max_x = r->x + r->w;
                                    r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                                    if (fabsf(r->w - w) > 0.f) {
                                        r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
                                    }
                                });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_e", 6), ui_rect(r->x + r->w * (1.f - border_ratio), r->y + r->h * border_ratio, r->w * border_ratio, r->h * (1.f - 2.f * border_ratio)),
                                UI_MOUSE_LEFT, UI_SPLIT_RESIZE_E, UI_KEY_ALT, { r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40); });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_n", 6), ui_rect(r->x + r->w * border_ratio, r->y, r->w * (1.f - 2.f * border_ratio), r->h * border_ratio), UI_MOUSE_LEFT, UI_SPLIT_RESIZE_N,
                                UI_KEY_ALT, {
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                                    if (fabsf(r->h - h) > 0.f) {
                                        r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
                                    }
                                });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_s", 6), ui_rect(r->x + r->w * border_ratio, r->y + r->h * (1.f - border_ratio), r->w * (1.f - 2.f * border_ratio), r->h * border_ratio),
                                UI_MOUSE_LEFT, UI_SPLIT_RESIZE_S, UI_KEY_ALT, { r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40); });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_se", 7), ui_rect(r->x + r->w - r->w * border_ratio, r->y + r->h * (1.f - border_ratio), r->w * border_ratio, r->h * border_ratio), UI_MOUSE_LEFT,
                                UI_SPLIT_RESIZE_SE, UI_KEY_ALT, {
                                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);
                                });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_ne", 7), ui_rect(r->x + r->w - r->w * border_ratio, r->y, r->w * border_ratio, r->h * border_ratio), UI_MOUSE_LEFT, UI_SPLIT_RESIZE_NE, UI_KEY_ALT,
                                {
                                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                                    if (fabsf(r->h - h) > 0.f) {
                                        r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
                                    }
                                });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_nw", 7), ui_rect(r->x, r->y, r->w * border_ratio, r->h * border_ratio), UI_MOUSE_LEFT, UI_SPLIT_RESIZE_NW, UI_KEY_ALT, {
            float h = r->h;
            float max_y = h + r->y;
            r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
            if (fabsf(r->h - h) > 0.f) {
                r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
            }

            float w = r->w;
            float max_x = r->x + r->w;
            r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
            if (fabsf(r->w - w) > 0.f) {
                r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
            }
        });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_sw", 7), ui_rect(r->x, r->y + r->h - r->h * border_ratio, r->w * border_ratio, r->h * border_ratio), UI_MOUSE_LEFT, UI_SPLIT_RESIZE_SW, UI_KEY_ALT,
                                {
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);

                                    float w = r->w;
                                    float max_x = r->x + r->w;
                                    r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                                    if (fabsf(r->w - w) > 0.f) {
                                        r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
                                    }
                                });

        // move instead of resize?
        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_c", 6), ui_rect(r->x + r->w * border_ratio, r->y + r->h * border_ratio, r->w * border_ratio, r->h * border_ratio), UI_MOUSE_LEFT, UI_SPLIT_MOVE,
                                UI_KEY_ALT, {
                                    ctx->next_focus_root = cnt;
                                    r->x += ctx->mouse_delta.x;
                                    r->y += ctx->mouse_delta.y;
                                });

        static bool capture = false;
        static vec2 mp = {0};
        static ui_rect_t _rect = {0};

        /*
        _gui_window_resize_ctrl(
            ui_get_id(ctx, "!res_c", 5),
            ui_rect(r->x + r->w * border_ratio, r->y + r->h * border_ratio, r->w * border_ratio, r->h * border_ratio),
            UI_SPLIT_RESIZE_CENTER,
            {
                if (!capture)
                {
                    capture = true;
                    mp = ctx->mouse_pos;
                    _rect = *r;
                }

                // Grow based on dist from center
                vec2 c = neko_v2(r->x + r->w * 0.5f, r->y + r->h * 0.5f);
                vec2 a = vec2_sub(c, mp);
                vec2 b = vec2_sub(c, ctx->mouse_pos);
                vec2 na = vec2_norm(a);
                vec2 nb = vec2_norm(b);
                float dist = vec2_len(vec2_sub(b, a));
                float dot = vec2_dot(na, nb);
                neko_println("len: %.2f, dot: %.2f", dist, dot);

                // Grow rect by dot product (scale dimensions)
                float sign = dot >= 0.f ? 1.f : -1.f;
                float factor = 1.f - dist / 1000.f;
                r->w = _rect.w * factor * sign;
                r->h = _rect.h * factor * sign;

                // Equidistant resize from middle (grow rect based on delta)
                float h = r->h;
                float max_y = h + r->y;
                r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                if (fabsf(r->h - h) > 0.f)
                {
                    r->y = NEKO_MIN(r->y - ctx->mouse_delta.y, max_y);
                }

                float w = r->w;
                float max_x = r->x + r->w;
                r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                if (fabsf(r->w - w) > 0.f)
                {
                    r->x = NEKO_MIN(r->x - ctx->mouse_delta.x, max_x);
                }
            });
        */

        if (ctx->mouse_down != UI_MOUSE_LEFT) {
            capture = false;
            mp = neko_v2s(0.f);
        }
    }

    // Determine if focus root in same tab group as current window for docking
    bool can_dock = true;
    if (cnt->tab_bar) {
        ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
        for (u32 t = 0; t < tab_bar->size; ++t) {
            if (tab_bar->items[t].data == ctx->focus_root) {
                can_dock = false;
            }
        }
    }

    // Do docking overlay (if enabled)
    if (can_dock && ~cnt->opt & UI_OPT_NODOCK && ctx->focus_root && ctx->focus_root != cnt &&
        ui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) &&  // This is the incorrect part - need to check if this container isn't being overlapped by another
        ctx->mouse_down == UI_MOUSE_LEFT && ~cnt->opt & UI_OPT_NOHOVER && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        ui_split_t* focus_split = ui_get_root_split(ctx, ctx->focus_root);
        ui_split_t* cnt_split = ui_get_root_split(ctx, cnt);

        // NOTE: this is incorrect...
        if ((!focus_split && !cnt_split) || ((focus_split || cnt_split) && (focus_split != cnt_split))) {
            // Set dockable root container
            ctx->dockable_root = ctx->dockable_root && cnt->zindex > ctx->dockable_root->zindex ? cnt : ctx->dockable_root ? ctx->dockable_root : cnt;
        }
    }

    // Set current frame
    cnt->frame = ctx->frame;

    // Pop root container
    ui_root_container_end(ctx);
}

void ui_popup_open(ui_context_t* ctx, const char* name) {
    ui_container_t* cnt = ui_get_container(ctx, name);

    // Set as hover root so popup isn't closed in window_begin_ex()
    ctx->hover_root = ctx->next_hover_root = cnt;

    // position at mouse cursor, open and bring-to-front
    cnt->rect = ui_rect(ctx->mouse_pos.x, ctx->mouse_pos.y, 100, 100);
    cnt->open = 1;
    ui_bring_to_front(ctx, cnt);
}

i32 ui_popup_begin_ex(ui_context_t* ctx, const char* name, ui_rect_t r, const ui_selector_desc_t* desc, u64 opt) {
    opt |= (UI_OPT_POPUP | UI_OPT_NODOCK | UI_OPT_CLOSED);
    return ui_window_begin_ex(ctx, name, r, NULL, NULL, opt);
}

void ui_popup_end(ui_context_t* ctx) { ui_window_end(ctx); }

void ui_panel_begin_ex(ui_context_t* ctx, const char* name, const ui_selector_desc_t* desc, u64 opt) {
    ui_container_t* cnt;
    const i32 elementid = UI_ELEMENT_PANEL;
    char id_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, name, id_tag, sizeof(id_tag), opt);

    // if (id_tag) ui_push_id(ctx, id_tag, strlen(id_tag));
    // else ui_push_id(ctx, name, strlen(name));
    ui_push_id(ctx, name, strlen(name));
    cnt = ui_get_container_ex(ctx, ctx->last_id, opt);
    cnt->rect = ui_layout_next(ctx);

    const ui_id id = ui_get_id(ctx, name, strlen(name));

    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    if (~opt & UI_OPT_NOFRAME) {
        ui_draw_frame(ctx, cnt->rect, &style);
    }

    // Need a way to push/pop syles temp styles
    ui_stack_push(ctx->container_stack, cnt);
    ui_push_container_body(ctx, cnt, cnt->rect, desc, opt);
    ui_push_clip_rect(ctx, cnt->body);
}

void ui_panel_end(ui_context_t* ctx) {
    ui_pop_clip_rect(ctx);
    ui_pop_container(ctx);
}

static u8 uint8_slider(ui_context_t* ctx, unsigned char* value, int low, int high, const ui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    ui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = ui_slider_ex(ctx, &tmp, (ui_real)low, (ui_real)high, 0, "%.0f", desc, opt);
    *value = (u8)tmp;
    ui_pop_id(ctx);
    return res;
}

static i32 int32_slider(ui_context_t* ctx, i32* value, i32 low, i32 high, const ui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    ui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = ui_slider_ex(ctx, &tmp, (ui_real)low, (ui_real)high, 0, "%.0f", desc, opt);
    *value = (i32)tmp;
    ui_pop_id(ctx);
    return res;
}

static i16 int16_slider(ui_context_t* ctx, i16* value, i32 low, i32 high, const ui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    ui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = ui_slider_ex(ctx, &tmp, (ui_real)low, (ui_real)high, 0, "%.0f", desc, opt);
    *value = (i16)tmp;
    ui_pop_id(ctx);
    return res;
}

//=== Demos ===//

i32 ui_style_editor(ui_context_t* ctx, ui_style_sheet_t* style_sheet, ui_rect_t rect, bool* open) {
    if (!style_sheet) {
        style_sheet = &ui_default_style_sheet;
    }

    static struct {
        const char* label;
        i32 idx;
    } elements[] = {{"container", UI_ELEMENT_CONTAINER}, {"button", UI_ELEMENT_BUTTON}, {"panel", UI_ELEMENT_PANEL},
                    {"input", UI_ELEMENT_INPUT},         {"label", UI_ELEMENT_LABEL},   {"text", UI_ELEMENT_TEXT},
                    {"scroll", UI_ELEMENT_SCROLL},       {"image", UI_ELEMENT_IMAGE},   {NULL}};

    static const char* states[] = {"default", "hover", "focus"};

    static struct {
        const char* label;
        i32 idx;
    } colors[] = {{"background", UI_COLOR_BACKGROUND},
                  {"border", UI_COLOR_BORDER},
                  {"shadow", UI_COLOR_SHADOW},
                  {"content", UI_COLOR_CONTENT},
                  {"content_shadow", UI_COLOR_CONTENT_SHADOW},
                  {"content_background", UI_COLOR_CONTENT_BACKGROUND},
                  {"content_border", UI_COLOR_CONTENT_BORDER},
                  {NULL}};

    if (ui_window_begin_ex(ctx, "Style_Editor", rect, open, NULL, 0x00)) {
        for (u32 i = 0; elements[i].label; ++i) {
            i32 idx = elements[i].idx;

            if (ui_treenode_begin_ex(ctx, elements[i].label, NULL, 0x00)) {
                for (u32 j = 0; j < UI_ELEMENT_STATE_COUNT; ++j) {
                    ui_push_id(ctx, &j, sizeof(j));
                    ui_style_t* s = &style_sheet->styles[idx][j];
                    if (ui_treenode_begin_ex(ctx, states[j], NULL, 0x00)) {
                        ui_style_t* save = ui_push_style(ctx, &ctx->style_sheet->styles[UI_ELEMENT_PANEL][0x00]);
                        i32 row[] = {-1};
                        ui_layout_row(ctx, 1, row, 300);
                        ui_panel_begin(ctx, states[j]);
                        {
                            ui_layout_t* l = ui_get_layout(ctx);
                            ui_rect_t* r = &l->body;

                            const i32 ls = 80;

                            // size
                            i32 w = (i32)((l->body.w - ls) * 0.35f);
                            {
                                i32 row[] = {ls, w, w};
                                ui_layout_row(ctx, 3, row, 0);
                            }

                            ui_label(ctx, "size:");
                            ui_slider(ctx, &s->size[0], 0.f, 500.f);
                            ui_slider(ctx, &s->size[1], 0.f, 500.f);

                            w = (i32)((l->body.w - ls) * 0.2f);

                            {
                                i32 row[] = {ls, w, w, w, w};
                                ui_layout_row(ctx, 5, row, 0);
                            }

                            ui_label(ctx, "border_width:");
                            int16_slider(ctx, &s->border_width[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[3], 0, 100, NULL, 0x00);

                            ui_label(ctx, "border_radius:");
                            int16_slider(ctx, &s->border_radius[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[3], 0, 100, NULL, 0x00);

                            // padding/margin
                            ui_label(ctx, "padding:");
                            int32_slider(ctx, &s->padding[0], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[1], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[2], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[3], 0, 100, NULL, 0x00);

                            ui_label(ctx, "margin:");
                            int16_slider(ctx, &s->margin[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[3], 0, 100, NULL, 0x00);

                            // Colors
                            int sw = (i32)(l->body.w * 0.14);
                            {
                                i32 row[] = {80, sw, sw, sw, sw, -1};
                                ui_layout_row(ctx, 6, row, 0);
                            }

                            for (u32 c = 0; colors[c].label; ++c) {
                                ui_label(ctx, colors[c].label);
                                uint8_slider(ctx, &s->colors[c].r, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].g, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].b, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].a, 0, 255, NULL, 0x00);
                                ui_draw_rect(ctx, ui_layout_next(ctx), s->colors[c]);
                            }
                        }
                        ui_panel_end(ctx);
                        ui_pop_style(ctx, save);

                        ui_treenode_end(ctx);
                    }
                    ui_pop_id(ctx);
                }
                ui_treenode_end(ctx);
            }
        }
        ui_window_end(ctx);
    }

    return 0x01;
}

i32 ui_demo_window(ui_context_t* ctx, ui_rect_t rect, bool* open) {

    if (ui_window_begin_ex(ctx, "Demo_Window", rect, open, NULL, 0x00)) {
        ui_container_t* win = ui_get_current_container(ctx);

        if (ui_treenode_begin(ctx, "Help")) {
            {
                i32 row[] = {-10};
                ui_layout_row(ctx, 1, row, 170);
            }

            ui_panel_begin(ctx, "#!window_info");
            {
                {
                    i32 row[] = {-1};
                    ui_layout_row(ctx, 1, row, 0);
                }
                ui_label(ctx, "ABOUT THIS DEMO:");
                ui_text(ctx, "  - Sections below are demonstrating many aspects of the util.");
                ui_text(ctx, " 测试中文，你好世界");
            }
            ui_panel_end(ctx);
            ui_treenode_end(ctx);
        }

        if (ui_treenode_begin(ctx, "Window Info")) {
            {
                i32 row[] = {-10};
                ui_layout_row(ctx, 1, row, 170);
            }
            ui_panel_begin(ctx, "#!window_info");
            {
                char buf[64];
                {
                    i32 row[] = {65, -1};
                    ui_layout_row(ctx, 2, row, 0);
                }

                ui_label(ctx, "Position:");
                neko_snprintf(buf, 64, "%.2f, %.2f", win->rect.x, win->rect.y);
                ui_label(ctx, buf);

                ui_label(ctx, "Size:");
                neko_snprintf(buf, 64, "%.2f, %.2f", win->rect.w, win->rect.h);
                ui_label(ctx, buf);

                ui_label(ctx, "Title:");
                ui_label(ctx, win->name);

                ui_label(ctx, "ID:");
                neko_snprintf(buf, 64, "%zu", win->id);
                ui_label(ctx, buf);

                ui_label(ctx, "Open:");
                neko_snprintf(buf, 64, "%s", win->open ? "true" : "close");
                ui_label(ctx, buf);
            }
            ui_panel_end(ctx);

            ui_treenode_end(ctx);
        }

        if (ui_treenode_begin(ctx, "Context State")) {
            {
                i32 row[] = {-10};
                ui_layout_row(ctx, 1, row, 170);
            }
            ui_panel_begin(ctx, "#!context_state");
            {
                char buf[64];
                {
                    i32 row[] = {80, -1};
                    ui_layout_row(ctx, 2, row, 0);
                }

                ui_label(ctx, "Hovered:");
                neko_snprintf(buf, 64, "%s", ctx->hover_root ? ctx->hover_root->name : "NULL");
                ui_label(ctx, buf);

                ui_label(ctx, "Focused:");
                neko_snprintf(buf, 64, "%s", ctx->focus_root ? ctx->focus_root->name : "NULL");
                ui_label(ctx, buf);

                ui_label(ctx, "Active:");
                neko_snprintf(buf, 64, "%s", ctx->active_root ? ctx->active_root->name : "NULL");
                ui_label(ctx, buf);

                ui_label(ctx, "Lock Focus:");
                neko_snprintf(buf, 64, "%zu", ctx->lock_focus);
                ui_label(ctx, buf);
            }
            ui_panel_end(ctx);

            ui_treenode_end(ctx);
        }

        if (ui_treenode_begin(ctx, "Widgets")) {
            {
                i32 row[] = {-10};
                ui_layout_row(ctx, 1, row, 170);
            }
            ui_panel_begin(ctx, "#!widgets");
            {
                {
                    i32 row[] = {150, 50};
                    ui_layout_row(ctx, 2, row, 0);
                }
                ui_layout_column_begin(ctx);
                {
                    {
                        i32 row[] = {0};
                        ui_layout_row(ctx, 1, row, 0);
                    }
                    ui_button(ctx, "Button");

                    // Label
                    ui_label(ctx, "Label");

                    // Text
                    {
                        i32 row[] = {150};
                        ui_layout_row(ctx, 1, row, 0);
                    }
                    ui_text(ctx, "This is some text");

                    static char buf[64] = {0};
                    ui_textbox(ctx, buf, 64);
                }
                ui_layout_column_end(ctx);

                ui_layout_column_begin(ctx);
                {
                    ui_label(ctx, "(?)");
                    if (ctx->hover == ctx->last_id) neko_println("HOVERED");
                }
                ui_layout_column_end(ctx);
            }
            ui_panel_end(ctx);
            ui_treenode_end(ctx);
        }

        ui_window_end(ctx);
    }
    return 0x01;
}