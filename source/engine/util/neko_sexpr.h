#ifndef NEKO_SEXPR_H
#define NEKO_SEXPR_H

// neko_s_expression

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/neko.h"

enum neko_sexpr_types {
    NEKO_SEXPR_TYPE_NONE,
    NEKO_SEXPR_TYPE_INT,
    NEKO_SEXPR_TYPE_FLOAT,
    NEKO_SEXPR_TYPE_STRING,
    NEKO_SEXPR_TYPE_ARRAY,  // 数组包括原始数组和具名节点
};

typedef struct neko_sexpr_data_entry_s {
    enum neko_sexpr_types type;
    union {
        s32 i;
        f32 f;
        char* str;
        struct neko_sexpr_data_entry_s* node;
    };

    s32 is_str_allocated;

    //////////////////
    // NEKO_SEXPR_TYPE_ARRAY stuff
    s32 node_len;
    s32 is_node_allocated;
    s32 len_is_fixed;

    // for pretty serialisation
    s32 new_line_at_start;
    s32 new_line_at_end;
    s32 new_line_at_end_of_subsequent_elements;
} neko_sexpr_data_entry_t;

typedef neko_sexpr_data_entry_t neko_snode_t;

#define neko_sexpr_i(__i) \
    { .type = NEKO_SEXPR_TYPE_INT, .i = __i }

#define neko_sexpr_f(__f) \
    { .type = NEKO_SEXPR_TYPE_FLOAT, .f = __f }

#define neko_sexpr_s(__s) \
    { .type = NEKO_SEXPR_TYPE_STRING, .str = __s }

#define neko_sexpr_a(__count, ...)                                                                   \
    {                                                                                                \
        .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = __count, .node = (neko_snode_t[]) { __VA_ARGS__ } \
    }

#define neko_sexpr_new_a(__count, ...)                                                                                                            \
    {                                                                                                                                             \
        .new_line_at_end_of_subsequent_elements = 1, .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = __count, .node = (neko_snode_t[]) { __VA_ARGS__ } \
    }

#define neko_sexpr_tagged_i(__name, __i)                                                                                   \
    {                                                                                                                      \
        .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = 2, .node = (neko_snode_t[]) { neko_sexpr_s(__name), neko_sexpr_i(__i) } \
    }

#define neko_sexpr_tagged_f(__name, __f)                                                                                   \
    {                                                                                                                      \
        .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = 2, .node = (neko_snode_t[]) { neko_sexpr_s(__name), neko_sexpr_f(__f) } \
    }

#define neko_sexpr_tagged_s(__name, __s)                                                                                   \
    {                                                                                                                      \
        .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = 2, .node = (neko_snode_t[]) { neko_sexpr_s(__name), neko_sexpr_s(__s) } \
    }

#define neko_sexpr_tagged_a(__name, __count, ...) neko_sexpr_a(__count + 1, neko_sexpr_s(__name), __VA_ARGS__)

#define neko_sexpr_tagged_new_a(__name, __count, ...) neko_sexpr_new_a(__count + 1, neko_sexpr_s(__name), __VA_ARGS__)

extern s32 neko_sexpr_node_fits_format(neko_snode_t* node, neko_snode_t* fmt);

extern neko_snode_t* neko_sexpr_node_get_value(neko_snode_t* node, s32* len);  // 返回 node->node + 1 如果 node->node 是字符串 (判定为具名节点)
extern neko_snode_t* neko_sexpr_node_get_tagged(neko_snode_t* node, const_str tag);
extern neko_snode_t* neko_sexpr_node_get_index(neko_snode_t* node, s32 index);

extern s32 neko_sexpr_write_to_file(char* filename, const neko_snode_t node);

extern neko_snode_t neko_sexpr_parse_file(char* filename);
extern neko_snode_t neko_sexpr_parse_memory(char* mem, s32 sz);

