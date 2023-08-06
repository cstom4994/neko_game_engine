#ifndef NEKO_LEXER_H
#define NEKO_LEXER_H

#include "engine/common/neko_types.h"
#include "engine/common/neko_util.h"

#define __neko_parser_debug_enabled 0

/*==============================
// Neko_Token
==============================*/

typedef struct {
    const_str text;
    const_str type;  // Would prefer this as an enum
    u32 len;
} neko_token;

neko_static_inline neko_token neko_token_invalid_token() {
    neko_token t;
    t.text = "";
    t.type = "_invalid_";
    return t;
}

neko_static_inline b8 neko_token_compare_type(neko_token t, const_str match_type) { return (neko_string_compare_equal(t.type, match_type)); }

neko_static_inline b8 neko_token_compare_text(neko_token t, const_str match_text) { return (neko_string_compare_equal_n(t.text, match_text, t.len)); }

neko_static_inline void neko_token_print_text(neko_token t) { neko_printf("%.*s\n", t.len, t.text); }

neko_static_inline void neko_token_debug_print(neko_token t) { neko_printf("%s: %.*s\n", t.type, t.len, t.text); }

neko_static_inline b8 neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }

neko_static_inline b8 neko_token_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_token_is_end_of_line(c)); }

neko_static_inline b8 neko_token_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }

neko_static_inline b8 neko_token_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

/*==============================
// Neko_Lexer
==============================*/

typedef struct neko_lexer {
    const_str at;
    const_str contents;
    neko_token current_token;

    neko_token (*next_token)(struct neko_lexer*);
} neko_lexer;

typedef struct {
    neko_lexer _base;
} neko_lexer_c;

neko_static_inline b8 neko_lexer_can_lex(neko_lexer* lex) {
    char c = *lex->at;
    return (lex->at != NULL && *(lex->at) != '\0');
}

// Assumes that container and ignore list are set
neko_static_inline void neko_lexer_set_contents(neko_lexer* lex, const_str contents) {
    lex->at = contents;
    lex->current_token = neko_token_invalid_token();
}

neko_static_inline void neko_lexer_advance_position(neko_lexer* lex, usize advance) { lex->at += advance; }

neko_static_inline neko_token neko_lexer_next_token(neko_lexer* lex) {
    neko_token t = lex->next_token(lex);
    lex->current_token = t;
    return t;
}

neko_static_inline void neko_lexer_eat_whitespace(neko_lexer* lex) {
    for (;;) {
        if (neko_token_char_is_white_space(lex->at[0])) {
            lex->at++;
        }

        // Single line comment
        else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '/')) {
            lex->at += 2;
            while (lex->at[0] && !neko_token_is_end_of_line(lex->at[0])) {
                lex->at++;
            }
        }

        // Multi line comment
        else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '*')) {
            lex->at += 2;
            while (lex->at[0] && lex->at[1] && !(lex->at[0] == '*' && lex->at[1] == '/')) {
                lex->at++;
            }
            if (lex->at[0] == '*') {
                lex->at++;
            }
        }

        else {
            break;
        }
    }
}

