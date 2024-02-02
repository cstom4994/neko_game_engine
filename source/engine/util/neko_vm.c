#include "neko_vm.h"

#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ns_free(ptr) free(ptr)
#define ns_malloc(size) malloc(size)
#define ns_realloc(ptr, size) realloc(ptr, size)

#ifdef neko_printf
#define ns_printf(...) neko_printf(__VA_ARGS__)
#else
#define ns_printf(...) printf(__VA_ARGS__)
#endif

static void free_root(neko_script_node_t *node) {
    neko_script_node_free(node->root.funcs);
    neko_script_node_free(node->root.stmts);
    ns_free(node);
}

neko_script_node_t *node_root(neko_script_marker_t marker, neko_script_node_t *funcs, neko_script_node_t *stmts) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_ROOT;
    node->marker = marker;
    node->root.funcs = funcs;
    node->root.stmts = stmts;
    node->codegen = neko_script_codegen_root;
    node->free = free_root;
    return node;
}

static void free_ident(neko_script_node_t *node) {
    ns_free(node->ident);
    ns_free(node);
}

neko_script_node_t *node_ident(neko_script_marker_t marker, char *name) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_IDENT;
    node->marker = marker;
    node->ident = name;
    node->codegen = neko_script_codegen_ident;
    node->free = free_ident;
    return node;
}

static void free_unary(neko_script_node_t *node) {
    neko_script_node_free(node->unary.val);
    ns_free(node);
}

neko_script_node_t *node_unary(neko_script_marker_t marker, int op, neko_script_node_t *val) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_UNARY;
    node->marker = marker;
    node->unary.op = op;
    node->unary.val = val;
    node->codegen = neko_script_codegen_unary;
    node->free = free_unary;
    return node;
}

static void free_binary(neko_script_node_t *node) {
    neko_script_node_free(node->binary.a);
    neko_script_node_free(node->binary.b);
    ns_free(node);
}

neko_script_node_t *node_binary(neko_script_marker_t marker, int op, neko_script_node_t *a, neko_script_node_t *b) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_BINARY;
    node->marker = marker;
    node->binary.op = op;
    node->binary.a = a;
    node->binary.b = b;
    node->codegen = neko_script_codegen_binary;
    node->free = free_binary;
    return node;
}

static void free_number(neko_script_node_t *node) { ns_free(node); }

neko_script_node_t *node_number(neko_script_marker_t marker, double number) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_NUMBER;
    node->marker = marker;
    node->number = number;
    node->codegen = neko_script_codegen_double;
    node->free = free_number;
    return node;
}

static void free_string(neko_script_node_t *node) {
    ns_free(node->string);
    ns_free(node);
}

neko_script_node_t *node_string(neko_script_marker_t marker, char *string) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_STRING;
    node->marker = marker;
    node->string = string;
    node->codegen = neko_script_codegen_string;
    node->free = free_string;
    return node;
}

static void free_call(neko_script_node_t *node) {
    neko_script_node_free(node->call.func);
    neko_script_node_free(node->call.args);
    ns_free(node);
}

neko_script_node_t *node_call(neko_script_marker_t marker, neko_script_node_t *func, neko_script_node_t *args) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_CALL;
    node->marker = marker;
    node->call.func = func;
    node->call.args = args;
    node->codegen = neko_script_codegen_call;
    node->free = free_call;
    return node;
}

static void free_func(neko_script_node_t *node) {
    neko_script_node_free(node->func.body);
    for (int i = 0; i < neko_script_vector_size(node->func.args); i++) ns_free(node->func.args[i]);
    neko_script_vector_free(node->func.args);
    ns_free(node->func.name);
    ns_free(node);
}

neko_script_node_t *node_func(neko_script_marker_t marker, char *name, neko_script_vector(char *) args, neko_script_node_t *body) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_FUNC;
    node->marker = marker;
    node->func.name = name;
    node->func.args = args;
    node->func.body = body;
    node->codegen = neko_script_codegen_func;
    node->free = free_func;
    return node;
}

static void free_return(neko_script_node_t *node) {
    if (node->ret) neko_script_node_free(node->ret);
    ns_free(node);
}

neko_script_node_t *node_return(neko_script_marker_t marker, neko_script_node_t *expr) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_RETURN;
    node->marker = marker;
    node->ret = expr;
    node->codegen = neko_script_codegen_return;
    node->free = free_return;
    return node;
}

static void free_cond(neko_script_node_t *node) {
    neko_script_node_free(node->cond.arg);
    neko_script_node_free(node->cond.body);
    if (node->cond.elsebody) neko_script_node_free(node->cond.elsebody);
    ns_free(node);
}

neko_script_node_t *node_cond(neko_script_marker_t marker, neko_script_node_t *arg, neko_script_node_t *body, neko_script_node_t *elsebody) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_COND;
    node->marker = marker;
    node->cond.arg = arg;
    node->cond.body = body;
    node->cond.elsebody = elsebody;
    node->codegen = neko_script_codegen_cond;
    node->free = free_cond;
    return node;
}

static void free_loop(neko_script_node_t *node) {
    neko_script_node_free(node->loop.arg);
    neko_script_node_free(node->loop.body);
    ns_free(node);
}

neko_script_node_t *node_loop(neko_script_marker_t marker, neko_script_node_t *arg, neko_script_node_t *body) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_LOOP;
    node->marker = marker;
    node->loop.arg = arg;
    node->loop.body = body;
    node->codegen = neko_script_codegen_loop;
    node->free = free_loop;
    return node;
}

static void free_break(neko_script_node_t *node) { ns_free(node); }

neko_script_node_t *node_break(neko_script_marker_t marker) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_BREAK;
    node->marker = marker;
    node->codegen = neko_script_codegen_break;
    node->free = free_break;
    return node;
}

static void free_decl(neko_script_node_t *node) {
    for (int i = 0; i < neko_script_vector_size(node->decl.names); i++) {
        ns_free(node->decl.names[i]);
    }
    neko_script_vector_free(node->decl.names);
    neko_script_node_free(node->decl.value);
    ns_free(node);
}

neko_script_node_t *node_decl(neko_script_marker_t marker, neko_script_vector(char *) names, neko_script_node_t *value) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_DECL;
    node->marker = marker;
    node->decl.names = names;
    node->decl.value = value;
    node->codegen = neko_script_codegen_decl;
    node->free = free_decl;
    return node;
}

static void free_index(neko_script_node_t *node) {
    neko_script_node_free(node->index.var);
    neko_script_node_free(node->index.expr);
    ns_free(node);
}

neko_script_node_t *node_index(neko_script_marker_t marker, neko_script_node_t *var, neko_script_node_t *expr) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_INDEX;
    node->marker = marker;
    node->index.var = var;
    node->index.expr = expr;
    node->codegen = neko_script_codegen_index;
    node->free = free_index;
    return node;
}

static void free_block(neko_script_node_t *node) {
    for (int i = 0; i < neko_script_vector_size(node->block); i++) neko_script_node_free(node->block[i]);
    neko_script_vector_free(node->block);
    ns_free(node);
}

neko_script_node_t *node_block(neko_script_marker_t marker, neko_script_vector(neko_script_node_t *) list) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_BLOCK;
    node->marker = marker;
    node->block = list;
    node->codegen = neko_script_codegen_block;
    node->free = free_block;
    return node;
}

static void free_member(neko_script_node_t *node) {
    neko_script_node_free(node->member.parent);
    ns_free(node->member.name);
    ns_free(node);
}

neko_script_node_t *node_member(neko_script_marker_t marker, neko_script_node_t *parent, char *name) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_MEMBER;
    node->marker = marker;
    node->member.name = name;
    node->member.parent = parent;
    node->codegen = neko_script_codegen_member;
    node->free = free_member;
    return node;
}

static void free_import(neko_script_node_t *node) {
    ns_free(node->import);
    ns_free(node);
}

neko_script_node_t *node_import(neko_script_marker_t marker, char *name) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_MEMBER;
    node->marker = marker;
    node->import = name;
    node->codegen = neko_script_codegen_import;
    node->free = free_import;
    return node;
}

static void free_class(neko_script_node_t *node) {
    neko_script_vector_free(node->class_t.methods);
    ns_free(node->class_t.name);
    ns_free(node);
}

neko_script_node_t *node_class(neko_script_marker_t marker, char *name, neko_script_vector(neko_script_node_t *) list) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_CLASS;
    node->marker = marker;
    node->class_t.name = name;
    node->class_t.methods = list;
    node->codegen = neko_script_codegen_class;
    node->free = free_class;
    return node;
}

static void free_trycatch(neko_script_node_t *node) {
    neko_script_node_free(node->trycatch.tryblock);
    neko_script_node_free(node->trycatch.catchblock);
    ns_free(node);
}

neko_script_node_t *node_trycatch(neko_script_marker_t marker, neko_script_node_t *tryblock, neko_script_node_t *catchblock) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_TRYCATCH;
    node->marker = marker;
    node->trycatch.tryblock = tryblock;
    node->trycatch.catchblock = catchblock;
    node->codegen = neko_script_codegen_trycatch;
    node->free = free_trycatch;
    return node;
}

static void free_throw(neko_script_node_t *node) {
    neko_script_node_free(node->throw_t);
    ns_free(node);
}

neko_script_node_t *node_throw(neko_script_marker_t marker, neko_script_node_t *expr) {
    neko_script_node_t *node = (neko_script_node_t *)ns_malloc(sizeof(neko_script_node_t));
    node->type = NEKO_SC_NODETYPE_THROW;
    node->marker = marker;
    node->throw_t = expr;
    node->codegen = neko_script_codegen_throw;
    node->free = free_throw;
    return node;
}

static void printescape(char *str) {
    while (*str != '\0') {
        if (*str == '\n')
            ns_printf("\\n");
        else if (*str == '\r')
            ns_printf("\\r");
        else
            putchar(*str);
        str++;
    }
}

static void printtab(int n) {
    for (int i = 0; i < n; i++) ns_printf("  ");
}

void neko_script_node_print(neko_script_node_t *node, int ind) {
    printtab(ind);
    switch (node->type) {
        case NEKO_SC_NODETYPE_ROOT:
            ns_printf("root\n");
            neko_script_node_print(node->root.funcs, ind + 1);
            neko_script_node_print(node->root.stmts, ind + 1);
            break;
        case NEKO_SC_NODETYPE_BLOCK:
            ns_printf("block\n");
            for (int i = 0; i < neko_script_vector_size(node->block); i++) {
                neko_script_node_print(node->block[i], ind + 1);
            }
            break;
        case NEKO_SC_NODETYPE_IDENT:
            ns_printf("ident: %s\n", node->ident);
            break;
        case NEKO_SC_NODETYPE_UNARY:
            ns_printf("unary: %s\n", neko_script_tokenstr(node->unary.op));
            neko_script_node_print(node->unary.val, ind + 1);
            break;
        case NEKO_SC_NODETYPE_BINARY:
            ns_printf("binary: %s\n", neko_script_tokenstr(node->binary.op));
            neko_script_node_print(node->binary.a, ind + 1);
            neko_script_node_print(node->binary.b, ind + 1);
            break;
        case NEKO_SC_NODETYPE_NUMBER:
            ns_printf("num %g\n", node->number);
            break;
        case NEKO_SC_NODETYPE_STRING:
            ns_printf("string \"");
            printescape(node->string);
            ns_printf("\"\n");
            break;
        case NEKO_SC_NODETYPE_CALL:
            ns_printf("call\n");
            printtab(ind);
            ns_printf("->func:\n");
            neko_script_node_print(node->call.func, ind + 1);
            printtab(ind);
            ns_printf("->args:\n");
            neko_script_node_print(node->call.args, ind + 1);
            break;
        case NEKO_SC_NODETYPE_FUNC:
            ns_printf("function: %s(", node->func.name);
            for (int i = 0; i < neko_script_vector_size(node->func.args); i++) {
                ns_printf("%s", node->func.args[i]);
                if (i != neko_script_vector_size(node->func.args) - 1) ns_printf(", ");
            }
            ns_printf(")\n");
            neko_script_node_print(node->func.body, ind + 1);
            break;
        case NEKO_SC_NODETYPE_RETURN:
            ns_printf("return\n");
            if (node->ret) neko_script_node_print(node->ret, ind + 1);
            break;
        case NEKO_SC_NODETYPE_COND:
            ns_printf("cond\n");
            printtab(ind);
            ns_printf("->arg:\n");
            neko_script_node_print(node->cond.arg, ind + 1);
            printtab(ind);
            ns_printf("->body:\n");
            neko_script_node_print(node->cond.body, ind + 1);
            break;
        case NEKO_SC_NODETYPE_LOOP:
            ns_printf("loop\n");
            printtab(ind);
            ns_printf("->arg:\n");
            neko_script_node_print(node->loop.arg, ind + 1);
            printtab(ind);
            ns_printf("->body:\n");
            neko_script_node_print(node->loop.body, ind + 1);
            break;
        case NEKO_SC_NODETYPE_BREAK:
            ns_printf("break\n");
            break;
        case NEKO_SC_NODETYPE_DECL:
            ns_printf("decl\n");
            printtab(ind);
            ns_printf("->name:\n");
            for (int i = 0; i < neko_script_vector_size(node->decl.names); i++) {
                printtab(ind + 1);
                ns_printf("%s\n", node->decl.names[i]);
            }
            printtab(ind);
            ns_printf("->value:\n");
            neko_script_node_print(node->decl.value, ind + 1);
            break;
        case NEKO_SC_NODETYPE_INDEX:
            ns_printf("index\n");
            neko_script_node_print(node->index.var, ind + 1);
            neko_script_node_print(node->index.expr, ind + 1);
            break;
        case NEKO_SC_NODETYPE_IMPORT:
            ns_printf("import \"%s\"\n", node->string);
            break;
        case NEKO_SC_NODETYPE_MEMBER:
            ns_printf("member \"%s\"\n", node->member.name);
            neko_script_node_print(node->member.parent, ind + 1);
            break;
    }
}

// Bytecode Begin