extern void neko_sexpr_free_node(neko_snode_t* node);
extern neko_snode_t neko_sexpr_dup_node(neko_snode_t* node, s32 free_old_node);

#ifdef NEKO_SEXPR_IMPL

s32 neko_sexpr_node_fits_format(neko_snode_t* node, neko_snode_t* fmt) {
    if (!node) return 0;

    if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        for (s32 i = node->node_len && node->node->type == NEKO_SEXPR_TYPE_STRING; i < node->node_len; i++) {
            if (node->node[i].type == NEKO_SEXPR_TYPE_ARRAY && node->node[i].node_len && node->node[i].node->type == NEKO_SEXPR_TYPE_STRING) {
                if (!neko_sexpr_node_get_tagged(fmt, node->node[i].node->str)) return 0;
            } else {
                neko_snode_t* res = neko_sexpr_node_get_index(fmt, i);
                if (!res) return 0;
                if (node->node[i].type != NEKO_SEXPR_TYPE_ARRAY && res->type == NEKO_SEXPR_TYPE_ARRAY && res->node_len == 2) {
                    if (res->node[1].type != node->node[i].type) return 0;
                } else if (node->node[i].type != res->type) {
                    return 0;
                }
            }
        }
        return 1;
    }
    return node->type == fmt->type;
}

neko_snode_t* neko_sexpr_node_get_value(neko_snode_t* node, s32* len) {
    if (!node) return NULL;
    if (len) *len = 1;
    if (node->type != NEKO_SEXPR_TYPE_ARRAY) return node;

    if (!node->node || node->node_len < 2 || node->node->type != NEKO_SEXPR_TYPE_STRING) return node->node;
    if (len) *len = node->node_len - 1;
    return node->node + 1;
}

neko_snode_t* neko_sexpr_node_get_tagged(neko_snode_t* node, const_str tag) {
    if (!node || node->type != NEKO_SEXPR_TYPE_ARRAY || !node->node) return NULL;

    for (s32 i = 0; i < node->node_len; i++)
        if (node->node[i].type == NEKO_SEXPR_TYPE_ARRAY && node->node[i].node_len && node->node[i].node->type == NEKO_SEXPR_TYPE_STRING) {
            if (strcmp(tag, node->node[i].node->str) == 0) return node->node + i;
        }
    return NULL;
}

neko_snode_t* neko_sexpr_node_get_index(neko_snode_t* node, s32 index) {
    if (!node || node->type != NEKO_SEXPR_TYPE_ARRAY || !node->node || index < 0 || index >= node->node_len) return NULL;
    return node->node + index;
}

static void neko_sexpr_write_nde(FILE* fd, const neko_snode_t* node, s32 root, s32 indents, s32 new_line_start, s32 first) {
    if (!node) {
        neko_log_warning("neko S-expression error: node was NULL");
        return;
    }

    if ((new_line_start && (!first || root)) || node->new_line_at_start) {
        fprintf(fd, "\n");
        for (s32 i = 0; i < indents; i++) fprintf(fd, "    ");
    } else if (!root && !first) {
        fprintf(fd, " ");
    }

    if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        fprintf(fd, "(");
    } else if (root) {
        neko_log_warning("neko S-expression error: a root node was not an array");
        return;
    }

    if (node->type == NEKO_SEXPR_TYPE_NONE) {
        neko_log_warning("neko S-expression error: skipping writing an uninitialized member");
    } else if (node->type == NEKO_SEXPR_TYPE_INT) {
        fprintf(fd, "%d", node->i);
    } else if (node->type == NEKO_SEXPR_TYPE_FLOAT) {
        fprintf(fd, "%f", node->f);
    } else if (node->type == NEKO_SEXPR_TYPE_STRING) {
        s32 res = 0;
        for (s32 i = 0; i < strlen(node->str); i++) {
            if (i == 0 && memchr("1234567890.-", node->str[i], sizeof("1234567890.-"))) {
                res = 1;
                break;
            }
            if (memchr("\n '\t\v()\r", node->str[i], sizeof("\n '\t\v()\r"))) {
                res = 1;
                break;
            }
        }
        if (strchr(node->str, '`')) {
            neko_log_warning("neko S-expression error: string '%s' contains a `, nisse does not support this", node->str);
        } else {
            if (res)
                fprintf(fd, "`%s`", node->str);
            else
                fprintf(fd, "%s", node->str);
        }
    } else if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        for (s32 i = 0; i < node->node_len; i++) neko_sexpr_write_nde(fd, node->node + i, 0, indents + 1, node->new_line_at_end_of_subsequent_elements, i == 0);
    }

    if (node->new_line_at_end_of_subsequent_elements) {
        fprintf(fd, "\n");
        for (s32 i = 0; i < indents; i++) fprintf(fd, "    ");
    }
    if (node->type == NEKO_SEXPR_TYPE_ARRAY) fprintf(fd, ")");
}