// Explicit tokenizing (not using regex)
neko_static_inline neko_token neko_lexer_c_next_token(neko_lexer* lex) {
    // Eat all white space
    // neko_lexer_eat_whitespace(lex);

    neko_token t = neko_token_invalid_token();
    t.text = lex->at;
    t.len = 1;

    if (neko_lexer_can_lex(lex)) {
        char c = lex->at[0];
        switch (c) {
            case '(': {
                t.type = "lparen";
                lex->at++;
            } break;
            case ')': {
                t.type = "rparen";
                lex->at++;
            } break;
            case '<': {
                t.type = "lthan";
                lex->at++;
            } break;
            case '>': {
                t.type = "gthan";
                lex->at++;
            } break;
            case ';': {
                t.type = "semi_colon";
                lex->at++;
            } break;
            case ':': {
                t.type = "colon";
                lex->at++;
            } break;
            case ',': {
                t.type = "comma";
                lex->at++;
            } break;
            case '=': {
                t.type = "equal";
                lex->at++;
            } break;
            case '!': {
                t.type = "not";
                lex->at++;
            } break;
            case '#': {
                t.type = "hash";
                lex->at++;
            } break;
            case '|': {
                t.type = "pipe";
                lex->at++;
            } break;
            case '&': {
                t.type = "ampersand";
                lex->at++;
            } break;
            case '{': {
                t.type = "lbrace";
                lex->at++;
            } break;
            case '}': {
                t.type = "rbrace";
                lex->at++;
            } break;
            case '[': {
                t.type = "lbracket";
                lex->at++;
            } break;
            case ']': {
                t.type = "rbracket";
                lex->at++;
            } break;
            case '-': {
                t.type = "minus";
                lex->at++;
            } break;
            case '+': {
                t.type = "plus";
                lex->at++;
            } break;
            case '*': {
                t.type = "asterisk";
                lex->at++;
            } break;
            case '\\': {
                t.type = "bslash";
                lex->at++;
            } break;
            case '?': {
                t.type = "qmark";
                lex->at++;
            } break;
            case ' ': {
                t.type = "space";
                lex->at++;
            } break;
            case '\n': {
                t.type = "newline";
                lex->at++;
            } break;
            case '\r': {
                t.type = "newline";
                lex->at++;
            } break;
            case '\t': {
                t.type = "tab";
                lex->at++;
            } break;

            case '/': {
                // Single line comment
                if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '/')) {
                    lex->at += 2;
                    while (lex->at[0] && !neko_token_is_end_of_line(lex->at[0])) {
                        lex->at++;
                    }
                    t.len = lex->at - t.text;
                    t.type = "single_line_comment";
                }

                // Multi line comment
                else if ((lex->at[0] == '/') && (lex->at[1]) && (lex->at[1] == '*')) {
                    lex->at += 2;
                    while (lex->at[0] && lex->at[1] && !(lex->at[0] == '*' && lex->at[1] == '/')) {
                        lex->at++;
                    }
                    if (lex->at[0] == '*') {
                        lex->at++;
                    }
                    t.len = lex->at - t.text;
                    t.type = "multi_line_comment";
                }

            } break;

            case '"': {
                // Move forward after finding first quotation
                lex->at++;

                while (lex->at[0] && lex->at[0] != '"') {
                    if (lex->at[0] == '\\' && lex->at[1]) {
                        lex->at++;
                    }

                    lex->at++;
                }

                // Move past the quotation
                lex->at++;

                t.len = lex->at - t.text;
                t.type = "string";
            } break;

            default: {
                if ((neko_token_char_is_alpha(c) || c == '_') && c != '-') {
                    while (neko_token_char_is_alpha(lex->at[0]) || neko_token_char_is_numeric(lex->at[0]) || lex->at[0] == '_') {
                        lex->at++;
                    }

                    t.len = lex->at - t.text;
                    t.type = "identifier";
                } else if (neko_token_char_is_numeric(c) || c == '-') {
                    u32 num_decimals = 0;
                    while (neko_token_char_is_numeric(lex->at[0]) || (lex->at[0] == '.' && num_decimals == 0) || lex->at[0] == 'f') {
                        // Grab decimal
                        num_decimals = lex->at[0] == '.' ? num_decimals++ : num_decimals;

                        // Increment
                        lex->at++;
                    }

                    t.len = lex->at - t.text;
                    t.type = "number";
                } else {
                    t.type = "unknown";
                    lex->at++;
                }

            } break;
        }
    }

    return t;
}

neko_static_inline neko_token neko_lexer_current_token(neko_lexer* lex) { return lex->current_token; }

neko_static_inline b8 neko_lexer_current_token_type_eq(neko_lexer* lex, const_str match_type) { return (neko_string_compare_equal(neko_lexer_current_token(lex).type, match_type)); }