neko_script_binary_t *neko_script_binary_new() {
    neko_script_binary_t *bin = (neko_script_binary_t *)ns_malloc(sizeof(neko_script_binary_t));
    bin->adresses = NULL;
    bin->symbols = NULL;
    bin->fadresses = NULL;
    bin->fsymbols = NULL;
    bin->debug = NULL;
    bin->size = 0;
    bin->block = NULL;
    bin->index = 0;
    bin->loop = -1;
    return bin;
}

void neko_script_binary_free(neko_script_binary_t *bin) {
    neko_script_vector_free(bin->adresses);
    for (int i = 0; i < neko_script_vector_size(bin->symbols); i++) {
        ns_free(bin->symbols[i]);
    }
    neko_script_vector_free(bin->symbols);
    neko_script_vector_free(bin->fadresses);
    for (int i = 0; i < neko_script_vector_size(bin->fsymbols); i++) {
        ns_free(bin->fsymbols[i]);
    }
    neko_script_vector_free(bin->fsymbols);
    neko_script_vector_free(bin->debug);
    ns_free(bin->block);
    ns_free(bin);
}

void neko_script_binary_save(neko_script_binary_t *bin, char *filename) {
    FILE *file = fopen(filename, "wb");

    fwrite(bin->block, 1, bin->size, file);

    fclose(file);
}

int neko_script_bytecode_emit(neko_script_binary_t *bin, int opcode) {
    bin->size += 1;
    bin->block = (char *)ns_realloc(bin->block, bin->size);
    bin->block[bin->size - 1] = (char)opcode;
    return bin->size - 1;
}

int neko_script_bytecode_emitstr(neko_script_binary_t *bin, int opcode, char *string) {
    int len = (int)strlen(string);
    bin->size += 1 + len + 1;
    bin->block = (char *)ns_realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - len - 1] = (char)opcode;
    memcpy(&bin->block[bin->size - 1 - len], string, len);
    bin->block[bin->size - 1] = 0;
    return bin->size - 1 - len - 1;
}

int neko_script_bytecode_emitint(neko_script_binary_t *bin, int opcode, int number) {
    bin->size += 1 + 4;
    bin->block = (char *)ns_realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - 4] = (char)opcode;
    *(int *)(&(bin->block[bin->size - 4])) = number;
    return bin->size - 1 - 4;
}

int neko_script_bytecode_emitdouble(neko_script_binary_t *bin, int opcode, double number) {
    bin->size += 1 + 8;
    bin->block = (char *)ns_realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - 8] = (char)opcode;
    *(double *)(&(bin->block[bin->size - 8])) = number;
    return bin->size - 1 - 8;
}

int neko_script_bytecode_addlabel(neko_script_binary_t *bin, char *name, int adress) {
    neko_script_vector_push(bin->adresses, adress);
    neko_script_vector_push(bin->symbols, name);
    return 0;
}

int neko_script_bytecode_addtofill(neko_script_binary_t *bin, char *name, int adress) {
    neko_script_vector_push(bin->fadresses, adress);
    neko_script_vector_push(bin->fsymbols, name);
    return 0;
}

int neko_script_bytecode_fill(neko_script_binary_t *bin) {
    for (int i = 0; i < neko_script_vector_size(bin->fadresses); i++) {
        bin->fadresses[i];
        bin->fsymbols[i];
        for (int j = 0; j < neko_script_vector_size(bin->adresses); j++) {
            if (strcmp(bin->fsymbols[i], bin->symbols[j]) == 0) {
                *(int *)(&(bin->block[bin->fadresses[i]])) = bin->adresses[j];
                goto end;
            }
        }
        neko_script_throw("Symbol not found: %s\n", bin->fsymbols[i]);
    end:
        1;
    }
    return 0;
}

void neko_script_bytecode_adddebug(neko_script_binary_t *bin, neko_script_marker_t marker) {
    neko_script_vector_push(bin->debug, ((neko_script_debug_symbol_t){bin->size, marker.line, marker.column}));
}

// Bytecode End

// Codegen Begin

void neko_script_codegen_root(neko_script_node_t *node, neko_script_binary_t *binary) {
    for (int i = 0; i < neko_script_vector_size(node->root.funcs->block); i++) {
        neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_PUSHI, (int)neko_script_vector_size(node->root.funcs->block[i]->func.args));
        int adr = neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_PUSHI, 0x22222222);
        neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_STOREFN, node->root.funcs->block[i]->func.name);

        char *name = neko_script_mprintf("func_%s", node->root.funcs->block[i]->func.name);
        neko_script_bytecode_addtofill(binary, name, adr + 1);
    }

    for (int i = 0; i < neko_script_vector_size(node->root.stmts->block); i++) {
        neko_script_node_t *n = node->root.stmts->block[i];
        n->codegen(n, binary);
    }

    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_RETN);

    for (int i = 0; i < neko_script_vector_size(node->root.funcs->block); i++) {
        neko_script_node_t *f = node->root.funcs->block[i];
        f->codegen(f, binary);
    }
}
void neko_script_codegen_ident(neko_script_node_t *node, neko_script_binary_t *binary) {
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_PUSHV, node->ident);
}

void neko_script_codegen_unary(neko_script_node_t *node, neko_script_binary_t *binary) {
    node->unary.val->codegen(node->unary.val, binary);
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_UNARY, node->unary.op);
}

void neko_script_codegen_binary(neko_script_node_t *node, neko_script_binary_t *binary) {
    node->binary.a->codegen(node->binary.a, binary);
    node->binary.b->codegen(node->binary.b, binary);
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_BINARY, node->binary.op);
}

void neko_script_codegen_double(neko_script_node_t *node, neko_script_binary_t *binary) {
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emitdouble(binary, NEKO_SC_OPCODE_PUSHN, node->number);
}

void neko_script_codegen_string(neko_script_node_t *node, neko_script_binary_t *binary) {
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_PUSHS, node->string);
}

void neko_script_codegen_call(neko_script_node_t *node, neko_script_binary_t *binary) {
    for (int i = (int)neko_script_vector_size(node->call.args->block) - 1; i >= 0; i--) {
        neko_script_node_t *n = node->call.args->block[i];
        n->codegen(n, binary);
    }
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_PUSHI, (int)neko_script_vector_size(node->call.args->block));

    node->call.func->codegen(node->call.func, binary);

    if (node->call.func->type == NEKO_SC_NODETYPE_MEMBER) {
        int adr = binary->size - (int)strlen(node->call.func->member.name) - 1 - 1;
        binary->block[adr] = NEKO_SC_OPCODE_MEMBERD;
        neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_CALLM);
    } else {
        neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_CALL);
    }
}

void neko_script_codegen_func(neko_script_node_t *node, neko_script_binary_t *binary) {
    char *name = neko_script_mprintf("func_%s", node->func.name);
    neko_script_bytecode_addlabel(binary, name, binary->size);

    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_SCOPE);
    neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_STORE, "argc");
    for (int i = 0; i < (int)neko_script_vector_size(node->func.args); i++) {
        neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_STORE, node->func.args[i]);
    }

    node->func.body->codegen(node->func.body, binary);

    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_ENDSCOPE);

    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_RETN);
}

void neko_script_codegen_return(neko_script_node_t *node, neko_script_binary_t *binary) {
    if (node->ret != NULL) {
        node->ret->codegen(node->ret, binary);
        neko_script_bytecode_adddebug(binary, node->marker);
        neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_RET);
    } else {
        neko_script_bytecode_adddebug(binary, node->marker);
        neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_RETN);
    }
}

void neko_script_codegen_cond(neko_script_node_t *node, neko_script_binary_t *binary) {
    node->cond.arg->codegen(node->cond.arg, binary);
    neko_script_bytecode_adddebug(binary, node->marker);
    int adr = neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_BRZ, 0x22222222);  // fill it later with adress!!!
    char *name = neko_script_mprintf("cond_%d", binary->index);
    char *ename = neko_script_mprintf("condend_%d", binary->index++);
    neko_script_bytecode_addtofill(binary, strdup(name), adr + 1);
    node->cond.body->codegen(node->cond.body, binary);

    if (node->cond.elsebody != NULL) {
        adr = neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_JMP, 0x22222222);
        neko_script_bytecode_addlabel(binary, strdup(name), binary->size);
        neko_script_bytecode_addtofill(binary, strdup(ename), adr + 1);
        node->cond.elsebody->codegen(node->cond.elsebody, binary);
        neko_script_bytecode_addlabel(binary, strdup(ename), binary->size);
    } else {
        neko_script_bytecode_addlabel(binary, strdup(name), binary->size);
    }
    ns_free(ename);
    ns_free(name);
}

void neko_script_codegen_loop(neko_script_node_t *node, neko_script_binary_t *binary) {
    int old = binary->loop;
    int i = binary->index++;
    binary->loop = i;

    char *lname = neko_script_mprintf("loops_%d", i);
    char *lename = neko_script_mprintf("loopend_%d", i);

    neko_script_bytecode_addlabel(binary, strdup(lname), binary->size);
    node->loop.arg->codegen(node->loop.arg, binary);
    neko_script_bytecode_adddebug(binary, node->marker);
    int adr = neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_BRZ, 0x22222222);
    neko_script_bytecode_addtofill(binary, strdup(lename), adr + 1);
    node->loop.body->codegen(node->loop.body, binary);
    adr = neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_JMP, 0x22222222);
    neko_script_bytecode_addtofill(binary, strdup(lname), adr + 1);
    neko_script_bytecode_addlabel(binary, strdup(lename), binary->size);

    binary->loop = old;
    ns_free(lname);
    ns_free(lename);
}

void neko_script_codegen_break(neko_script_node_t *node, neko_script_binary_t *binary) {
    if (binary->loop < 0) neko_script_throw("No loop to break from at line %d column %d", node->marker.line, node->marker.column);

    char *lename = neko_script_mprintf("loopend_%d", binary->loop);

    neko_script_bytecode_adddebug(binary, node->marker);
    int adr = neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_JMP, 0x22222222);
    neko_script_bytecode_addtofill(binary, lename, adr + 1);
}

void neko_script_codegen_decl(neko_script_node_t *node, neko_script_binary_t *binary) {
    if (neko_script_vector_size(node->decl.names) == 1) {
        neko_script_bytecode_adddebug(binary, node->marker);
        neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_PUSHI, 0);
        neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_STORE, node->decl.names[0]);

        neko_script_bytecode_adddebug(binary, node->marker);
        neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_PUSHV, node->decl.names[0]);
        node->decl.value->codegen(node->decl.value, binary);
        neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_BINARY, NEKO_SC_TOKEN_ASSIGN);
    } else {
        neko_script_bytecode_adddebug(binary, node->marker);
        for (int i = 0; i < neko_script_vector_size(node->decl.names); i++) {
            neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_PUSHI, 0);
            neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_STORE, node->decl.names[i]);
        }

        neko_script_bytecode_adddebug(binary, node->marker);
        for (int i = 0; i < neko_script_vector_size(node->decl.names); i++) {
            // make tuple out of variables
            neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_PUSHV, node->decl.names[i]);
            if (i >= 1) neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_BINARY, NEKO_SC_TOKEN_COMMA);
        }
        node->decl.value->codegen(node->decl.value, binary);

        neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_BINARY, NEKO_SC_TOKEN_ASSIGN);
    }
}

void neko_script_codegen_index(neko_script_node_t *node, neko_script_binary_t *binary) {
    node->index.var->codegen(node->index.var, binary);
    node->index.expr->codegen(node->index.expr, binary);
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_INDEX);
}

void neko_script_codegen_block(neko_script_node_t *node, neko_script_binary_t *binary) {
    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_SCOPE);
    for (int i = 0; i < neko_script_vector_size(node->block); i++) node->block[i]->codegen(node->block[i], binary);
    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_ENDSCOPE);
}

void neko_script_codegen_member(neko_script_node_t *node, neko_script_binary_t *binary) {
    node->member.parent->codegen(node->member.parent, binary);
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_MEMBER, node->member.name);
}

void neko_script_codegen_import(neko_script_node_t *node, neko_script_binary_t *binary) {
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_IMPORT, node->import);
}

void neko_script_codegen_class(neko_script_node_t *node, neko_script_binary_t *binary) {}

void neko_script_codegen_trycatch(neko_script_node_t *node, neko_script_binary_t *binary) {
    int i = binary->index++;

    char *lname = neko_script_mprintf("catch_%d", i);
    char *lename = neko_script_mprintf("catchend_%d", i);

    neko_script_bytecode_adddebug(binary, node->marker);
    int adr = neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_TRY, 0x22222222);
    neko_script_bytecode_addtofill(binary, strdup(lname), adr + 1);

    node->trycatch.tryblock->codegen(node->trycatch.tryblock, binary);

    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_ENDTRY);
    adr = neko_script_bytecode_emitint(binary, NEKO_SC_OPCODE_JMP, 0x22222222);
    neko_script_bytecode_addtofill(binary, strdup(lename), adr + 1);

    neko_script_bytecode_addlabel(binary, strdup(lname), binary->size);
    // TODO: Add support for custom exception name
    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_SCOPE);
    neko_script_bytecode_emitstr(binary, NEKO_SC_OPCODE_STORE, "exception");
    node->trycatch.tryblock->codegen(node->trycatch.catchblock, binary);
    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_ENDSCOPE);

    neko_script_bytecode_addtofill(binary, strdup(lename), adr + 1);
    neko_script_bytecode_addlabel(binary, strdup(lename), binary->size);

    ns_free(lname);
    ns_free(lename);
}

void neko_script_codegen_throw(neko_script_node_t *node, neko_script_binary_t *binary) {
    node->ret->codegen(node->throw_t, binary);
    neko_script_bytecode_adddebug(binary, node->marker);
    neko_script_bytecode_emit(binary, NEKO_SC_OPCODE_THROW);
}

// Codegen End

neko_script_lexer_t *neko_script_lexer_new(char *input) {
    neko_script_lexer_t *lexer = (neko_script_lexer_t *)ns_malloc(sizeof(neko_script_lexer_t));
    lexer->input = input;
    lexer->buffer = (char *)ns_malloc(255);
    lexer->number = 0;
    lexer->marker.index = 0;
    lexer->marker.line = 1;
    lexer->marker.column = 1;
    lexer->lastchar = ' ';
    return lexer;
}