s32 neko_sexpr_write_to_file(char* filename, const neko_snode_t node) {
    if (node.type != NEKO_SEXPR_TYPE_ARRAY) return 0;

    FILE* fd = fopen(filename, "wb");
    if (!fd) {
        neko_log_warning("neko S-expression error: unable to open file");
        return 0;
    }

    for (s32 i = 0; i < node.node_len; i++) neko_sexpr_write_nde(fd, node.node + i, 1, 0, i > 0, 1);

    fclose(fd);
    return 1;
}

neko_snode_t neko_sexpr_parse_file(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        neko_log_warning("neko S-expression error: unable to open file");
        return (neko_snode_t){0};
    }

    fseek(file, 0L, SEEK_END);
    s32 readsize = ftell(file);
    rewind(file);

    char* buffer = neko_safe_malloc(readsize);
    if (!buffer) return (neko_snode_t){0};

    fread(buffer, 1, readsize, file);
    fclose(file);

    return neko_sexpr_parse_memory(buffer, readsize);
}

static s32 neko_sexpr_seek_whitespace(char* mem, s32 index, s32 sz) {
    while (index < sz && !memchr("\n '\t()\v\r", mem[index], sizeof("\n '\t()\v\r"))) index++;
    return index;
}

static s32 neko_sexpr_parse_memory_array(char* mem, s32 sz, neko_snode_t* node) {
    s32 index = 0;
    if (index >= sz) return index;
    if (mem[index] == '(') index++;
    if (index >= sz) return index;

    node->type = NEKO_SEXPR_TYPE_ARRAY;
    node->is_node_allocated = 1;
    s32 new_line_on_all_elements = 1;

    while (index < sz) {
        s32 last_index = index;

        while (index < sz && memchr("\n \t\v\r", mem[index], sizeof("\n \t\v\r"))) index++;

        if (mem[index] == ')') {
            if (node->node_len <= 1) new_line_on_all_elements = 0;
            break;
        }

        s32 i = ++node->node_len - 1;
        node->node = neko_safe_realloc(node->node, sizeof(*node->node) * node->node_len);
        node->node[i] = (neko_snode_t){0};

        if (index != last_index && memchr(mem + last_index, '\n', index - last_index)) node->node[i].new_line_at_start = 1;

        if (memchr("1234567890.-", mem[index], sizeof("1234567890.-"))) {
            s32 number_end = neko_sexpr_seek_whitespace(mem, index, sz);

            char tmp = mem[number_end];
            mem[number_end] = 0;
            if (memchr(mem + index, '.', number_end - index)) {  // 浮点型
                node->node[i].f = atof(mem + index);
                node->node[i].type = NEKO_SEXPR_TYPE_FLOAT;
            } else /* s32 */ {
                node->node[i].i = atoi(mem + index);
                node->node[i].type = NEKO_SEXPR_TYPE_INT;
            }
            mem[number_end] = tmp;
            index = number_end;
        } else if (mem[index] == '(') {  // 数组
            index += neko_sexpr_parse_memory_array(mem + index, sz - index, node->node + i);
        } else if (mem[index] == '`') {  // 字符串
            char* end;
            s32 times = 0;
            do {
                times++;
                end = (sz - index - 1 < 0) ? NULL : memchr(mem + index + times, '`', sz - index - times);
            } while (end && end[-1] == '\\');
            if (!end) {
                neko_log_warning("neko S-expression error: unable to find closing quote");
                return sz;
            }
            size_t strsz = (end) - (mem + index + 1);

            node->node[i].str = neko_safe_malloc(strsz + 1);
            node->node[i].is_str_allocated = 1;

            memcpy(node->node[i].str, mem + index + 1, strsz);
            node->node[i].str[strsz] = 0;

            index += strsz + 2;
            node->node[i].type = NEKO_SEXPR_TYPE_STRING;
        } else {  // 处理不带 " " 的字符串
            s32 end = neko_sexpr_seek_whitespace(mem, index, sz);

            size_t strsz = (end - index);

            node->node[i].str = neko_safe_malloc(strsz + 1);
            node->node[i].is_str_allocated = 1;

            memcpy(node->node[i].str, mem + index, strsz);
            node->node[i].str[strsz] = 0;

            index += strsz;
            node->node[i].type = NEKO_SEXPR_TYPE_STRING;
        }

        if (mem[index] == '\n' && node->node[i].type == NEKO_SEXPR_TYPE_ARRAY && !node->node[i].new_line_at_start) node->node[i].new_line_at_end = 1;

        if ((!node->node[i].new_line_at_start || node->node[i].new_line_at_end) && i != 0) new_line_on_all_elements = 0;
    }
    node->new_line_at_end_of_subsequent_elements = new_line_on_all_elements;

    if (index >= sz) {
        neko_log_warning("neko S-expression error: unable to find closing parenthesis");
        return sz;
    }
    return index + 1;
}