neko_static_inline neko_token neko_lexer_peek_next_token(neko_lexer* lex) {
    // Store the at
    const_str at = lex->at;
    neko_token cur_t = neko_lexer_current_token(lex);
    neko_token next_t = lex->next_token(lex);
    lex->current_token = cur_t;
    lex->at = at;
    return next_t;
}

// Checks to see if the token type of the next valid token matches the match_type argument
// Will restore pointer of lex if not a match
neko_static_inline b8 neko_lexer_require_token_text(neko_lexer* lex, const_str match_text) {
    // Store current position
    const_str at = lex->at;

    neko_token t = neko_lexer_next_token(lex);

#if __neko_parser_debug_enabled
    neko_printf("require_expect_text: %s, found: ", match_text);
    neko_token_debug_print(t);
#endif

    if (neko_string_compare_equal_n(t.text, match_text, t.len)) {
        return true;
    }

    // Hit invalid token, print error
    neko_printf("\nLex Error: Unexpected token text: %.*s, Expected: %s\n", t.len, t.text, match_text);

    // Restore position
    lex->at = at;

    return false;
}

// Checks to see if the token type of the next valid token matches the match_type argument
// Will restore pointer of lex if not a match
neko_static_inline b8 neko_lexer_require_token_type(neko_lexer* lex, const_str match_type) {
    // Store current position
    const_str at = lex->at;

    neko_token t = neko_lexer_next_token(lex);

#if __neko_parser_debug_enabled
    neko_printf("require_expect_type: %s, found: ", match_type);
    neko_token_debug_print(t);
#endif

    if (neko_string_compare_equal(t.type, match_type)) {
        return true;
    }

    // Hit invalid token, print error
    neko_printf("\nLex Error: Unexpected token type: %.s, Expected: %s\n\n", t.type, match_type);

    // Restore position
    lex->at = at;

    return false;
}

neko_static_inline b8 neko_lexer_optional_token_type(neko_lexer* lex, const_str match_type) {
    const_str at = lex->at;

    neko_token t = neko_lexer_next_token(lex);

#if __neko_parser_debug_enabled
    neko_printf("optional_expected: %s, found: ", match_type);
    neko_token_debug_print(t);
#endif

    if (neko_token_compare_type(t, match_type)) {
        return true;
    }

    // Restore previous position
    lex->at = at;

    return false;
}

neko_static_inline b8 neko_lexer_optional_token_text(neko_lexer* lex, const_str match_text) {
    const_str at = lex->at;

    neko_token t = neko_lexer_next_token(lex);

#if __neko_parser_debug_enabled
    neko_printf("optional_expect: %s, found: ", match_text);
    neko_token_debug_print(t);
#endif

    if (neko_token_compare_text(t, match_text)) {
        return true;
    }

    // Restore previous position
    lex->at = at;

    return false;
}

// Advances position until lexer can no longer lex or token of type is found
// Returns true if found, false if end of stream is found
neko_static_inline b8 neko_lexer_find_token_type(neko_lexer* lex, const_str match_type) {
    neko_token t = neko_lexer_current_token(lex);
    while (neko_lexer_can_lex(lex)) {
        if (neko_token_compare_type(t, match_type)) {
            return true;
        }
        t = neko_lexer_next_token(lex);
    }

    return false;
}

neko_static_inline neko_token neko_lexer_advance_before_next_token_type_occurence(neko_lexer* lex, const_str match_type) {
    neko_token t = neko_lexer_current_token(lex);
    neko_token peek_t = neko_lexer_peek_next_token(lex);

    // Continue right up the token before the required type
    while (!neko_token_compare_type(peek_t, match_type)) {
        t = neko_lexer_next_token(lex);
        peek_t = neko_lexer_peek_next_token(lex);
    }

    return t;
}

neko_static_inline neko_lexer_c neko_lexer_c_ctor(const_str contents) {
    neko_lexer_c lex;
    lex._base.at = contents;
    lex._base.contents = contents;
    lex._base.current_token = neko_token_invalid_token();
    lex._base.next_token = &neko_lexer_c_next_token;

    return lex;
}

#endif  // NEKO_LEXER_H