void neko_script_lexer_free(neko_script_lexer_t *lexer) {
    ns_free(lexer->buffer);
    ns_free(lexer);
}

static int nextchar(neko_script_lexer_t *lexer) {
    if (lexer->input[lexer->marker.index] == 0) {
        lexer->lastchar = EOF;
    } else {
        lexer->lastchar = lexer->input[lexer->marker.index];
        lexer->marker.index++;
    }

    if (lexer->lastchar == '\n') {
        lexer->marker.line++;
        lexer->marker.column = 1;
    } else {
        lexer->marker.column++;
    }
    if (lexer->lastchar < -1) lexer->lastchar = -1;
    return lexer->lastchar;
}

int neko_script_gettoken(neko_script_lexer_t *lexer) {
    while (isspace(lexer->lastchar)) nextchar(lexer);  // eaat white space

    lexer->startmarker = lexer->marker;

    if (lexer->lastchar <= 0) {
        return NEKO_SC_TOKEN_EOF;
    } else if (isalpha(lexer->lastchar) || lexer->lastchar == '_') {
        int ptr = 0;
        do {
            lexer->buffer[ptr++] = (char)tolower(lexer->lastchar);
            nextchar(lexer);
        } while (isalpha(lexer->lastchar) || isdigit(lexer->lastchar) || lexer->lastchar == '_');

        lexer->buffer[ptr] = 0;

        if (strcmp(lexer->buffer, "f") == 0 && lexer->lastchar == '"') {
            neko_script_gettoken(lexer);
            return NEKO_SC_TOKEN_FSTRING;
        }

        if (strcmp(lexer->buffer, "if") == 0)
            return NEKO_SC_TOKEN_IF;
        else if (strcmp(lexer->buffer, "else") == 0)
            return NEKO_SC_TOKEN_ELSE;
        else if (strcmp(lexer->buffer, "while") == 0)
            return NEKO_SC_TOKEN_WHILE;
        else if (strcmp(lexer->buffer, "let") == 0)
            return NEKO_SC_TOKEN_LET;
        else if (strcmp(lexer->buffer, "fn") == 0)
            return NEKO_SC_TOKEN_FN;
        else if (strcmp(lexer->buffer, "return") == 0)
            return NEKO_SC_TOKEN_RETURN;
        else if (strcmp(lexer->buffer, "break") == 0)
            return NEKO_SC_TOKEN_BREAK;
        else if (strcmp(lexer->buffer, "import") == 0)
            return NEKO_SC_TOKEN_IMPORT;
        else if (strcmp(lexer->buffer, "class") == 0)
            return NEKO_SC_TOKEN_CLASS;
        else if (strcmp(lexer->buffer, "try") == 0)
            return NEKO_SC_TOKEN_TRY;
        else if (strcmp(lexer->buffer, "catch") == 0)
            return NEKO_SC_TOKEN_CATCH;
        else if (strcmp(lexer->buffer, "throw") == 0)
            return NEKO_SC_TOKEN_THROW;

        return NEKO_SC_TOKEN_IDENT;
    } else if (isdigit(lexer->lastchar)) {
        double value = 0.0;
        int exponent = 0;

        if (lexer->lastchar == '0') {
            nextchar(lexer);
            if (lexer->lastchar == 'x' || lexer->lastchar == 'X') {
                nextchar(lexer);
                while (isdigit(lexer->lastchar) || (lexer->lastchar >= 'A' && lexer->lastchar <= 'F') || (lexer->lastchar >= 'a' && lexer->lastchar <= 'f')) {
                    if (lexer->lastchar >= 'a')
                        value = value * 16 + (lexer->lastchar - 'a' + 10);
                    else if (lexer->lastchar >= 'A')
                        value = value * 16 + (lexer->lastchar - 'A' + 10);
                    else
                        value = value * 16 + (lexer->lastchar - '0');
                    nextchar(lexer);
                }
                lexer->number = value;

                return NEKO_SC_TOKEN_NUMBER;
            }
        }

        while (isdigit(lexer->lastchar)) {
            value = value * 10 + (lexer->lastchar - '0');
            nextchar(lexer);
        }

        if (lexer->lastchar == '.') {
            nextchar(lexer);
            while (isdigit(lexer->lastchar)) {
                value = value * 10 + (lexer->lastchar - '0');
                exponent--;
                nextchar(lexer);
            }
        }

        if (lexer->lastchar == 'e' || lexer->lastchar == 'E') {
            int sign = 1;
            int i = 0;
            nextchar(lexer);
            if (lexer->lastchar == '-') {
                sign = -1;
                nextchar(lexer);
            } else if (lexer->lastchar == '+') {
                /* do nothing when positive :) */
                nextchar(lexer);
            }
            while (isdigit(lexer->lastchar)) {
                i = i * 10 + (lexer->lastchar - '0');
                nextchar(lexer);
            }
            exponent += sign * i;
        }

        while (exponent > 0) {
            value *= 10;
            exponent--;
        }
        while (exponent < 0) {
            value *= 0.1;
            exponent++;
        }
        lexer->number = value;

        return NEKO_SC_TOKEN_NUMBER;
    } else if (lexer->lastchar == '"') {
        nextchar(lexer);
        int ptr = 0;
        while (lexer->lastchar != '"' && lexer->lastchar > 0) {
            if (lexer->lastchar == '\\') {
                nextchar(lexer);
                switch (lexer->lastchar) {
                    case 'a':
                        lexer->buffer[ptr++] = '\a';
                        break;
                    case 'b':
                        lexer->buffer[ptr++] = '\b';
                        break;
                    case 'e':
                        lexer->buffer[ptr++] = 0x1B;
                        break;
                    case 'f':
                        lexer->buffer[ptr++] = '\f';
                        break;
                    case 'n':
                        lexer->buffer[ptr++] = '\n';
                        break;
                    case 'r':
                        lexer->buffer[ptr++] = '\r';
                        break;
                    case 't':
                        lexer->buffer[ptr++] = '\t';
                        break;
                    case 'v':
                        lexer->buffer[ptr++] = '\v';
                        break;
                    case '\\':
                        lexer->buffer[ptr++] = '\\';
                        break;
                    case '\'':
                        lexer->buffer[ptr++] = '\'';
                        break;
                    case '\"':
                        lexer->buffer[ptr++] = '\"';
                        break;
                    default:
                        neko_script_throw("Unexpected character after \\ in string on line %d column %d", lexer->marker.line, lexer->marker.column);
                        break;
                }
            } else {
                lexer->buffer[ptr++] = (char)lexer->lastchar;
            }
            nextchar(lexer);
        }
        if (lexer->lastchar < 0) neko_script_throw("Missing ending \" in string on line %d column %d", lexer->startmarker.line, lexer->startmarker.column);

        lexer->buffer[ptr] = 0;
        nextchar(lexer);
        return NEKO_SC_TOKEN_STRING;
    }

    int tmp = NEKO_SC_TOKEN_UNKOWN;
    switch (lexer->lastchar) {
        case ',':
            tmp = NEKO_SC_TOKEN_COMMA;
            break;
        case ':':
            tmp = NEKO_SC_TOKEN_COLON;
            break;
        case ';':
            tmp = NEKO_SC_TOKEN_SEMICOLON;
            break;
        case '.':
            tmp = NEKO_SC_TOKEN_DOT;
            break;
        case '+':
            tmp = NEKO_SC_TOKEN_PLUS;
            break;
        case '-':
            tmp = NEKO_SC_TOKEN_MINUS;
            break;
        case '/':
            if (nextchar(lexer) == '/') {
                size_t l = lexer->marker.line;
                while (nextchar(lexer) >= 0 && l == lexer->marker.line)
                    ;
                return neko_script_gettoken(lexer);
            } else if (lexer->lastchar == '*') {
                nextchar(lexer);
                while (!(lexer->lastchar == '*' && nextchar(lexer) == '/')) {
                    if (lexer->lastchar < 0) neko_script_throw("Missing \"*/\" in muliline comment starting at line %d colum %d", lexer->startmarker.line, lexer->startmarker.column);
                    nextchar(lexer);
                }
                nextchar(lexer);
                return neko_script_gettoken(lexer);
            } else
                return NEKO_SC_TOKEN_SLASH;
            break;
        case '*':
            tmp = NEKO_SC_TOKEN_ASTERISK;
            break;
        case '!': {
            if (nextchar(lexer) == '=')
                tmp = NEKO_SC_TOKEN_NOTEQUAL;
            else
                return NEKO_SC_TOKEN_EXCLAM;
            break;
        }
        case '=': {
            if (nextchar(lexer) == '=')
                tmp = NEKO_SC_TOKEN_EQUAL;
            else
                return NEKO_SC_TOKEN_ASSIGN;
            break;
        }
        case '(':
            tmp = NEKO_SC_TOKEN_LPAREN;
            break;
        case ')':
            tmp = NEKO_SC_TOKEN_RPAREN;
            break;
        case '{':
            tmp = NEKO_SC_TOKEN_LBRACE;
            break;
        case '}':
            tmp = NEKO_SC_TOKEN_RBRACE;
            break;
        case '[':
            tmp = NEKO_SC_TOKEN_LBRACK;
            break;
        case ']':
            tmp = NEKO_SC_TOKEN_RBRACK;
            break;
        case '<': {
            if (nextchar(lexer) == '=')
                tmp = NEKO_SC_TOKEN_LESSEQUAL;
            else
                return NEKO_SC_TOKEN_LCHEVR;
            break;
        }
        case '>': {
            if (nextchar(lexer) == '=')
                tmp = NEKO_SC_TOKEN_MOREEQUAL;
            else
                return NEKO_SC_TOKEN_RCHEVR;
            break;
        }
        case '&': {
            if (nextchar(lexer) == '&')
                tmp = NEKO_SC_TOKEN_AND;
            else
                return NEKO_SC_TOKEN_UNKOWN;
            break;
        }
        case '|': {
            if (nextchar(lexer) == '|')
                tmp = NEKO_SC_TOKEN_OR;
            else
                return NEKO_SC_TOKEN_UNKOWN;
            break;
        }
    }

    nextchar(lexer);
    return tmp;
}

char *neko_script_tokenstr(int token) {
    if (token < NEKO_SC_TOKEN_IDENT || token > NEKO_SC_TOKEN_UNKOWN) return "WRONG TOKEN!";
    char *names[] = {
            "NEKO_SC_TOKEN_IDENT",      //
            "NEKO_SC_TOKEN_NUMBER",     //
            "NEKO_SC_TOKEN_STRING",     //
            "NEKO_SC_TOKEN_FSTRING",    //
            "NEKO_SC_TOKEN_FN",         //
            "NEKO_SC_TOKEN_RETURN",     //
            "NEKO_SC_TOKEN_LET",        //
            "NEKO_SC_TOKEN_IF",         //
            "NEKO_SC_TOKEN_ELSE",       //
            "NEKO_SC_TOKEN_WHILE",      //
            "NEKO_SC_TOKEN_BREAK",      //
            "NEKO_SC_TOKEN_IMPORT",     //
            "NEKO_SC_TOKEN_CLASS",      //
            "NEKO_SC_TOKEN_TRY",        //
            "NEKO_SC_TOKEN_CATCH",      //
            "NEKO_SC_TOKEN_THROW",      //
                                        //
            "NEKO_SC_TOKEN_COLON",      // :
            "NEKO_SC_TOKEN_SEMICOLON",  // ;
            "NEKO_SC_TOKEN_DOT",        // .
            "NEKO_SC_TOKEN_COMMA",      // ,
                                        //
            "NEKO_SC_TOKEN_PLUS",       // +
            "NEKO_SC_TOKEN_MINUS",      // -
            "NEKO_SC_TOKEN_SLASH",      // /
            "NEKO_SC_TOKEN_ASTERISK",   // *
                                        //
            "NEKO_SC_TOKEN_ASSIGN",     // =
            "NEKO_SC_TOKEN_EQUAL",      // ==
            "NEKO_SC_TOKEN_NOTEQUAL",   // !=
            "NEKO_SC_TOKEN_LESSEQUAL",  // <=
            "NEKO_SC_TOKEN_MOREEQUAL",  // >=
            "NEKO_SC_TOKEN_LCHEVR",     // <
            "NEKO_SC_TOKEN_RCHEVR",     // >
            "NEKO_SC_TOKEN_AND",        // &&
            "NEKO_SC_TOKEN_OR",         // ||
                                        //
            "NEKO_SC_TOKEN_LPAREN",     // (
            "NEKO_SC_TOKEN_RPAREN",     // )
            "NEKO_SC_TOKEN_LBRACE",     // {
            "NEKO_SC_TOKEN_RBRACE",     // }
            "NEKO_SC_TOKEN_LBRACK",     // [
            "NEKO_SC_TOKEN_RBRACK",     // ]
            "NEKO_SC_TOKEN_EXCLAM",     // !
                                        //
            "NEKO_SC_TOKEN_EOF",        //
            "NEKO_SC_TOKEN_UNKOWN"      //
    };
    return names[token - NEKO_SC_TOKEN_IDENT];
}

neko_script_parser_t *neko_script_parser_new(char *input) {
    neko_script_parser_t *parser = (neko_script_parser_t *)ns_malloc(sizeof(neko_script_parser_t));
    parser->lexer = neko_script_lexer_new(input);
    parser->lasttoken = 0;
    return parser;
}

void neko_script_parser_free(neko_script_parser_t *parser) {
    neko_script_lexer_free(parser->lexer);
    ns_free(parser);
}

static int nexttoken(neko_script_parser_t *parser) { return parser->lasttoken = neko_script_gettoken(parser->lexer); }

static void match(neko_script_parser_t *parser, int token) {
    if (parser->lasttoken != token)
        neko_script_throw("Unexpected token, expect %s got %s on line %d column %d", neko_script_tokenstr(token), neko_script_tokenstr(parser->lasttoken), parser->lexer->startmarker.line,
                          parser->lexer->startmarker.column);
}

neko_script_node_t *expr(neko_script_parser_t *parser, int min, int mintok);

#define expr_norm(p) expr(p, 0, NEKO_SC_TOKEN_PLUS)
#define expr_tuple(p) expr(p, 0, NEKO_SC_TOKEN_COMMA)