neko_snode_t neko_sexpr_parse_memory(char* mem, s32 sz) {
    s32 index = 0;
    neko_snode_t node = {.type = NEKO_SEXPR_TYPE_ARRAY};
    node.node_len = 0;
    while (index < sz) {
        char* new_pos = memchr(mem + index, '(', sz - index);
        if (!new_pos) return node;

        index += (s32)(new_pos - (mem + index));

        node.node_len += 1;
        node.node = neko_safe_realloc(node.node, sizeof(*node.node) * node.node_len);
        node.node[node.node_len - 1] = (neko_snode_t){0};

        index += neko_sexpr_parse_memory_array(mem + index, sz - index, node.node + (node.node_len - 1));
    }
    if (node.node_len) node.is_node_allocated = 1;
    return node;
}

void neko_sexpr_free_node(neko_snode_t* node) {

    if (node->type == NEKO_SEXPR_TYPE_STRING && node->is_str_allocated) {
        neko_safe_free(node->str);
    } else if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        for (s32 i = 0; i < node->node_len; i++) neko_sexpr_free_node(node->node + i);
        if (node->is_node_allocated) neko_safe_free(node->node);
    }
}

neko_snode_t neko_sexpr_dup_node(neko_snode_t* node, s32 free_old_node) {
    if (node->type == NEKO_SEXPR_TYPE_STRING) {
        char* str = strdup(node->str);
        if (free_old_node && node->is_str_allocated) neko_safe_free(node->str);
        node->is_str_allocated = 1;
        node->str = str;
    } else if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        neko_snode_t* n = neko_safe_malloc(node->node_len * sizeof(*n));
        memcpy(n, node->node, node->node_len * sizeof(*n));
        if (free_old_node && node->is_node_allocated) neko_safe_free(node->node);
        node->node = n;
        node->is_node_allocated = 1;
        for (s32 i = 0; i < node->node_len; i++) neko_sexpr_dup_node(node->node + i, free_old_node);
    }
    return *node;
}

#endif

#endif