// primary :=  ident | number | string | '(' expr ')' | UNARY_OP primary |
//             array | dict | fstring
//             primary '[' expr ']' | primary '(' expr ')'| primary '.' ident
// 对基本元素进行解析
neko_script_node_t *primary(neko_script_parser_t *parser) {
    neko_script_node_t *prim = NULL;
    neko_script_marker_t marker = parser->lexer->startmarker;
    if (parser->lasttoken == NEKO_SC_TOKEN_NUMBER) {
        prim = node_number(marker, parser->lexer->number);
        nexttoken(parser);
    } else if (parser->lasttoken == NEKO_SC_TOKEN_STRING) {
        prim = node_string(marker, strdup(parser->lexer->buffer));
        nexttoken(parser);
    } else if (parser->lasttoken == NEKO_SC_TOKEN_IDENT) {
        prim = node_ident(marker, strdup(parser->lexer->buffer));
        nexttoken(parser);
    } else if (parser->lasttoken == NEKO_SC_TOKEN_FSTRING) {
        // split string into parts
        neko_script_vector(char *) parts = NULL;
        int ptr = 0;
        char *buffer = (char *)ns_malloc(255);
        size_t len = strlen(parser->lexer->buffer);
        char *fstring = parser->lexer->buffer;
        int inexpr = 0;
        for (int i = 0; i < len; i++) {
            if (inexpr) {
                if (fstring[i] == '}') {
                    // 创建缓冲区副本并添加到 parts
                    buffer[ptr] = 0;
                    char *cpy = strdup(buffer);
                    ptr = 0;
                    neko_script_vector_push(parts, cpy);
                    inexpr = 0;
                    continue;
                }
            } else {
                // TODO: Add backslash support!
                if (fstring[i] == '$' && fstring[i + 1] == '{') {
                    if (i != 0) {
                        // create bufffer copy and add to parts
                        buffer[ptr] = 0;
                        char *cpy = strdup(buffer);
                        ptr = 0;
                        neko_script_vector_push(parts, cpy);
                    }
                    inexpr = 1;
                }
            }
            buffer[ptr] = fstring[i];
            ptr++;
        }
        if (ptr != 0) {
            buffer[ptr] = 0;
            char *cpy = strdup(buffer);
            ptr = 0;
            neko_script_vector_push(parts, cpy);
        }
        if (inexpr) neko_script_throw("Missing '}' in string interpolation on line %d column %d", parser->lexer->startmarker.line, parser->lexer->startmarker.column);

        neko_script_vector(neko_script_node_t *) nodes = NULL;
        for (int i = 0; i < neko_script_vector_size(parts); i++) {
            char *p = parts[i];
            if (parts[i][0] == '$' && parts[i][1] == '{') {
                // create parser and parse this string!!!
                neko_script_parser_t *pars = neko_script_parser_new(parts[i] + 2);
                nexttoken(pars);
                neko_script_node_t *ex = expr_norm(pars);
                neko_script_parser_free(pars);

                neko_script_vector(neko_script_node_t *) node = NULL;
                neko_script_vector_push(node, ex);
                neko_script_vector_push(nodes, node_call(marker, node_ident(marker, strdup("str")), node_block(marker, node)));
                ns_free(parts[i]);
            } else {
                // check memory management!!!
                neko_script_vector_push(nodes, node_string(marker, parts[i]));
            }
        }
        neko_script_vector_free(parts);

        size_t nodescnt = neko_script_vector_size(nodes);

        // generate binary additions for all nodes
        while (neko_script_vector_size(nodes) > 1) {
            neko_script_node_t *a = nodes[nodescnt - 2];
            neko_script_node_t *b = nodes[nodescnt - 1];

            nodes[nodescnt - 2] = node_binary(marker, NEKO_SC_TOKEN_PLUS, a, b);

            neko_script_vector_pop(nodes);
            nodescnt--;
        }
        nexttoken(parser);
        ns_free(buffer);
        prim = nodes[0];
        neko_script_vector_free(nodes);
    } else if (parser->lasttoken == NEKO_SC_TOKEN_LPAREN)  // '(' expr ')'
    {
        nexttoken(parser);
        prim = expr_norm(parser);
        match(parser, NEKO_SC_TOKEN_RPAREN);
        nexttoken(parser);
    } else if (parser->lasttoken == NEKO_SC_TOKEN_PLUS || parser->lasttoken == NEKO_SC_TOKEN_MINUS || parser->lasttoken == NEKO_SC_TOKEN_EXCLAM)  // UNARY_OP primary
    {
        // UNARY_OP primary
        int op = parser->lasttoken;
        nexttoken(parser);
        return node_unary(marker, op, primary(parser));
    } else if (parser->lasttoken == NEKO_SC_TOKEN_LBRACK) {
        // new array
        nexttoken(parser);
        neko_script_vector(neko_script_node_t *) elements = NULL;
        while (parser->lasttoken != NEKO_SC_TOKEN_RBRACK) {
            neko_script_node_t *e = expr_norm(parser);
            neko_script_vector_push(elements, e);
            if (parser->lasttoken != NEKO_SC_TOKEN_COMMA) break;
            nexttoken(parser);
        }
        match(parser, NEKO_SC_TOKEN_RBRACK);
        nexttoken(parser);

        // syntax sugar, convert [1,2,3] to list(1,2,3)
        prim = node_call(marker, node_ident(marker, strdup("list")), node_block(marker, elements));
    } else if (parser->lasttoken == NEKO_SC_TOKEN_LBRACE) {
        // new dictonary
        nexttoken(parser);
        neko_script_vector(neko_script_node_t *) keys = NULL;
        neko_script_vector(neko_script_node_t *) values = NULL;
        while (parser->lasttoken != NEKO_SC_TOKEN_RBRACE) {
            if (!(parser->lasttoken == NEKO_SC_TOKEN_STRING || parser->lasttoken == NEKO_SC_TOKEN_IDENT))
                neko_script_throw("Unexpected token, expect NEKO_SC_TOKEN_STRING or NEKO_SC_TOKEN_IDENT got %s on line %s column %s", neko_script_tokenstr(parser->lasttoken),
                                  parser->lexer->startmarker.line, parser->lexer->startmarker.column);
            neko_script_vector_push(keys, node_string(marker, strdup(parser->lexer->buffer)));

            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_COLON);
            nexttoken(parser);

            neko_script_node_t *e = expr_norm(parser);
            neko_script_vector_push(values, e);

            if (parser->lasttoken != NEKO_SC_TOKEN_COMMA) break;
            nexttoken(parser);
        }
        match(parser, NEKO_SC_TOKEN_RBRACE);
        nexttoken(parser);

        // syntax sugar, convert {a:1, b:2+2} to dict(list('a', 'b'), list(1, 2+2))
        neko_script_vector(neko_script_node_t *) args = NULL;
        neko_script_vector_push(args, node_call(marker, node_ident(marker, strdup("list")), node_block(marker, keys)));
        neko_script_vector_push(args, node_call(marker, node_ident(marker, strdup("list")), node_block(marker, values)));
        prim = node_call(marker, node_ident(marker, strdup("dict")), node_block(marker, args));
    } else {
        neko_script_throw("Unexpected token in primary line %d column %d", parser->lexer->startmarker.line, parser->lexer->startmarker.column);
    }

    while (parser->lasttoken == NEKO_SC_TOKEN_DOT || parser->lasttoken == NEKO_SC_TOKEN_LBRACK || parser->lasttoken == NEKO_SC_TOKEN_LPAREN) {
        if (parser->lasttoken == NEKO_SC_TOKEN_LPAREN)  // primary '(' expr ')'
        {
            nexttoken(parser);
            neko_script_vector(neko_script_node_t *) args = NULL;
            while (parser->lasttoken != NEKO_SC_TOKEN_RPAREN) {
                neko_script_node_t *e = expr_norm(parser);
                neko_script_vector_push(args, e);

                if (parser->lasttoken != NEKO_SC_TOKEN_COMMA) break;
                nexttoken(parser);
            }

            match(parser, NEKO_SC_TOKEN_RPAREN);
            nexttoken(parser);
            prim = node_call(marker, prim, node_block(marker, args));
        } else if (parser->lasttoken == NEKO_SC_TOKEN_LBRACK)  // primary '[' expr ']'
        {
            nexttoken(parser);
            neko_script_node_t *e = expr_norm(parser);
            match(parser, NEKO_SC_TOKEN_RBRACK);
            nexttoken(parser);
            prim = node_index(marker, prim, e);
        } else if (parser->lasttoken == NEKO_SC_TOKEN_DOT)  // primary '.' ident
        {
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_IDENT);
            char *name = strdup(parser->lexer->buffer);
            nexttoken(parser);
            prim = node_member(marker, prim, name);
        }
    }
    return prim;
}

// expr := primary |  expr OP expr
neko_script_node_t *expr(neko_script_parser_t *parser, int min, int mintok) {
    int pre[] = {
            1,  // ,
            6,  // +
            6,  // -
            7,  // /
            7,  // *
            0,  // =
            4,  // ==
            4,  // !=
            5,  // <=
            5,  // >=
            5,  // <
            5,  // >
            3,  // &&
            2   // ||
    };
    neko_script_node_t *lhs = primary(parser);
    while (1) {
        if (parser->lasttoken < mintok || parser->lasttoken > NEKO_SC_TOKEN_OR || pre[parser->lasttoken - NEKO_SC_TOKEN_COMMA] < min) break;

        neko_script_marker_t marker = parser->lexer->startmarker;
        int op = parser->lasttoken;
        int prec = pre[parser->lasttoken - NEKO_SC_TOKEN_COMMA];
        int assoc = (parser->lasttoken == NEKO_SC_TOKEN_ASSIGN) ? 1 : 0;  // 0 left, 1 right
        int nextmin = assoc ? prec : prec + 1;
        nexttoken(parser);
        neko_script_node_t *rhs = expr(parser, nextmin, mintok);
        lhs = node_binary(marker, op, lhs, rhs);
    }
    return lhs;
}

// block := '{' statement '}'
// let := 'let' ident '=' expr ';'
// if := 'if' '(' expr ')' statement ['else' statement]
// while := 'while' '(' expr ')' statement
// return := 'return' ';' | 'return' expr ';'
// break := 'break' ';'
// import := 'import' ident ';'
// class := 'class' ident '{' methods '}'
// try := 'try' statement 'catch' statement
// throw := 'throw' expr ';'
// statement := block | let | if | while | funca | return | break | import | class | try | expr ';'
// 对声明式进行解析
neko_script_node_t *statment(neko_script_parser_t *parser) {
    neko_script_marker_t marker = parser->lexer->startmarker;
    switch (parser->lasttoken) {
        case NEKO_SC_TOKEN_LBRACE:
            nexttoken(parser);

            neko_script_vector(neko_script_node_t *) list = NULL;
            while (parser->lasttoken != NEKO_SC_TOKEN_RBRACE) {
                neko_script_node_t *node = statment(parser);
                neko_script_vector_push(list, node);
            }
            match(parser, NEKO_SC_TOKEN_RBRACE);

            nexttoken(parser);
            return node_block(marker, list);
        case NEKO_SC_TOKEN_LET:
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_IDENT);
            neko_script_vector(char *) names = NULL;
            char *name = strdup(parser->lexer->buffer);
            neko_script_vector_push(names, name);

            nexttoken(parser);
            while (parser->lasttoken == NEKO_SC_TOKEN_COMMA) {
                nexttoken(parser);
                match(parser, NEKO_SC_TOKEN_IDENT);
                char *name2 = strdup(parser->lexer->buffer);
                neko_script_vector_push(names, name2);
                nexttoken(parser);
            }

            match(parser, NEKO_SC_TOKEN_ASSIGN);
            nexttoken(parser);

            neko_script_node_t *exp = expr_tuple(parser);
            match(parser, NEKO_SC_TOKEN_SEMICOLON);

            nexttoken(parser);
            return node_decl(marker, names, exp);
        case NEKO_SC_TOKEN_IF:
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_LPAREN);

            nexttoken(parser);
            neko_script_node_t *arg = expr_norm(parser);
            match(parser, NEKO_SC_TOKEN_RPAREN);

            nexttoken(parser);
            neko_script_node_t *body = statment(parser);
            neko_script_node_t *elsebody = NULL;

            if (parser->lasttoken == NEKO_SC_TOKEN_ELSE) {
                nexttoken(parser);
                elsebody = statment(parser);
            }

            return node_cond(marker, arg, body, elsebody);
        case NEKO_SC_TOKEN_WHILE:
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_LPAREN);

            nexttoken(parser);
            neko_script_node_t *arg2 = expr_norm(parser);
            match(parser, NEKO_SC_TOKEN_RPAREN);

            nexttoken(parser);
            neko_script_node_t *body2 = statment(parser);

            return node_loop(marker, arg2, body2);
        case NEKO_SC_TOKEN_FN:
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_IDENT);

            char *fnname = strdup(parser->lexer->buffer);

            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_LPAREN);

            neko_script_vector(char *) args = NULL;

            while (nexttoken(parser) != NEKO_SC_TOKEN_RPAREN) {
                match(parser, NEKO_SC_TOKEN_IDENT);
                neko_script_vector_push(args, strdup(parser->lexer->buffer));
                if (nexttoken(parser) != NEKO_SC_TOKEN_COMMA) break;
            }

            match(parser, NEKO_SC_TOKEN_RPAREN);
            nexttoken(parser);

            neko_script_node_t *fnbody = statment(parser);

            return node_func(marker, fnname, args, fnbody);
        case NEKO_SC_TOKEN_RETURN:
            nexttoken(parser);

            neko_script_node_t *retnode = NULL;
            if (parser->lasttoken != NEKO_SC_TOKEN_SEMICOLON) retnode = expr_tuple(parser);

            match(parser, NEKO_SC_TOKEN_SEMICOLON);
            nexttoken(parser);

            return node_return(marker, retnode);
        case NEKO_SC_TOKEN_BREAK:
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_SEMICOLON);
            nexttoken(parser);

            return node_break(marker);
        case NEKO_SC_TOKEN_IMPORT:
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_IDENT);
            char *module_name = strdup(parser->lexer->buffer);

            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_SEMICOLON);

            nexttoken(parser);
            return node_import(marker, module_name);
        case NEKO_SC_TOKEN_CLASS:
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_IDENT);
            char *class_name = strdup(parser->lexer->buffer);

            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_LBRACE);

            nexttoken(parser);

            neko_script_vector(neko_script_node_t *) constructor = NULL;

            // TODO: move most of this nonsense to codegen!

            // generated constructor code:
            // let this = dict();
            // this.old_name = __class_old_name_;
            // ...
            // return this;

            neko_script_vector(char *) this_name = NULL;
            neko_script_vector_push(this_name, strdup("this"));
            neko_script_vector_push(constructor, node_decl(marker, this_name, node_call(marker, node_ident(marker, strdup("dict")), node_block(marker, NULL))));

            neko_script_vector(neko_script_node_t *) constructors = NULL;
            neko_script_vector(neko_script_node_t *) methods_list = NULL;
            while (parser->lasttoken != NEKO_SC_TOKEN_RBRACE) {
                if (parser->lasttoken != NEKO_SC_TOKEN_FN) {
                    neko_script_throw("Expect only methods inside class on line %s column %s", parser->lexer->startmarker.line, parser->lexer->startmarker.column);
                }
                neko_script_node_t *node = statment(parser);

                // change name to __class_name__
                char *old_name = node->func.name;
                char *new_name = ns_malloc(6 + strlen(old_name) + strlen(class_name));
                strcpy(new_name, "__");
                strcat(new_name, class_name);
                strcat(new_name, "_");
                strcat(new_name, old_name);
                strcat(new_name, "__");

                if (strcmp(old_name, class_name) == 0) {
                    neko_script_vector_push(constructors, node);
                }

                // add code to conctructor
                neko_script_vector_push(constructor,
                                        node_binary(marker, NEKO_SC_TOKEN_ASSIGN, node_member(marker, node_ident(marker, strdup("this")), strdup(old_name)), node_ident(marker, strdup(new_name))));

                ns_free(old_name);
                node->func.name = new_name;

                neko_script_vector_push(methods_list, node);
            }
            match(parser, NEKO_SC_TOKEN_RBRACE);

            if (neko_script_vector_size(constructors) > 1) {
                neko_script_throw("Class %s has more than one constructor", class_name);
            } else if (neko_script_vector_size(constructors) == 1) {
                if (constructors[0]->func.body->type != NEKO_SC_NODETYPE_BLOCK) {
                    neko_script_throw("Constructor body need to be block on line %d colum %d", constructors[0]->func.body->marker.line, constructors[0]->func.body->marker.column);
                }

                neko_script_vector(neko_script_node_t *) args = NULL;
                for (int i = 0; i < neko_script_vector_size(constructors[0]->func.args); i++) {
                    neko_script_vector_push(args, node_ident(marker, strdup(constructors[0]->func.args[i])));
                }

                neko_script_vector_push(constructor, node_call(marker, node_member(marker, node_ident(marker, strdup("this")), strdup(class_name)), node_block(marker, args)));
                neko_script_vector_push(constructor, node_return(marker, node_ident(marker, strdup("this"))));

                neko_script_vector(char *) args2 = NULL;
                for (int i = 0; i < neko_script_vector_size(constructors[0]->func.args); i++) {
                    neko_script_vector_push(args2, strdup(constructors[0]->func.args[i]));
                }
                neko_script_vector_push(methods_list, node_func(marker, strdup(class_name), args2, node_block(marker, constructor)));
            } else {
                neko_script_vector_push(constructor, node_return(marker, node_ident(marker, strdup("this"))));
                neko_script_vector_push(methods_list, node_func(marker, strdup(class_name), NULL, node_block(marker, constructor)));
            }

            neko_script_vector_free(constructors);
            nexttoken(parser);
            return node_class(marker, class_name, methods_list);
        case NEKO_SC_TOKEN_TRY:
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_LBRACE);
            neko_script_node_t *tryblock = statment(parser);

            match(parser, NEKO_SC_TOKEN_CATCH);
            nexttoken(parser);
            match(parser, NEKO_SC_TOKEN_LBRACE);
            neko_script_node_t *catchblock = statment(parser);

            return node_trycatch(marker, tryblock, catchblock);
        case NEKO_SC_TOKEN_THROW: {
            nexttoken(parser);
            neko_script_node_t *e = expr_norm(parser);
            match(parser, NEKO_SC_TOKEN_SEMICOLON);
            nexttoken(parser);
            return node_throw(marker, e);
        }
        default:;
            neko_script_node_t *e = expr_tuple(parser);
            match(parser, NEKO_SC_TOKEN_SEMICOLON);
            nexttoken(parser);
            return e;
    }
}

neko_script_node_t *neko_script_parse(neko_script_parser_t *parser) {
    nexttoken(parser);
    neko_script_vector(neko_script_node_t *) funcs = NULL;
    neko_script_vector(neko_script_node_t *) stmts = NULL;
    neko_script_marker_t marker = parser->lexer->marker;
    while (parser->lasttoken != NEKO_SC_TOKEN_EOF) {
        neko_script_node_t *n = statment(parser);

        if (n->type == NEKO_SC_NODETYPE_FUNC) {
            neko_script_vector_push(funcs, n);
        } else if (n->type == NEKO_SC_NODETYPE_CLASS) {
            // generate constructor from class
            for (int i = 0; i < neko_script_vector_size(n->class_t.methods); i++) neko_script_vector_push(funcs, n->class_t.methods[i]);
            neko_script_node_free(n);
        } else {
            neko_script_vector_push(stmts, n);
        }
    }
    neko_script_node_t *root = node_root(marker, node_block(marker, funcs), node_block(marker, stmts));
    return root;
}

jmp_buf __neko_script_ex_buf__;
char neko_script_ex_msg[256];

void neko_script_throw(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vsnprintf(neko_script_ex_msg, 256, msg, args);
    va_end(args);

    longjmp(__neko_script_ex_buf__, 1);
}

char *neko_script_mprintf(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    char *buffer = (char *)ns_malloc(124);
    vsprintf(buffer, fmt, va);
    va_end(va);
    return buffer;
}

void neko_script_vector_grow_impl(void **vector, size_t more, size_t type_size) {
    neko_script_vector_t *meta = neko_script_vector_meta(*vector);
    size_t count = 0;
    void *data = NULL;

    if (*vector) {
        count = 2 * meta->allocated + more;
        data = ns_realloc(meta, type_size * count + sizeof *meta);
    } else {
        count = more + 1;
        data = ns_malloc(type_size * count + sizeof *meta);
        ((neko_script_vector_t *)data)->used = 0;
    }

    meta = (neko_script_vector_t *)data;
    meta->allocated = count;
    *vector = meta + 1;
}

// VM BEGIN

static void print(int argc, neko_script_ctx_t *ctx) {
    int end = (int)neko_script_vector_size(ctx->stack);
    for (int i = 0; i < argc; i++) {
        neko_script_value_t *v = neko_script_vector_pop(ctx->stack);
        if (v->type == NEKO_SC_VALUE_NUMBER)
            ns_printf("%g", v->number);
        else if (v->type == NEKO_SC_VALUE_STRING)
            ns_printf("%s", v->string);
    }
    ns_printf("\n");
    fflush(stdout);
    neko_script_vector_push(ctx->stack, neko_script_value_null());
}

static void list(int argc, neko_script_ctx_t *ctx) {
    neko_script_vector(neko_script_value_t *) arr = NULL;
    for (int i = (int)neko_script_vector_size(ctx->stack) - 1; i >= (int)neko_script_vector_size(ctx->stack) - argc; i--) {
        neko_script_vector_push(arr, ctx->stack[i]);
    }
    neko_script_vector_shrinkby(ctx->stack, argc);

    neko_script_vector_push(ctx->stack, neko_script_value_array(arr));
}

static void dict(int argc, neko_script_ctx_t *ctx) {
    if (argc != 0 && argc != 2) neko_script_throw("Function dict() takes 0 or 2 arguments");

    if (argc == 0)
        neko_script_vector_push(ctx->stack, neko_script_value_dict(NULL, NULL));
    else {
        neko_script_value_t *k = neko_script_vector_pop(ctx->stack);
        neko_script_value_t *v = neko_script_vector_pop(ctx->stack);
        if (v->type != NEKO_SC_VALUE_ARRAY || k->type != NEKO_SC_VALUE_ARRAY) neko_script_throw("Function dict takes arguments of type array");
        neko_script_vector(char *) keys = NULL;
        for (size_t i = 0; i < neko_script_vector_size(k->array); i++) {
            neko_script_value_t *key = neko_script_value_get(i, k);
            if (key->type != NEKO_SC_VALUE_STRING) neko_script_throw("Key of dictionary must be a string");
            neko_script_vector_push(keys, strdup(key->string));
        }
        neko_script_vector(neko_script_value_t *) values = NULL;
        for (size_t i = 0; i < neko_script_vector_size(v->array); i++) {
            neko_script_vector_push(values, neko_script_value_get(i, v));
        }

        neko_script_vector_push(ctx->stack, neko_script_value_dict(keys, values));
    }
}

static void len(int argc, neko_script_ctx_t *ctx) {
    neko_script_value_t *v = neko_script_vector_pop(ctx->stack);
    while (v->type == NEKO_SC_VALUE_REF) v = v->ref;
    if (v->type == NEKO_SC_VALUE_STRING) {
        neko_script_vector_push(ctx->stack, neko_script_value_number((double)strlen(v->string)));
        return;
    } else if (v->type == NEKO_SC_VALUE_ARRAY) {
        neko_script_vector_push(ctx->stack, neko_script_value_number((double)neko_script_vector_size(v->array)));
        return;
    }

    neko_script_throw("Function len() need argument of type string or array.");
}

static void ord(int argc, neko_script_ctx_t *ctx) {
    neko_script_value_t *v = neko_script_vector_pop(ctx->stack);
    while (v->type == NEKO_SC_VALUE_REF) v = v->ref;

    if (v->type == NEKO_SC_VALUE_STRING) {
        neko_script_vector_push(ctx->stack, neko_script_value_number(v->string[0]));
        return;
    }

    neko_script_throw("Function ord need argument of type string");
}

static void chr(int argc, neko_script_ctx_t *ctx) {
    neko_script_value_t *v = neko_script_vector_pop(ctx->stack);
    if (v->type == NEKO_SC_VALUE_NUMBER) {
        char *data = (char *)ns_malloc(sizeof(char));
        data[0] = (char)v->number;
        neko_script_vector_push(ctx->stack, neko_script_value_string(data));
        return;
    }

    neko_script_throw("Function chr need argument of type number.");
}

static void super(int argc, neko_script_ctx_t *ctx) {
    neko_script_value_t *a = neko_script_vector_pop(ctx->stack);
    neko_script_value_t *b = neko_script_vector_pop(ctx->stack);
    while (a->type == NEKO_SC_VALUE_REF) a = a->ref;

    while (b->type == NEKO_SC_VALUE_REF) b = b->ref;

    if (a->type == NEKO_SC_VALUE_DICT && b->type == NEKO_SC_VALUE_DICT) {
        for (size_t i = 0; i < neko_script_vector_size(b->dict.names); i++) {
            int flag = 0;
            for (size_t j = 0; j < neko_script_vector_size(a->dict.names); j++) {
                if (strcmp(a->dict.names[j], b->dict.names[i]) == 0) {
                    goto end;
                }
            }
            neko_script_vector_push(a->dict.names, strdup(b->dict.names[i]));
            neko_script_vector_push(a->dict.values, b->dict.values[i]);
        end:;
        }

        neko_script_vector_push(ctx->stack, a);
        return;
    }

    neko_script_throw("Function _super_ need arguments of type dict.");
}

static void str(int argc, neko_script_ctx_t *ctx) {
    neko_script_value_t *v = neko_script_vector_pop(ctx->stack);
    if (v->type == NEKO_SC_VALUE_STRING) {
        neko_script_vector_push(ctx->stack, v);
        return;
    }

    char *str = NULL;
    if (v->type == NEKO_SC_VALUE_NUMBER)
        str = neko_script_mprintf("%g", v->number);
    else
        str = neko_script_mprintf("[object]");

    neko_script_vector_push(ctx->stack, neko_script_value_string(str));
}

static void isnull(int argc, neko_script_ctx_t *ctx) {
    neko_script_value_t *val = neko_script_vector_pop(ctx->stack);
    neko_script_vector_push(ctx->stack, neko_script_value_number(val->type == NEKO_SC_VALUE_NULL));
}

static void input(int argc, neko_script_ctx_t *ctx) {
    if (argc > 0) {
        neko_script_value_t *v = neko_script_vector_pop(ctx->stack);
        ns_printf("%s", v->string);
    }

    char buff[256];

    if (scanf("%255s", buff) == 0)
        neko_script_vector_push(ctx->stack, neko_script_value_string(strdup("")));
    else
        neko_script_vector_push(ctx->stack, neko_script_value_string(strdup(buff)));
}

void neko_script_builtin_install(neko_script_ctx_t *ctx) {
    neko_script_ctx_addfn(ctx, NULL, strdup("print"), 0, 0, print);
    neko_script_ctx_addfn(ctx, NULL, strdup("list"), 0, 0, list);
    neko_script_ctx_addfn(ctx, NULL, strdup("dict"), 0, 0, dict);
    neko_script_ctx_addfn(ctx, NULL, strdup("len"), 1, 0, len);
    neko_script_ctx_addfn(ctx, NULL, strdup("ord"), 1, 0, ord);
    neko_script_ctx_addfn(ctx, NULL, strdup("chr"), 1, 0, chr);
    neko_script_ctx_addfn(ctx, NULL, strdup("_super_"), 2, 0, super);
    neko_script_ctx_addfn(ctx, NULL, strdup("str"), 1, 0, str);
    neko_script_ctx_addfn(ctx, NULL, strdup("isnull"), 1, 0, isnull);
    neko_script_ctx_addfn(ctx, NULL, strdup("input"), 0, 0, input);

    neko_script_ctx_addvar(ctx, strdup("null"), neko_script_value_null());
    neko_script_ctx_addvar(ctx, strdup("true"), neko_script_value_number(1.0));
    neko_script_ctx_addvar(ctx, strdup("false"), neko_script_value_number(0.0));
}

neko_script_ctx_t *neko_script_ctx_new(neko_script_ctx_t *parent) {
    neko_script_ctx_t *ctx = (neko_script_ctx_t *)neko_script_gc_alloc_ctx();
    ctx->parent = parent;
    ctx->vars = NULL;
    ctx->stack = NULL;
    ctx->markbit = 0;
    return ctx;
}

void neko_script_ctx_free(neko_script_ctx_t *ctx) {

    for (size_t i = 0; i < neko_script_vector_size(ctx->vars); i++) {
        ns_free(ctx->vars[i]->name);  // strdup 申请的字符串需要释放
        ns_free(ctx->vars[i]);
    }
    neko_script_vector_free(ctx->vars);

    neko_script_vector_free(ctx->stack);

    ns_free(ctx);
}

neko_script_value_t *neko_script_ctx_getvar(neko_script_ctx_t *ctx, char *name) {
    neko_script_ctx_t *c = ctx;
    while (c) {
        for (int i = (int)neko_script_vector_size(c->vars) - 1; i >= 0; i--) {
            neko_script_var_t *v = c->vars[i];

            if (strcmp(v->name, name) == 0) return v->val;
        }
        c = c->parent;
    }
    return NULL;
}

void neko_script_ctx_addvar(neko_script_ctx_t *ctx, char *name, neko_script_value_t *val) {
    neko_script_var_t *var = (neko_script_var_t *)ns_malloc(sizeof(neko_script_var_t));
    var->name = name;
    var->val = val;

    neko_script_vector_push(ctx->vars, var);
}

neko_script_fn_t *neko_script_ctx_getfn(neko_script_ctx_t *ctx, char *name) {
    neko_script_value_t *v = neko_script_ctx_getvar(ctx, name);
    while (v != NULL && v->type == NEKO_SC_VALUE_REF) v = v->ref;  // 追溯函数引用
    if (v == NULL || v->type != NEKO_SC_VALUE_FN) return NULL;
    return v->fn;
}

void neko_script_ctx_addfn(neko_script_ctx_t *ctx, neko_script_binary_t *binary, char *name, int argc, int address, void (*fn)(int, neko_script_ctx_t *)) {
    neko_script_fn_t *func = (neko_script_fn_t *)ns_malloc(sizeof(neko_script_fn_t));
    func->address = address;
    func->argc = argc;
    func->native = fn;
    func->func_binary = binary;
    func->ctx = ctx;

    neko_script_value_t *v = neko_script_value_fn(func);
    neko_script_ctx_addvar(ctx, name, v);
}

neko_script_vector(neko_script_value_t *) values;
neko_script_vector(neko_script_ctx_t *) ctxs;

size_t maxmem = 1024;
size_t usedmem = 0;

extern neko_script_ctx_t **current;
extern neko_script_vector(neko_script_ctx_t *) ctx_stack;

void neko_script_gc_count(u64 *max, u64 *used) {
    *max = maxmem;
    *used = usedmem;
}

void *neko_script_safe_alloc(int size) {
    void *data = ns_malloc(size);
    if (!data) neko_script_throw("Cannot alloc memory!");
    return data;
}

neko_script_value_t *neko_script_gc_alloc_value() {
    void *v = neko_script_safe_alloc(sizeof(neko_script_value_t));
    neko_script_vector_push(values, v);
    usedmem += 1;
    return v;
}

void neko_script_gc_trigger() {
    if (usedmem >= maxmem) {
        neko_script_gc_collect();
        usedmem = neko_script_vector_size(values);
        if (usedmem >= maxmem) maxmem = maxmem * 2;
    }
}

neko_script_ctx_t *neko_script_gc_alloc_ctx() {
    void *ctx = neko_script_safe_alloc(sizeof(neko_script_ctx_t));
    neko_script_vector_push(ctxs, ctx);
    return ctx;
}

static void gc_mark(neko_script_value_t *val);

static void gc_markctx(neko_script_ctx_t *c) {
    while (c) {
        if (c->markbit != 1) {
            c->markbit = 1;
            for (size_t i = 0; i < neko_script_vector_size(c->vars); i++) {
                gc_mark(c->vars[i]->val);
            }
            for (size_t i = 0; i < neko_script_vector_size(c->stack); i++) {
                gc_mark(c->stack[i]);
            }
        }
        c = c->parent;
    }
}

static void gc_mark(neko_script_value_t *val) {
    if (val->markbit == 1) return;

    val->markbit = 1;

    if (val->type == NEKO_SC_VALUE_REF) {
        gc_mark(val->ref);
    } else if (val->type == NEKO_SC_VALUE_ARRAY) {
        for (int j = (int)neko_script_vector_size(val->array) - 1; j >= 0; j--) gc_mark(val->array[j]);
    } else if (val->type == NEKO_SC_VALUE_TUPLE) {
        for (int j = (int)neko_script_vector_size(val->tuple) - 1; j >= 0; j--) gc_mark(val->array[j]);
    } else if (val->type == NEKO_SC_VALUE_DICT) {
        for (int j = (int)neko_script_vector_size(val->dict.values) - 1; j >= 0; j--) gc_mark(val->dict.values[j]);
    } else if (val->type == NEKO_SC_VALUE_FN) {
        // mark function context variables
        gc_markctx(val->fn->ctx);
    }
}

void neko_script_gc_collect() {
    for (size_t i = 0; i < neko_script_vector_size(ctx_stack); i++) {
        neko_script_ctx_t *c = ctx_stack[i];
        gc_markctx(c);
    }

    gc_markctx(*current);

    for (int i = (int)neko_script_vector_size(values) - 1; i >= 0; i--) {
        neko_script_value_t *v = values[i];

        if (v->markbit == 0) {
            neko_script_value_free(v);
            int l = (int)neko_script_vector_size(values) - 1;
            if (i != l) values[i] = values[l];
            neko_script_vector_pop(values);
        } else {
            v->markbit = 0;
        }
    }

    for (int i = (int)neko_script_vector_size(ctxs) - 1; i >= 0; i--) {
        if (ctxs[i]->markbit == 0) {
            neko_script_ctx_free(ctxs[i]);
            int l = (int)neko_script_vector_size(ctxs) - 1;
            if (i != l) ctxs[i] = ctxs[l];
            neko_script_vector_pop(ctxs);
        } else {
            ctxs[i]->markbit = 0;
        }
    }
}

void neko_script_gc_freeall() {
    for (size_t i = 0; i < neko_script_vector_size(ctxs); i++) {
        neko_script_ctx_free(ctxs[i]);
    }
    neko_script_vector_free(ctxs);
    for (size_t i = 0; i < neko_script_vector_size(values); i++) {
        neko_script_value_free(values[i]);
    }
    neko_script_vector_free(values);
    usedmem = 0;
    maxmem = 128;
}

neko_script_value_t *neko_script_value_null() {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_NULL;
    v->constant = 0;
    v->refs = 0;
    v->markbit = 0;
    return v;
}

neko_script_value_t *neko_script_value_number(double val) {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_NUMBER;
    v->constant = 0;
    v->refs = 0;
    v->number = val;
    v->markbit = 0;
    return v;
}

neko_script_value_t *neko_script_value_string(char *val) {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_STRING;
    v->constant = 0;
    v->refs = 0;
    v->string = val;
    v->markbit = 0;
    return v;
}

neko_script_value_t *neko_script_value_array(neko_script_vector(neko_script_value_t *) arr) {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_ARRAY;
    v->constant = 0;
    v->refs = 0;
    v->array = arr;
    v->markbit = 0;
    return v;
}

neko_script_value_t *neko_script_value_tuple(neko_script_vector(neko_script_value_t *) tuple) {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_TUPLE;
    v->constant = 0;
    v->refs = 0;
    v->tuple = tuple;
    v->markbit = 0;
    return v;
}

neko_script_value_t *neko_script_value_dict(neko_script_vector(char *) names, neko_script_vector(neko_script_value_t *) values) {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_DICT;
    v->constant = 0;
    v->refs = 0;
    v->dict.names = names;
    v->dict.values = values;
    v->markbit = 0;
    return v;
}

neko_script_value_t *neko_script_value_fn(neko_script_fn_t *fn) {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_FN;
    v->constant = 0;
    v->refs = 0;
    v->fn = fn;
    v->markbit = 0;
    return v;
}

neko_script_value_t *neko_script_value_ref(neko_script_value_t *val) {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_REF;
    v->refs = 0;
    v->ref = val;
    val->refs++;
    v->markbit = 0;
    return v;
}

neko_script_value_t *neko_script_value_native(ns_userdata_t val) {
    neko_script_value_t *v = neko_script_gc_alloc_value();
    v->type = NEKO_SC_VALUE_NATIVE;
    v->refs = 0;
    v->ref = val;
    v->markbit = 0;
    return v;
}

void neko_script_value_free_members(neko_script_value_t *val) {
    if (val->type == NEKO_SC_VALUE_STRING) {
        ns_free(val->string);
    } else if (val->type == NEKO_SC_VALUE_ARRAY) {
        neko_script_vector_free(val->array);
    } else if (val->type == NEKO_SC_VALUE_TUPLE) {
        neko_script_vector_free(val->tuple);
    } else if (val->type == NEKO_SC_VALUE_DICT) {
        for (size_t i = 0; i < neko_script_vector_size(val->dict.names); i++) ns_free(val->dict.names[i]);
        neko_script_vector_free(val->dict.names);
        neko_script_vector_free(val->dict.values);
    } else if (val->type == NEKO_SC_VALUE_FN) {
        ns_free(val->fn);
    }
}

void neko_script_value_free(neko_script_value_t *val) {
    neko_script_value_free_members(val);
    ns_free(val);
}

void neko_script_value_assign(neko_script_value_t *a, neko_script_value_t *b) {
    while (b->type == NEKO_SC_VALUE_REF) b = b->ref;

    if (a->constant) neko_script_throw("Cannot assign to const value");

    if (a->type == NEKO_SC_VALUE_TUPLE || b->type == NEKO_SC_VALUE_TUPLE) {
        neko_script_vector(neko_script_value_t *) tmp = NULL;
        if (a->type != NEKO_SC_VALUE_TUPLE) {
            // convert a to tuple
            neko_script_vector_push(tmp, a);
            a = neko_script_value_tuple(tmp);
        } else if (b->type != NEKO_SC_VALUE_TUPLE) {
            // convert b to tuple
            neko_script_vector_push(tmp, b);
            b = neko_script_value_tuple(tmp);
        }

        size_t maxb = neko_script_vector_size(b->tuple);
        for (int i = 0; i < neko_script_vector_size(a->tuple); i++) {
            neko_script_value_assign(a->tuple[i], i >= maxb ? neko_script_value_null() : b->tuple[i]);
        }
        return;
    }

    if (a->type == NEKO_SC_VALUE_STRING) ns_free(a->string);

    if (b->type == NEKO_SC_VALUE_ARRAY || b->type == NEKO_SC_VALUE_DICT || b->type == NEKO_SC_VALUE_FN) {
        neko_script_value_free_members(a);
        a->ref = b;
        a->type = NEKO_SC_VALUE_REF;
        b->refs++;
    } else {
        memcpy(a, b, sizeof(neko_script_value_t));
    }

    if (a->type == NEKO_SC_VALUE_STRING) a->string = strdup(b->string);
}

neko_script_value_t *neko_script_value_unary(int op, neko_script_value_t *a) {
    while (a->type == NEKO_SC_VALUE_REF) a = a->ref;

    if (a->type != NEKO_SC_VALUE_NUMBER) neko_script_throw("Cannot perform unary operation on non numbers");

    switch (op) {
        case NEKO_SC_TOKEN_PLUS:
            return neko_script_value_number(a->number);
        case NEKO_SC_TOKEN_MINUS:
            return neko_script_value_number(-a->number);
        case NEKO_SC_TOKEN_EXCLAM:
            return neko_script_value_number(!a->number);
    }
    neko_script_throw("Unkown unary operation %d", op);
    return neko_script_value_null();
}

static neko_script_value_t *binary_number(int op, neko_script_value_t *a, neko_script_value_t *b) {
    switch (op) {
        case NEKO_SC_TOKEN_PLUS:
            return neko_script_value_number(a->number + b->number);
        case NEKO_SC_TOKEN_MINUS:
            return neko_script_value_number(a->number - b->number);
        case NEKO_SC_TOKEN_ASTERISK:
            return neko_script_value_number(a->number * b->number);
        case NEKO_SC_TOKEN_SLASH:
            return neko_script_value_number(a->number / b->number);
        case NEKO_SC_TOKEN_EQUAL:
            return neko_script_value_number(a->number == b->number);
        case NEKO_SC_TOKEN_NOTEQUAL:
            return neko_script_value_number(a->number != b->number);
        case NEKO_SC_TOKEN_LESSEQUAL:
            return neko_script_value_number(a->number <= b->number);
        case NEKO_SC_TOKEN_MOREEQUAL:
            return neko_script_value_number(a->number >= b->number);
        case NEKO_SC_TOKEN_LCHEVR:
            return neko_script_value_number(a->number < b->number);
        case NEKO_SC_TOKEN_RCHEVR:
            return neko_script_value_number(a->number > b->number);
        case NEKO_SC_TOKEN_AND:
            return neko_script_value_number(a->number && b->number);
        case NEKO_SC_TOKEN_OR:
            return neko_script_value_number(a->number || b->number);
    }
    neko_script_throw("Unkown binary operation %d", op);
    return neko_script_value_null();
}

static neko_script_value_t *binary_string(int op, neko_script_value_t *a, neko_script_value_t *b) {
    switch (op) {
        case NEKO_SC_TOKEN_PLUS:;
            int len1 = (int)strlen(a->string);
            int len2 = (int)strlen(b->string);
            char *str = (char *)ns_malloc(sizeof(char) * (len1 + len2 + 1));
            str[0] = 0;
            strcat(str, a->string);
            strcat(str, b->string);
            return neko_script_value_string(str);
        case NEKO_SC_TOKEN_EQUAL:
            return neko_script_value_number(strcmp(a->string, b->string) == 0);
        case NEKO_SC_TOKEN_NOTEQUAL:
            return neko_script_value_number(strcmp(a->string, b->string) != 0);
    }
    neko_script_throw("Unknown string binary operation %d", op);
    return neko_script_value_null();
}

static neko_script_value_t *binary_array(int op, neko_script_value_t *a, neko_script_value_t *b) {
    neko_script_throw("Cannot perform any binary operation on type array");
    return neko_script_value_null();
}

static neko_script_value_t *binary_dict(int op, neko_script_value_t *a, neko_script_value_t *b) {
    neko_script_throw("Cannot perform any binary operation on type dict");
    return neko_script_value_null();
}

neko_script_value_t *neko_script_value_binary(int op, neko_script_value_t *a, neko_script_value_t *b) {
    if (op == NEKO_SC_TOKEN_ASSIGN) {
        neko_script_value_assign(a, b);
        return a;
    }

    while (a->type == NEKO_SC_VALUE_REF) a = a->ref;
    while (b->type == NEKO_SC_VALUE_REF) b = b->ref;

    if (op == NEKO_SC_TOKEN_COMMA) {
        neko_script_vector(neko_script_value_t *) tmp = NULL;
        if (a->type != NEKO_SC_VALUE_TUPLE)
            neko_script_vector_push(tmp, a);
        else
            neko_script_vector_append(tmp, neko_script_vector_size(a->tuple), a->tuple);

        if (b->type != NEKO_SC_VALUE_TUPLE)
            neko_script_vector_push(tmp, b);
        else
            neko_script_vector_append(tmp, neko_script_vector_size(b->tuple), b->tuple);

        return neko_script_value_tuple(tmp);
    }

    if (a->type != b->type) neko_script_throw("Type mismatch %s %s %s", neko_script_tokenstr(op), neko_script_valuetypestr(a->type), neko_script_valuetypestr(b->type));

    switch (a->type) {
        case NEKO_SC_VALUE_NULL:
            neko_script_throw("Cannot perform operation on null");
        case NEKO_SC_VALUE_NUMBER:
            return binary_number(op, a, b);
        case NEKO_SC_VALUE_STRING:
            return binary_string(op, a, b);
        case NEKO_SC_VALUE_ARRAY:
            return binary_string(op, a, b);
        case NEKO_SC_VALUE_DICT:
            return binary_dict(op, a, b);
        case NEKO_SC_VALUE_TUPLE:
        case NEKO_SC_VALUE_FN:
        case NEKO_SC_VALUE_NATIVE:
            break;
    }
    neko_script_throw("Unkown value type");
    return neko_script_value_null();
}

neko_script_value_t *neko_script_value_get(int i, neko_script_value_t *a) {
    while (a->type == NEKO_SC_VALUE_REF) a = a->ref;

    if (a->type == NEKO_SC_VALUE_STRING) {
        int len = (int)strlen(a->string);
        if (i >= len) neko_script_throw("Index out of range");

        char buf[2];
        buf[0] = a->string[i];
        buf[1] = 0;
        return neko_script_value_string(strdup(buf));
    } else if (a->type == NEKO_SC_VALUE_ARRAY) {
        if (i >= neko_script_vector_size(a->array)) neko_script_throw("Index out of range");
        return a->array[i];
    }

    neko_script_throw("Cannot index value of this type");
    return neko_script_value_null();
}

neko_script_value_t *neko_script_value_member(char *name, neko_script_value_t *a) {
    while (a->type == NEKO_SC_VALUE_REF) a = a->ref;

    if (a->type == NEKO_SC_VALUE_DICT) {
        for (size_t i = 0; i < neko_script_vector_size(a->dict.names); i++) {
            if (strcmp(a->dict.names[i], name) == 0) return a->dict.values[i];
        }

        neko_script_vector_push(a->dict.names, strdup(name));
        neko_script_vector_push(a->dict.values, neko_script_value_null());

        return a->dict.values[neko_script_vector_size(a->dict.values) - 1];
    }

    neko_script_throw("Cannot get member for type %s", neko_script_valuetypestr(a->type));
    return neko_script_value_null();
}

static char *getstr(char *opcodes, int *ip) {
    char buffer[512];
    int i = 0;
    while (opcodes[*ip + i] != '\0') {
        buffer[i] = opcodes[*ip + i];
        i++;
    }
    buffer[i] = 0;
    *ip += i + 1;
    return strdup(buffer);
}

static int getint(char *opcodes, int *ip) {
    int value = *(int *)&opcodes[*ip];
    *ip += 4;
    return value;
}

static double getdouble(char *opcodes, int *ip) {
    double value = *(double *)&opcodes[*ip];
    *ip += 8;
    return value;
}

void dis(char *opcodes, long fsize) {
    int ip = 0;
    while (1) {
        if (ip >= fsize) break;
        int byte = opcodes[ip];
        ns_printf("%d %02hhx:\t\t\t\t", ip, byte);
        ip += 1;

        switch (byte) {
            case NEKO_SC_OPCODE_NOP:
                ns_printf("NOP");
                break;
            case NEKO_SC_OPCODE_PUSHI:
                ns_printf("pushi %d", getint(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_PUSHN:
                ns_printf("pushn %f", getdouble(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_PUSHS:
                ns_printf("pushs \"%s\"", getstr(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_PUSHV:
                ns_printf("pushv \"%s\"", getstr(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_POP:
                ns_printf("pop");
                break;
            case NEKO_SC_OPCODE_STOREFN:
                ns_printf("storefn \"%s\"", getstr(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_STORE:
                ns_printf("store \"%s\"", getstr(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_UNARY:
                ns_printf("unary %d", getint(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_BINARY:
                ns_printf("binary %d", getint(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_CALL:
                ns_printf("call");
                break;
            case NEKO_SC_OPCODE_CALLM:
                ns_printf("callm");
                break;
            case NEKO_SC_OPCODE_RETN:
                ns_printf("retn");
                break;
            case NEKO_SC_OPCODE_RET:
                ns_printf("ret");
                break;
            case NEKO_SC_OPCODE_JMP:
                ns_printf("jmp %d", getint(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_BRZ:
                ns_printf("brz %d", getint(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_INDEX:
                ns_printf("index");
                break;
            case NEKO_SC_OPCODE_MEMBER:
                ns_printf("member \"%s\"", getstr(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_MEMBERD:
                ns_printf("memberd \"%s\"", getstr(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_IMPORT:
                ns_printf("import \"%s\"", getstr(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_TRY:
                ns_printf("try %d", getint(opcodes, &ip));
                break;
            case NEKO_SC_OPCODE_ENDTRY:
                ns_printf("endtry");
                break;
            case NEKO_SC_OPCODE_THROW:
                ns_printf("throw");
                break;
            case NEKO_SC_OPCODE_SCOPE:
                ns_printf("scope");
                break;
            case NEKO_SC_OPCODE_ENDSCOPE:
                ns_printf("endscope");
                break;
            default:
                ns_printf("data %x", byte);
                break;
        }

        putchar('\n');
    }

    ip = 0;
}

struct tryptr {
    int addr;
    size_t calls;
    neko_script_ctx_t *ctx;
    size_t ctx_size;
};

typedef struct tryptr tryptr_t;

struct callptr {
    int addr;
    size_t sp;
    neko_script_ctx_t *ctx;
    size_t ctx_size;
    neko_script_binary_t *bin;
};

typedef struct callptr callptr_t;

neko_script_marker_t neko_script_getmarker(neko_script_binary_t *binary, size_t ip) {
    for (size_t i = 0; i < neko_script_vector_size(binary->debug); i++) {
        if (binary->debug[i].addr == ip) return (neko_script_marker_t){0, binary->debug[i].line, binary->debug[i].column};
    }
    return (neko_script_marker_t){0, 0, 0};
}

neko_script_vector(neko_script_ctx_t *) ctx_stack = NULL;
neko_script_ctx_t **current = NULL;

void print_vars(neko_script_ctx_t *ctx) {
    if (ctx == NULL) return;
    putchar('(');
    for (int i = 0; i < neko_script_vector_size(ctx->vars); i++) {
        ns_printf("%s, ", ctx->vars[i]->name);
    }
    print_vars(ctx->parent);
    putchar(')');
}

void neko_script_exec(neko_script_ctx_t *global, neko_script_ctx_t *context, neko_script_binary_t *binary, int ip, neko_script_binary_t *(*load_module)(char *name),
                      void *(trap)(neko_script_ctx_t *ctx)) {
    neko_script_vector(callptr_t) call_stack = NULL;
    neko_script_vector(tryptr_t) try_stack = NULL;

    neko_script_vector(neko_script_ctx_t *) old_ctx_stack = ctx_stack;
    neko_script_ctx_t **old_current = current;

    ctx_stack = NULL;
    int oldip = 0;

    current = &context;

    jmp_buf old_try;  // TODO: Add real recursive try support
    memcpy(old_try, __neko_script_ex_buf__, sizeof(__neko_script_ex_buf__));

    neko_script_vector_push(ctx_stack, global);

#define opcodes binary->block

    neko_script_try {
        while (1) {
            if (ip >= binary->size) break;
            int byte = binary->block[ip];
            oldip = ip;
            ip += 1;

#ifdef NEKO_SC_DEBUG
            if (byte & NEKO_SC_OPCODE_TRAP_MASK)  // for debugging only
            {
                if (trap) trap(context);
                byte &= ~NEKO_SC_OPCODE_TRAP_MASK;
            }
#endif

            switch (byte) {
                case NEKO_SC_OPCODE_NOP:
                    break;
                case NEKO_SC_OPCODE_PUSHN:
                    neko_script_vector_push(global->stack, neko_script_value_number(getdouble(opcodes, &ip)));
                    neko_script_gc_trigger();
                    break;
                case NEKO_SC_OPCODE_PUSHI:
                    neko_script_vector_push(global->stack, neko_script_value_number(getint(opcodes, &ip)));
                    neko_script_gc_trigger();
                    break;
                case NEKO_SC_OPCODE_PUSHS:
                    neko_script_vector_push(global->stack, neko_script_value_string(getstr(opcodes, &ip)));
                    neko_script_gc_trigger();
                    break;
                case NEKO_SC_OPCODE_PUSHV: {
                    char *name = getstr(opcodes, &ip);
                    neko_script_value_t *val = neko_script_ctx_getvar(context, name);
                    if (val == NULL) {
                        neko_script_throw("No variable named '%s'", name);
                    }
                    if (val->type == NEKO_SC_VALUE_ARRAY || val->type == NEKO_SC_VALUE_DICT || val->type == NEKO_SC_VALUE_FN) val = neko_script_value_ref(val);
                    neko_script_vector_push(global->stack, val);
                    neko_script_gc_trigger();
                    ns_free(name);
                    break;
                }
                case NEKO_SC_OPCODE_POP:
                    neko_script_vector_pop(global->stack);
                    break;
                case NEKO_SC_OPCODE_STOREFN: {  // 声明定义函数
                    int adr = (int)neko_script_vector_pop(global->stack)->number;
                    int argc = (int)neko_script_vector_pop(global->stack)->number;
                    neko_script_ctx_addfn(context, binary, getstr(opcodes, &ip), argc, adr, NULL);
                    break;
                }
                case NEKO_SC_OPCODE_STORE: {
                    char *name = getstr(opcodes, &ip);
                    neko_script_ctx_t *parent = context->parent;
                    context->parent = NULL;
                    neko_script_value_t *val = neko_script_ctx_getvar(context, name);
                    context->parent = parent;
                    if (val != NULL) {
                        neko_script_throw("Variable redefinition '%s'", name);
                    }
                    neko_script_ctx_addvar(context, name, neko_script_vector_pop(global->stack));
                    break;
                }
                case NEKO_SC_OPCODE_UNARY: {
                    neko_script_value_t *a = neko_script_vector_pop(global->stack);
                    neko_script_vector_push(global->stack, neko_script_value_unary(getint(opcodes, &ip), a));
                    neko_script_gc_trigger();
                    break;
                }
                case NEKO_SC_OPCODE_BINARY: {
                    neko_script_value_t *b = neko_script_vector_pop(global->stack);
                    neko_script_value_t *a = neko_script_vector_pop(global->stack);
                    neko_script_vector_push(global->stack, neko_script_value_binary(getint(opcodes, &ip), a, b));
                    neko_script_gc_trigger();
                    break;
                }
                case NEKO_SC_OPCODE_CALL:     // 函数调用
                case NEKO_SC_OPCODE_CALLM: {  // 成员函数调用
                    neko_script_value_t *fn_value = neko_script_vector_pop(global->stack);
                    while (fn_value->type == NEKO_SC_VALUE_REF) fn_value = fn_value->ref;
                    if (fn_value->type != NEKO_SC_VALUE_FN) {
                        neko_script_throw("Can only call functions");
                    }
                    neko_script_fn_t *fn = fn_value->fn;
                    neko_script_value_t *parent = NULL;
                    if (byte == NEKO_SC_OPCODE_CALLM) {
                        parent = neko_script_vector_pop(global->stack);
                    }
                    neko_script_value_t *argc = neko_script_vector_pop(global->stack);  // 函数实参数量

                    // 判断调用函数实参数量是否正确
                    if ((int)(argc->number) < fn->argc) {
                        neko_script_throw("To little arguments for function, expect %d got %d", fn->argc, (int)(argc->number));
                    }

                    if (fn->native != NULL) {  // 判断是否为原生绑定函数
                        fn->native((int)(argc->number), global);
                    } else {
                        neko_script_vector_push(global->stack, argc);  // 推入(还原)函数实参数量
                        neko_script_binary_t *fn_binary = fn->func_binary;
                        size_t sp = neko_script_vector_size(global->stack) - 1 - (int)(argc->number);
                        if (binary == fn_binary) {
                            // 同一个模块
                            neko_script_vector_push(call_stack, ((callptr_t){ip, sp, context, neko_script_vector_size(ctx_stack), binary}));
                            neko_script_vector_push(ctx_stack, context);
                            context = neko_script_ctx_new(NULL);
                            context->parent = global;

                            if (byte == NEKO_SC_OPCODE_CALLM) {
                                neko_script_ctx_addvar(context, strdup("this"), parent);  // add "this" variable
                            }
                            ip = fn->address;
                        } else {
                            // 不同的模块
                            // TODO: Fix try .. catch on external modules
                            neko_script_vector_push(call_stack, ((callptr_t){ip, sp, context, neko_script_vector_size(ctx_stack), binary}));
                            neko_script_vector_push(ctx_stack, context);
                            context = neko_script_ctx_new(NULL);
                            context->parent = fn->ctx;

                            if (byte == NEKO_SC_OPCODE_CALLM) {
                                neko_script_ctx_addvar(context, strdup("this"), parent);  // add "this" variable
                            }
                            binary = fn_binary;
                            ip = fn->address;
                        }
                    }
                } break;
                case NEKO_SC_OPCODE_RETN:
                    neko_script_vector_push(global->stack, neko_script_value_null());
                case NEKO_SC_OPCODE_RET:
                    if (neko_script_vector_size(call_stack) == 0) {
                        // nothing to return
                        goto end;
                    }
                    callptr_t cp = neko_script_vector_pop(call_stack);
                    neko_script_value_t *val = neko_script_vector_pop(global->stack);
                    neko_script_vector_shrinkto(global->stack, cp.sp);
                    neko_script_vector_shrinkto(ctx_stack, cp.ctx_size);
                    neko_script_vector_push(global->stack, val);
                    binary = cp.bin;
                    ip = cp.addr;
                    context->parent = NULL;
                    context = cp.ctx;
                    break;
                case NEKO_SC_OPCODE_JMP:
                    ip = getint(opcodes, &ip);
                    break;
                case NEKO_SC_OPCODE_BRZ: {
                    neko_script_value_t *v = neko_script_vector_pop(global->stack);
                    int nip = getint(opcodes, &ip);
                    if (v->number == 0) ip = nip;
                } break;
                case NEKO_SC_OPCODE_INDEX: {
                    neko_script_value_t *expr = neko_script_vector_pop(global->stack);
                    neko_script_value_t *var = neko_script_vector_pop(global->stack);
                    neko_script_vector_push(global->stack, neko_script_value_get((int)expr->number, var));
                    break;
                }
                case NEKO_SC_OPCODE_MEMBER: {
                    neko_script_value_t *var = neko_script_vector_pop(global->stack);
                    char *name = getstr(opcodes, &ip);
                    neko_script_vector_push(global->stack, neko_script_value_member(name, var));
                    ns_free(name);
                    break;
                }
                case NEKO_SC_OPCODE_MEMBERD: {
                    neko_script_value_t *var = neko_script_vector_pop(global->stack);
                    char *name = getstr(opcodes, &ip);
                    neko_script_vector_push(global->stack, var);
                    neko_script_vector_push(global->stack, neko_script_value_member(name, var));
                    ns_free(name);
                    break;
                }
                case NEKO_SC_OPCODE_IMPORT: {
                    // eval module
                    char *name = getstr(opcodes, &ip);
                    if (load_module == NULL) neko_script_throw("Module loading not supported");

                    // TODO: cache loaded module and it's context
                    neko_script_binary_t *module = load_module(name);
                    if (module == NULL) neko_script_throw("Cannot find %s module", name);

                    neko_script_ctx_t *module_ctx = neko_script_ctx_new(NULL);
                    module_ctx->parent = global;
                    neko_script_exec(module_ctx, module_ctx, module, 0, load_module, trap);

                    // load it's context into dictionary value
                    neko_script_vector(char *) names = NULL;
                    neko_script_vector(neko_script_value_t *) values = NULL;
                    for (int i = 0; i < neko_script_vector_size(module_ctx->vars); i++) {
                        char *vname = strdup(module_ctx->vars[i]->name);
                        neko_script_value_t *value = module_ctx->vars[i]->val;
                        neko_script_vector_push(names, vname);
                        neko_script_vector_push(values, value);
                    }
                    neko_script_value_t *module_dict = neko_script_value_dict(names, values);

                    // assign this value to variable withing current context
                    neko_script_ctx_addvar(context, name, module_dict);
                    break;
                }
                case NEKO_SC_OPCODE_TRY: {
                    // push adress onto try stack
                    int adr = getint(opcodes, &ip);
                    neko_script_vector_push(try_stack, ((tryptr_t){adr, neko_script_vector_size(call_stack), context, neko_script_vector_size(ctx_stack)}));
                    break;
                }
                case NEKO_SC_OPCODE_ENDTRY: {
                    // pop adress from try stack
                    if (neko_script_vector_size(try_stack) == 0) neko_script_throw("No try to end!");

                    neko_script_vector_pop(try_stack);
                    break;
                }
                case NEKO_SC_OPCODE_THROW: {
                    if (neko_script_vector_size(try_stack) == 0) {
                        if (trap) {
                            trap(context);
                        } else {
                            neko_script_throw("Exception not handled!");
                        }
                    } else {
                        tryptr_t t = neko_script_vector_pop(try_stack);
                        ip = t.addr;
                        context = t.ctx;
                        neko_script_vector_shrinkto(call_stack, t.calls);
                        neko_script_vector_shrinkto(ctx_stack, t.ctx_size);
                        // TODO: Free orphan context
                        // TODO: Unwind stack
                    }

                    break;
                }
                case NEKO_SC_OPCODE_SCOPE:
                    neko_script_vector_push(ctx_stack, context);
                    context = neko_script_ctx_new(context);
                    break;
                case NEKO_SC_OPCODE_ENDSCOPE:
                    context->parent = NULL;
                    context = neko_script_vector_pop(ctx_stack);
                    break;
            }
        }
    }
    neko_script_catch {
        memcpy(__neko_script_ex_buf__, old_try, sizeof(__neko_script_ex_buf__));
        char *msg = strdup(neko_script_ex_msg);

        for (int i = 0; i < neko_script_vector_size(call_stack); i++) {
            neko_script_marker_t marker = neko_script_getmarker(binary, call_stack[i].addr);
            ns_printf("callstack: %zd\n", marker.line);
        }
        neko_script_marker_t marker = neko_script_getmarker(binary, oldip);
        neko_script_throw("%s at line %d", msg, marker.line);
        ns_free(msg);  // 这应该写在 neko_script_throw 里面
    }
end:
    neko_script_vector_free(call_stack);
    neko_script_vector_free(try_stack);
    neko_script_vector_free(ctx_stack);

    ctx_stack = old_ctx_stack;
    current = old_current;

#undef opcodes
}

// VM END

neko_script_binary_t *neko_script_compile_str(const char *code) {
    neko_script_try {
        neko_script_parser_t *parser = neko_script_parser_new(code);
        neko_script_node_t *tree = neko_script_parse(parser);
        neko_script_parser_free(parser);

        neko_script_binary_t *bin = neko_script_binary_new();
        tree->codegen(tree, bin);
        neko_script_node_free(tree);
        neko_script_bytecode_fill(bin);

        return bin;
    }
    neko_script_catch { return NULL; }
}

int neko_script_eval_str(neko_script_ctx_t *ctx, char *code, neko_script_binary_t *(*load_module)(char *name), void *(trap)(neko_script_ctx_t *ctx)) {
    neko_script_binary_t *bin = neko_script_compile_str(code);
    if (!bin) return 0;
    neko_script_try {
        neko_script_exec(ctx, ctx, bin, 0, load_module, trap);
        neko_script_binary_free(bin);
    }
    neko_script_catch { return 0; }
    return 1;
}

neko_script_binary_t *neko_script_compile_file(const char *filename) {
    FILE *fd = fopen(filename, "r");
    if (!fd) {
        sprintf(neko_script_ex_msg, "failed to open file %s", filename);
        return NULL;
    }

    fseek(fd, 0, SEEK_END);
    long fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    char *string = (char *)ns_malloc(fsize + 1);
    size_t r = fread(string, 1, fsize, fd);
    fclose(fd);

    string[r] = 0;

    neko_script_binary_t *bin = neko_script_compile_str(string);

    ns_free(string);

    return bin;
}

neko_script_binary_t *neko_script_eval_file(neko_script_ctx_t *ctx, const_str filename, neko_script_binary_t *(*load_module)(char *name), void *(trap)(neko_script_ctx_t *ctx), bool do_free) {
    neko_script_binary_t *bin = neko_script_compile_file(filename);
    if (!bin) return NULL;

    neko_script_try {
        neko_script_exec(ctx, ctx, bin, 0, load_module, trap);
        if (do_free) neko_script_binary_free(bin);
    }
    neko_script_catch { return NULL; }
    return bin;
}

int neko_script_dis_str(neko_script_ctx_t *ctx, char *code, neko_script_binary_t *(*load_module)(char *name), void *(trap)(neko_script_ctx_t *ctx)) {
    neko_script_try {
        neko_script_parser_t *parser = neko_script_parser_new(code);
        neko_script_node_t *tree = neko_script_parse(parser);
        neko_script_parser_free(parser);

        neko_script_binary_t *bin = neko_script_binary_new();
        tree->codegen(tree, bin);
        neko_script_node_free(tree);

        neko_script_bytecode_fill(bin);

        for (int i = 0; i < bin->size; i++) {
            ns_printf("%02hhx ", bin->block[i]);
        }
        ns_printf("\n");
        dis(bin->block, bin->size);
        neko_script_binary_free(bin);
    }
    neko_script_catch { return 0; }
    return 1;
}

#ifdef NEKO_SEXPR_IMPL

s32 neko_sexpr_node_fits_format(neko_snode_t *node, neko_snode_t *fmt) {
    if (!node) return 0;

    if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        for (s32 i = node->node_len && node->node->type == NEKO_SEXPR_TYPE_STRING; i < node->node_len; i++) {
            if (node->node[i].type == NEKO_SEXPR_TYPE_ARRAY && node->node[i].node_len && node->node[i].node->type == NEKO_SEXPR_TYPE_STRING) {
                if (!neko_sexpr_node_get_tagged(fmt, node->node[i].node->str)) return 0;
            } else {
                neko_snode_t *res = neko_sexpr_node_get_index(fmt, i);
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

neko_snode_t *neko_sexpr_node_get_value(neko_snode_t *node, s32 *len) {
    if (!node) return NULL;
    if (len) *len = 1;
    if (node->type != NEKO_SEXPR_TYPE_ARRAY) return node;

    if (!node->node || node->node_len < 2 || node->node->type != NEKO_SEXPR_TYPE_STRING) return node->node;
    if (len) *len = node->node_len - 1;
    return node->node + 1;
}

neko_snode_t *neko_sexpr_node_get_tagged(neko_snode_t *node, const_str tag) {
    if (!node || node->type != NEKO_SEXPR_TYPE_ARRAY || !node->node) return NULL;

    for (s32 i = 0; i < node->node_len; i++)
        if (node->node[i].type == NEKO_SEXPR_TYPE_ARRAY && node->node[i].node_len && node->node[i].node->type == NEKO_SEXPR_TYPE_STRING) {
            if (strcmp(tag, node->node[i].node->str) == 0) return node->node + i;
        }
    return NULL;
}

neko_snode_t *neko_sexpr_node_get_index(neko_snode_t *node, s32 index) {
    if (!node || node->type != NEKO_SEXPR_TYPE_ARRAY || !node->node || index < 0 || index >= node->node_len) return NULL;
    return node->node + index;
}

static void neko_sexpr_write_nde(FILE *fd, const neko_snode_t *node, s32 root, s32 indents, s32 new_line_start, s32 first) {
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

s32 neko_sexpr_write_to_file(char *filename, const neko_snode_t node) {
    if (node.type != NEKO_SEXPR_TYPE_ARRAY) return 0;

    FILE *fd = fopen(filename, "wb");
    if (!fd) {
        neko_log_warning("neko S-expression error: unable to open file");
        return 0;
    }

    for (s32 i = 0; i < node.node_len; i++) neko_sexpr_write_nde(fd, node.node + i, 1, 0, i > 0, 1);

    fclose(fd);
    return 1;
}

neko_snode_t neko_sexpr_parse_file(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        neko_log_warning("neko S-expression error: unable to open file");
        return (neko_snode_t){0};
    }

    fseek(file, 0L, SEEK_END);
    s32 readsize = ftell(file);
    rewind(file);

    char *buffer = neko_safe_malloc(readsize);
    if (!buffer) return (neko_snode_t){0};

    fread(buffer, 1, readsize, file);
    fclose(file);

    return neko_sexpr_parse_memory(buffer, readsize);
}

static s32 neko_sexpr_seek_whitespace(char *mem, s32 index, s32 sz) {
    while (index < sz && !memchr("\n '\t()\v\r", mem[index], sizeof("\n '\t()\v\r"))) index++;
    return index;
}

static s32 neko_sexpr_parse_memory_array(char *mem, s32 sz, neko_snode_t *node) {
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
            char *end;
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

neko_snode_t neko_sexpr_parse_memory(char *mem, s32 sz) {
    s32 index = 0;
    neko_snode_t node = {.type = NEKO_SEXPR_TYPE_ARRAY};
    node.node_len = 0;
    while (index < sz) {
        char *new_pos = memchr(mem + index, '(', sz - index);
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

void neko_sexpr_free_node(neko_snode_t *node) {

    if (node->type == NEKO_SEXPR_TYPE_STRING && node->is_str_allocated) {
        neko_safe_free(node->str);
    } else if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        for (s32 i = 0; i < node->node_len; i++) neko_sexpr_free_node(node->node + i);
        if (node->is_node_allocated) neko_safe_free(node->node);
    }
}

neko_snode_t neko_sexpr_dup_node(neko_snode_t *node, s32 free_old_node) {
    if (node->type == NEKO_SEXPR_TYPE_STRING) {
        char *str = strdup(node->str);
        if (free_old_node && node->is_str_allocated) neko_safe_free(node->str);
        node->is_str_allocated = 1;
        node->str = str;
    } else if (node->type == NEKO_SEXPR_TYPE_ARRAY) {
        neko_snode_t *n = neko_safe_malloc(node->node_len * sizeof(*n));
        memcpy(n, node->node, node->node_len * sizeof(*n));
        if (free_old_node && node->is_node_allocated) neko_safe_free(node->node);
        node->node = n;
        node->is_node_allocated = 1;
        for (s32 i = 0; i < node->node_len; i++) neko_sexpr_dup_node(node->node + i, free_old_node);
    }
    return *node;
}

#endif
