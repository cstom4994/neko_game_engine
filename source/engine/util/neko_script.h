#ifndef NEKO_SCRIPT_H
#define NEKO_SCRIPT_H

#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// 用于脚本编译期外部测试
#if defined(NO_NEKO_ENGINE)

#ifdef NEKO_API_DLL_EXPORT
#ifdef __cplusplus
#define NEKO_API_EXTERN extern "C" __declspec(dllexport)
#else
#define NEKO_API_EXTERN extern __declspec(dllexport)
#endif
#else
#ifdef __cplusplus
#define NEKO_API_EXTERN extern "C"
#define NEKO_CPP_SRC
#else
#define NEKO_API_EXTERN extern
#endif
#endif

#define NEKO_API_DECL NEKO_API_EXTERN
#define NEKO_API_PRIVATE NEKO_API_EXTERN

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef s32 b32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef const char *const_str;

#else
#include "engine/neko.h"  // 否则引用 neko 头
#endif

typedef struct neko_script_vector_s {
    size_t allocated;
    size_t used;
} neko_script_vector_t;

#define neko_script_vector(x) x *

#define neko_script_vector_try_grow(VECTOR, MORE)                                                                  \
    (((!(VECTOR) || neko_script_vector_meta(VECTOR)->used + (MORE) >= neko_script_vector_meta(VECTOR)->allocated)) \
             ? (void)neko_script_vector_grow_impl(((void **)&(VECTOR)), (MORE), sizeof(*(VECTOR)))                 \
             : (void)0)

#define neko_script_vector_meta(VECTOR) ((neko_script_vector_t *)(((unsigned char *)(VECTOR)) - sizeof(neko_script_vector_t)))

#define neko_script_vector_free(VECTOR) ((void)((VECTOR) ? (vec_delete((void *)(VECTOR)), (VECTOR) = NULL) : 0))

#define neko_script_vector_push(VECTOR, VALUE) (neko_script_vector_try_grow((VECTOR), 1), (VECTOR)[neko_script_vector_meta(VECTOR)->used] = (VALUE), neko_script_vector_meta(VECTOR)->used++)

#define neko_script_vector_size(VECTOR) ((VECTOR) ? neko_script_vector_meta(VECTOR)->used : 0)

#define neko_script_vector_capacity(VECTOR) ((VECTOR) ? neko_script_vector_meta(VECTOR)->allocated : 0)

#define neko_script_vector_resize(VECTOR, SIZE) \
    (neko_script_vector_try_grow((VECTOR), (SIZE)), neko_script_vector_meta(VECTOR)->used += (SIZE), &(VECTOR)[neko_script_vector_meta(VECTOR)->used - (SIZE)])

#define neko_script_vector_last(VECTOR) ((VECTOR)[neko_script_vector_meta(VECTOR)->used - 1])

#define neko_script_vector_lastn(VECTOR, N) (&(VECTOR)[neko_script_vector_meta(VECTOR)->used - N])

#define neko_script_vector_pop(VECTOR) ((VECTOR)[--neko_script_vector_meta(VECTOR)->used])

#define neko_script_vector_shrinkto(VECTOR, SIZE) ((void)(neko_script_vector_meta(VECTOR)->used = (SIZE)))

#define neko_script_vector_shrinkby(VECTOR, AMOUNT) ((void)(neko_script_vector_meta(VECTOR)->used -= (AMOUNT)))

#define neko_script_vector_append(VECTOR, COUNT, POINTER) ((void)(memcpy(neko_script_vector_resize((VECTOR), (COUNT)), (POINTER), (COUNT) * sizeof(*(POINTER)))))

#define neko_script_vector_remove(VECTOR, INDEX, COUNT)                                                                                                 \
    ((void)(memmove((VECTOR) + (INDEX), (VECTOR) + (INDEX) + (COUNT), sizeof(*(VECTOR)) * (neko_script_vector_meta(VECTOR)->used - (INDEX) - (COUNT))), \
            neko_script_vector_meta(VECTOR)->used -= (COUNT)))

#define vec_delete(VECTOR) free(neko_script_vector_meta(VECTOR))

NEKO_API_DECL void neko_script_vector_grow_impl(void **vector, size_t more, size_t type_size);

enum NEKO_SC_NODETYPE {
    NEKO_SC_NODETYPE_ROOT,
    NEKO_SC_NODETYPE_IDENT,
    NEKO_SC_NODETYPE_UNARY,
    NEKO_SC_NODETYPE_BINARY,
    NEKO_SC_NODETYPE_NUMBER,
    NEKO_SC_NODETYPE_STRING,
    NEKO_SC_NODETYPE_CALL,
    NEKO_SC_NODETYPE_FUNC,
    NEKO_SC_NODETYPE_RETURN,
    NEKO_SC_NODETYPE_COND,
    NEKO_SC_NODETYPE_LOOP,
    NEKO_SC_NODETYPE_BREAK,
    NEKO_SC_NODETYPE_DECL,
    NEKO_SC_NODETYPE_INDEX,
    NEKO_SC_NODETYPE_BLOCK,
    NEKO_SC_NODETYPE_MEMBER,
    NEKO_SC_NODETYPE_IMPORT,
    NEKO_SC_NODETYPE_CLASS,
    NEKO_SC_NODETYPE_TRYCATCH,
    NEKO_SC_NODETYPE_THROW,
};

struct neko_script_marker_s {
    size_t index;
    size_t line;
    size_t column;
};

typedef struct neko_script_marker_s neko_script_marker_t;

struct neko_script_binary_s;

struct neko_script_node_s {
    enum NEKO_SC_NODETYPE type;
    neko_script_marker_t marker;
    void (*codegen)(struct neko_script_node_s *, struct neko_script_binary_s *binary);
    void (*free)(struct neko_script_node_s *);
    union {
        struct {
            struct neko_script_node_s *funcs;
            struct neko_script_node_s *stmts;
        } root;
        char *ident;
        struct {
            int op;
            struct neko_script_node_s *val;
        } unary;
        struct {
            int op;
            struct neko_script_node_s *a;
            struct neko_script_node_s *b;
        } binary;
        double number;
        char *string;
        struct {
            struct neko_script_node_s *func;
            struct neko_script_node_s *args;
        } call;
        struct {
            char *name;
            neko_script_vector(char *) args;
            struct neko_script_node_s *body;
        } func;
        struct neko_script_node_s *ret;
        struct {
            struct neko_script_node_s *arg;
            struct neko_script_node_s *body;
            struct neko_script_node_s *elsebody;
        } cond;
        struct {
            struct neko_script_node_s *arg;
            struct neko_script_node_s *body;
        } loop;
        struct {
            neko_script_vector(char *) names;
            struct neko_script_node_s *value;
        } decl;
        struct {
            struct neko_script_node_s *var;
            struct neko_script_node_s *expr;
        } index;
        neko_script_vector(struct neko_script_node_s *) block;
        struct {
            struct neko_script_node_s *parent;
            char *name;
        } member;
        char *import;
        struct {
            char *name;
            neko_script_vector(struct neko_script_node_s *) methods;
        } class_t;
        struct {
            struct neko_script_node_s *tryblock;
            struct neko_script_node_s *catchblock;
        } trycatch;
        struct neko_script_node_s *throw_t;
    };
};

typedef struct neko_script_node_s neko_script_node_t;

struct neko_script_value_s;

struct neko_script_var_s {
    char *name;
    struct neko_script_value_s *val;
};

typedef struct neko_script_var_s neko_script_var_t;

struct neko_script_ctx_s;

struct neko_script_fn_s {
    int address;
    int argc;
    struct neko_script_binary_s *func_binary;  // 带function代码的二进制
    struct neko_script_ctx_s *ctx;
    void (*native)(int, struct neko_script_ctx_s *);  // 如果不为 NULL 则调用
};

typedef struct neko_script_fn_s neko_script_fn_t;

struct neko_script_ctx_s {
    struct neko_script_ctx_s *parent;
    neko_script_vector(struct neko_script_var_s *) vars;
    neko_script_vector(struct neko_script_value_s *) stack;
    int markbit;
};

typedef struct neko_script_ctx_s neko_script_ctx_t;

enum NEKO_SC_VALUE {
    NEKO_SC_VALUE_NULL,
    NEKO_SC_VALUE_NUMBER,
    NEKO_SC_VALUE_STRING,
    NEKO_SC_VALUE_ARRAY,
    NEKO_SC_VALUE_TUPLE,
    NEKO_SC_VALUE_DICT,
    NEKO_SC_VALUE_FN,
    NEKO_SC_VALUE_REF,
    NEKO_SC_VALUE_NATIVE,
};

struct neko_script_fn_s;

inline const_str neko_script_valuetypestr(int token) {
    if (token < 0 || token > NEKO_SC_VALUE_REF) return "WRONG VALUE!";
    const char *names[] = {
            "NEKO_SC_VALUE_NULL",    //
            "NEKO_SC_VALUE_NUMBER",  //
            "NEKO_SC_VALUE_STRING",  //
            "NEKO_SC_VALUE_ARRAY",   //
            "NEKO_SC_VALUE_DICT",    //
            "NEKO_SC_VALUE_FN",      //
            "NEKO_SC_VALUE_REF"      //
    };
    return names[token];
}

struct neko_script_value_s {
    enum NEKO_SC_VALUE type;
    int constant;
    int refs;
    int markbit;
    union {
        double number;
        char *string;
        neko_script_vector(struct neko_script_value_s *) array;
        neko_script_vector(struct neko_script_value_s *) tuple;
        struct {
            neko_script_vector(char *) names;
            neko_script_vector(struct neko_script_value_s *) values;
        } dict;
        struct neko_script_fn_s *fn;
        struct neko_script_value_s *ref;  // 存储引用 同时可以用来存储 native userdata (void*)
    };
};

typedef struct neko_script_value_s neko_script_value_t;

#define neko_script_node_free(node) ((node)->free((node)))

NEKO_API_DECL neko_script_node_t *node_root(neko_script_marker_t marker, neko_script_node_t *funcs, neko_script_node_t *stmts);
NEKO_API_DECL neko_script_node_t *node_ident(neko_script_marker_t marker, char *name);
NEKO_API_DECL neko_script_node_t *node_unary(neko_script_marker_t marker, int op, neko_script_node_t *val);
NEKO_API_DECL neko_script_node_t *node_binary(neko_script_marker_t marker, int op, neko_script_node_t *a, neko_script_node_t *b);
NEKO_API_DECL neko_script_node_t *node_number(neko_script_marker_t marker, double number);
NEKO_API_DECL neko_script_node_t *node_string(neko_script_marker_t marker, char *string);
NEKO_API_DECL neko_script_node_t *node_call(neko_script_marker_t marker, neko_script_node_t *func, neko_script_node_t *args);
NEKO_API_DECL neko_script_node_t *node_func(neko_script_marker_t marker, char *name, neko_script_vector(char *) args, neko_script_node_t *body);
NEKO_API_DECL neko_script_node_t *node_return(neko_script_marker_t marker, neko_script_node_t *expr);
NEKO_API_DECL neko_script_node_t *node_cond(neko_script_marker_t marker, neko_script_node_t *arg, neko_script_node_t *body, neko_script_node_t *elsebody);
NEKO_API_DECL neko_script_node_t *node_loop(neko_script_marker_t marker, neko_script_node_t *arg, neko_script_node_t *body);
NEKO_API_DECL neko_script_node_t *node_break(neko_script_marker_t marker);
NEKO_API_DECL neko_script_node_t *node_decl(neko_script_marker_t marker, neko_script_vector(char *) names, neko_script_node_t *value);
NEKO_API_DECL neko_script_node_t *node_index(neko_script_marker_t marker, neko_script_node_t *var, neko_script_node_t *expr);
NEKO_API_DECL neko_script_node_t *node_block(neko_script_marker_t marker, neko_script_vector(neko_script_node_t *) list);
NEKO_API_DECL neko_script_node_t *node_member(neko_script_marker_t marker, neko_script_node_t *parent, char *name);
NEKO_API_DECL neko_script_node_t *node_import(neko_script_marker_t marker, char *name);
NEKO_API_DECL neko_script_node_t *node_class(neko_script_marker_t marker, char *name, neko_script_vector(neko_script_node_t *) list);
NEKO_API_DECL neko_script_node_t *node_trycatch(neko_script_marker_t marker, neko_script_node_t *tryblock, neko_script_node_t *catchblock);
NEKO_API_DECL neko_script_node_t *node_throw(neko_script_marker_t marker, neko_script_node_t *expr);

NEKO_API_DECL void neko_script_node_print(neko_script_node_t *node, int ind);

NEKO_API_DECL void neko_script_builtin_install(neko_script_ctx_t *ctx);

enum NEKO_SC_OPCODE {
    NEKO_SC_OPCODE_NOP,
    NEKO_SC_OPCODE_PUSHN,  // push number
    NEKO_SC_OPCODE_PUSHI,  // push integer
    NEKO_SC_OPCODE_PUSHS,  // push string
    NEKO_SC_OPCODE_PUSHV,  // push value
    NEKO_SC_OPCODE_POP,    // remove element from stack
    NEKO_SC_OPCODE_STORE,
    NEKO_SC_OPCODE_STOREFN,  // store function
    NEKO_SC_OPCODE_UNARY,
    NEKO_SC_OPCODE_BINARY,
    NEKO_SC_OPCODE_CALL,
    NEKO_SC_OPCODE_CALLM,  // call member
    NEKO_SC_OPCODE_RET,
    NEKO_SC_OPCODE_RETN,  // return null
    NEKO_SC_OPCODE_JMP,
    NEKO_SC_OPCODE_BRZ,  // brach if zero
    NEKO_SC_OPCODE_INDEX,
    NEKO_SC_OPCODE_MEMBER,
    NEKO_SC_OPCODE_MEMBERD,  // member with parent duplicate
    NEKO_SC_OPCODE_IMPORT,
    NEKO_SC_OPCODE_TRY,     // try block start
    NEKO_SC_OPCODE_ENDTRY,  // try block end
    NEKO_SC_OPCODE_THROW,
    NEKO_SC_OPCODE_SCOPE,
    NEKO_SC_OPCODE_ENDSCOPE
};

#ifdef NEKO_SC_DEBUG

#define NEKO_SC_OPCODE_TRAP_MASK 128
#define neko_script_settrap(bin, ip) func_binary->block[ip] |= NEKO_SC_OPCODE_TRAP_MASK;
#define neko_script_cleartrap(bin, ip) func_binary->block[ip] &= ~NEKO_SC_OPCODE_TRAP_MASK;

#endif

struct neko_script_debug_symbol_s {
    size_t addr;
    size_t line;
    size_t column;
};

typedef struct neko_script_debug_symbol_s neko_script_debug_symbol_t;

// 以二进制格式存储字节码的结构
struct neko_script_binary_s {
    neko_script_vector(int) adresses;
    neko_script_vector(char *) symbols;
    neko_script_vector(int) fadresses;
    neko_script_vector(char *) fsymbols;
    neko_script_vector(neko_script_debug_symbol_t) debug;
    char *block;
    int size;
    int index;  // free 标签索引
    int loop;   // 保存当前循环索引用于 break
};

typedef struct neko_script_binary_s neko_script_binary_t;

NEKO_API_DECL neko_script_binary_t *neko_script_binary_new();
NEKO_API_DECL void neko_script_binary_free(neko_script_binary_t *bin);
NEKO_API_DECL void neko_script_binary_save(neko_script_binary_t *bin, char *filename);

NEKO_API_DECL int neko_script_bytecode_emit(neko_script_binary_t *bin, int opcode);
NEKO_API_DECL int neko_script_bytecode_emitstr(neko_script_binary_t *bin, int opcode, char *string);
NEKO_API_DECL int neko_script_bytecode_emitint(neko_script_binary_t *bin, int opcode, int number);
NEKO_API_DECL int neko_script_bytecode_emitdouble(neko_script_binary_t *bin, int opcode, double number);

NEKO_API_DECL int neko_script_bytecode_addlabel(neko_script_binary_t *bin, char *name, int adress);
NEKO_API_DECL int neko_script_bytecode_addtofill(neko_script_binary_t *bin, char *name, int adress);
NEKO_API_DECL int neko_script_bytecode_fill(neko_script_binary_t *bin);
NEKO_API_DECL void neko_script_bytecode_adddebug(neko_script_binary_t *bin, neko_script_marker_t marker);

NEKO_API_DECL void neko_script_codegen_root(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_ident(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_unary(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_binary(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_double(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_string(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_call(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_func(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_return(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_cond(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_loop(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_break(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_decl(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_index(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_block(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_member(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_import(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_class(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_trycatch(neko_script_node_t *node, neko_script_binary_t *binary);
NEKO_API_DECL void neko_script_codegen_throw(neko_script_node_t *node, neko_script_binary_t *binary);

NEKO_API_DECL neko_script_ctx_t *neko_script_ctx_new(neko_script_ctx_t *parent);
NEKO_API_DECL void neko_script_ctx_free(neko_script_ctx_t *ctx);
NEKO_API_DECL struct neko_script_value_s *neko_script_ctx_getvar(neko_script_ctx_t *ctx, char *name);
NEKO_API_DECL void neko_script_ctx_addvar(neko_script_ctx_t *ctx, char *name, neko_script_value_t *val);
NEKO_API_DECL neko_script_fn_t *neko_script_ctx_getfn(neko_script_ctx_t *ctx, char *name);
NEKO_API_DECL void neko_script_ctx_addfn(neko_script_ctx_t *ctx, neko_script_binary_t *binary, char *name, int argc, int address, void (*fn)(int, neko_script_ctx_t *));

#define NEKO_SC_ALLOC(type) (type *)neko_script_safe_alloc(sizeof(type))

NEKO_API_DECL void *neko_script_safe_alloc(int size);
NEKO_API_DECL struct neko_script_value_s *neko_script_gc_alloc_value();
NEKO_API_DECL struct neko_script_ctx_s *neko_script_gc_alloc_ctx();
NEKO_API_DECL void neko_script_gc_collect();
NEKO_API_DECL void neko_script_gc_freeall();
NEKO_API_DECL void neko_script_gc_trigger();
NEKO_API_DECL void neko_script_gc_count(u64 *max, u64 *used);

enum NEKO_SC_TOKEN {
    NEKO_SC_TOKEN_IDENT,
    NEKO_SC_TOKEN_NUMBER,
    NEKO_SC_TOKEN_STRING,
    NEKO_SC_TOKEN_FSTRING,

    // keyword
    NEKO_SC_TOKEN_FN,
    NEKO_SC_TOKEN_RETURN,
    NEKO_SC_TOKEN_LET,
    NEKO_SC_TOKEN_IF,
    NEKO_SC_TOKEN_ELSE,
    NEKO_SC_TOKEN_WHILE,
    NEKO_SC_TOKEN_BREAK,
    NEKO_SC_TOKEN_IMPORT,
    NEKO_SC_TOKEN_CLASS,
    NEKO_SC_TOKEN_TRY,
    NEKO_SC_TOKEN_CATCH,
    NEKO_SC_TOKEN_THROW,

    NEKO_SC_TOKEN_COLON,      // :
    NEKO_SC_TOKEN_SEMICOLON,  // ;
    NEKO_SC_TOKEN_DOT,        // .
    NEKO_SC_TOKEN_COMMA,      // ,

    NEKO_SC_TOKEN_PLUS,      // +
    NEKO_SC_TOKEN_MINUS,     // -
    NEKO_SC_TOKEN_SLASH,     // /
    NEKO_SC_TOKEN_ASTERISK,  // *

    NEKO_SC_TOKEN_ASSIGN,     // =
    NEKO_SC_TOKEN_EQUAL,      // ==
    NEKO_SC_TOKEN_NOTEQUAL,   // !=
    NEKO_SC_TOKEN_LESSEQUAL,  // <=
    NEKO_SC_TOKEN_MOREEQUAL,  // >=
    NEKO_SC_TOKEN_LCHEVR,     // <
    NEKO_SC_TOKEN_RCHEVR,     // >
    NEKO_SC_TOKEN_AND,        // &&
    NEKO_SC_TOKEN_OR,         // ||

    NEKO_SC_TOKEN_LPAREN,  // (
    NEKO_SC_TOKEN_RPAREN,  // )
    NEKO_SC_TOKEN_LBRACE,  // {
    NEKO_SC_TOKEN_RBRACE,  // }
    NEKO_SC_TOKEN_LBRACK,  // [
    NEKO_SC_TOKEN_RBRACK,  // ]
    NEKO_SC_TOKEN_EXCLAM,  // !

    NEKO_SC_TOKEN_EOF,
    NEKO_SC_TOKEN_UNKOWN
};

struct neko_script_lexer_s {
    char *input;
    char *buffer;
    double number;
    int lastchar;
    neko_script_marker_t startmarker;
    neko_script_marker_t marker;
};

typedef struct neko_script_lexer_s neko_script_lexer_t;

NEKO_API_DECL neko_script_lexer_t *neko_script_lexer_new(char *input);
NEKO_API_DECL void neko_script_lexer_free(neko_script_lexer_t *lexer);

NEKO_API_DECL int neko_script_gettoken(neko_script_lexer_t *lexer);
NEKO_API_DECL char *neko_script_tokenstr(int token);

struct neko_script_parser_s {
    neko_script_lexer_t *lexer;
    int lasttoken;
};

typedef struct neko_script_parser_s neko_script_parser_t;

typedef void *ns_userdata_t;

NEKO_API_DECL neko_script_parser_t *neko_script_parser_new(char *input);
NEKO_API_DECL void neko_script_parser_free(neko_script_parser_t *parser);

NEKO_API_DECL neko_script_node_t *neko_script_parse(neko_script_parser_t *parser);

NEKO_API_DECL jmp_buf __neko_script_ex_buf__;
NEKO_API_DECL char neko_script_ex_msg[256];

#define neko_script_try if (!setjmp(__neko_script_ex_buf__))
#define neko_script_catch else

NEKO_API_DECL void neko_script_throw(char *msg, ...);

NEKO_API_DECL char *neko_script_mprintf(char *fmt, ...);

NEKO_API_DECL neko_script_value_t *neko_script_value_null();
NEKO_API_DECL neko_script_value_t *neko_script_value_number(double val);
NEKO_API_DECL neko_script_value_t *neko_script_value_string(char *val);
NEKO_API_DECL neko_script_value_t *neko_script_value_array(neko_script_vector(neko_script_value_t *) arr);
NEKO_API_DECL neko_script_value_t *neko_script_value_tuple(neko_script_vector(neko_script_value_t *) tuple);
NEKO_API_DECL neko_script_value_t *neko_script_value_dict(neko_script_vector(char *) names, neko_script_vector(neko_script_value_t *) values);
NEKO_API_DECL neko_script_value_t *neko_script_value_fn(neko_script_fn_t *fn);
NEKO_API_DECL neko_script_value_t *neko_script_value_ref(neko_script_value_t *val);
NEKO_API_DECL neko_script_value_t *neko_script_value_native(ns_userdata_t val);

NEKO_API_DECL void neko_script_value_assign(neko_script_value_t *a, neko_script_value_t *b);

NEKO_API_DECL neko_script_value_t *neko_script_value_unary(int op, neko_script_value_t *a);
NEKO_API_DECL neko_script_value_t *neko_script_value_binary(int op, neko_script_value_t *a, neko_script_value_t *b);

NEKO_API_DECL neko_script_value_t *neko_script_value_get(int i, neko_script_value_t *a);
NEKO_API_DECL neko_script_value_t *neko_script_value_member(char *name, neko_script_value_t *a);

NEKO_API_DECL void neko_script_value_free(neko_script_value_t *val);

NEKO_API_DECL neko_script_marker_t neko_script_getmarker(neko_script_binary_t *binary, size_t ip);
NEKO_API_DECL void neko_script_exec(neko_script_ctx_t *global, neko_script_ctx_t *context, neko_script_binary_t *binary, int ip, neko_script_binary_t *(*load_module)(char *name),
                                    void *(trap)(neko_script_ctx_t *ctx));

NEKO_API_DECL neko_script_binary_t *neko_script_compile_str(const char *code);
NEKO_API_DECL neko_script_binary_t *neko_script_compile_file(const char *filename);

NEKO_API_DECL int neko_script_eval_str(neko_script_ctx_t *ctx, char *code, neko_script_binary_t *(*load_module)(char *name), void *(trap)(neko_script_ctx_t *ctx));
NEKO_API_DECL neko_script_binary_t *neko_script_eval_file(neko_script_ctx_t *ctx, char *filename, neko_script_binary_t *(*load_module)(char *name), void *(trap)(neko_script_ctx_t *ctx), bool do_free);
NEKO_API_DECL int neko_script_dis_str(neko_script_ctx_t *ctx, char *code, neko_script_binary_t *(*load_module)(char *name), void *(trap)(neko_script_ctx_t *ctx));

#if defined(NEKO_CPP_SRC)

#include <type_traits>
#include <vector>

template <typename T>
struct neko_is_vector : std::false_type {};

template <typename T, typename Alloc>
struct neko_is_vector<std::vector<T, Alloc>> : std::true_type {};

namespace detail {
// 某些旧版本的 GCC 需要
template <typename...>
struct voider {
    using type = void;
};

// std::void_t 将成为 C++17 的一部分 但在这里我还是自己实现吧
template <typename... T>
using void_t = typename voider<T...>::type;

template <typename T, typename U = void>
struct is_mappish_impl : std::false_type {};

template <typename T>
struct is_mappish_impl<T, void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T &>()[std::declval<const typename T::key_type &>()])>> : std::true_type {};
}  // namespace detail

template <typename T>
struct neko_is_mappish : detail::is_mappish_impl<T>::type {};

template <class... Ts>
struct neko_overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
neko_overloaded(Ts...) -> neko_overloaded<Ts...>;

template <typename T>
auto neko_script_auto(T &&value) {
    using TT = std::decay_t<decltype(value)>;
    if constexpr (neko_is_vector<TT>::value) {
        neko_script_vector(neko_script_value_t *) v = NULL;
        using ET = std::decay_t<decltype(value)>::value_type;  // std::vector 的元素类型
        if constexpr (std::is_same_v<ET, neko_script_value_t *>) {
            for (int i = 0; i < value.size(); i++) {
                neko_script_vector_push(v, (neko_script_value_t *)value[i]);
            }
        } else {
            for (int i = 0; i < value.size(); i++) {
                neko_script_vector_push(v, neko_script_auto(value[i]));
            }
        }
        return v;
    } else if constexpr (neko_is_mappish<TT>::value) {
        neko_script_vector(char *) k = NULL;
        neko_script_vector(neko_script_value_t *) v = NULL;
        using ET = std::decay_t<decltype(value)>::value_type::second_type;  // std::map 的值类型
        // print_type_name(ET);
        if constexpr (std::is_same_v<ET, neko_script_value_t *>) {
            for (auto &[kk, vv] : value) {
                neko_script_vector_push(k, strdup(kk));
                neko_script_vector_push(v, vv);
            }
        } else {
            for (auto &[kk, vv] : value) {
                neko_script_vector_push(k, strdup(kk));
                neko_script_vector_push(v, neko_script_auto(vv));
            }
        }
        return std::make_pair(k, v);
    } else {
        auto my_type = neko_overloaded{
                [](void) { return neko_script_value_null(); },                                                   //
                [](double value) { return neko_script_value_number(value); },                                    //
                [](const char *value) { return neko_script_value_string(strdup(value)); },                       //
                [](neko_script_vector(neko_script_value_t *) value) { return neko_script_value_array(value); },  //
                [](ns_userdata_t value) { return neko_script_value_native(value); },                             //
        };
        return my_type(value);
    }
}

template <typename T>
auto neko_script_auto_add(neko_script_ctx_t *ctx, const char *name, T &&value) -> void {
    using TT = std::decay_t<decltype(value)>;

    auto my_reg = neko_overloaded{
            [ctx, name](void) { neko_script_ctx_addvar(ctx, strdup(name), neko_script_value_null()); },
            [ctx, name](ns_userdata_t value) { neko_script_ctx_addvar(ctx, strdup(name), neko_script_value_native(value)); },
            [ctx, name](double value) { neko_script_ctx_addvar(ctx, strdup(name), neko_script_auto(value)); },
            [ctx, name](const char *value) { neko_script_ctx_addvar(ctx, strdup(name), neko_script_auto(value)); },
            [ctx, name](neko_script_vector(neko_script_value_t *) value) { neko_script_ctx_addvar(ctx, strdup(name), neko_script_auto(value)); },
            [ctx, name](neko_script_vector(char *) names, neko_script_vector(neko_script_value_t *) value) { neko_script_ctx_addvar(ctx, strdup(name), neko_script_value_dict(names, value)); },
    };

    if constexpr (neko_is_vector<TT>::value) {
        auto &&v = neko_script_auto(std::forward<T>(value));
        my_reg(v);
    } else if constexpr (neko_is_mappish<TT>::value) {
        auto [k, v] = neko_script_auto(std::forward<T>(value));
        my_reg(k, v);
    } else {
        auto &&v = std::forward<T>(value);
        my_reg(v);
    }
}

template <typename T>
auto neko_script_auto(neko_script_ctx_t *ctx, char *name) -> T {
    using TT = std::decay_t<T>;
    neko_script_value_t *value = neko_script_ctx_getvar(ctx, name);
    if (NULL == value) return {};
    if constexpr (std::same_as<T, double> || std::same_as<T, float> || std::is_integral_v<T> || std::is_floating_point_v<T>) {
        assert(value->type == NEKO_SC_VALUE_NUMBER);
        return static_cast<T>(value->number);
    } else if constexpr (std::same_as<T, ns_userdata_t>) {  // Native userdata
        assert(value->type == NEKO_SC_VALUE_NATIVE);
        return static_cast<T>(value->ref);
    } else if constexpr (std::same_as<T, char *>) {
        assert(value->type == NEKO_SC_VALUE_STRING);
        return static_cast<T>(value->string);
    } else if constexpr (neko_is_vector<TT>::value) {
        assert(value->type == NEKO_SC_VALUE_ARRAY);
        std::vector<double> vec{};
        for (int i = 0; i < neko_script_vector_size(value->array); i++) {
            double num = value->array[i]->number;
            vec.push_back(num);
        }
        return vec;
    } else if constexpr (neko_is_mappish<TT>::value) {
        assert(value->type == NEKO_SC_VALUE_DICT);
        neko_script_vector(char *) k = value->dict.names;
        neko_script_vector(neko_script_value_t *) v = value->dict.values;
        std::map<const char *, double> map{};
        for (int i = 0; i < neko_script_vector_size(k); i++) {
            char *name = k[i];
            double num = v[i]->number;
            map.insert(std::make_pair(name, num));
        }
        return map;
    } else {
        static_assert(std::is_same_v<T, void>, "Unsupported type for neko_script_auto");
    }
}

neko_inline void neko_script_print_stack(neko_script_ctx_t *ctx) {
    neko_script_ctx_t *c = ctx;

    printf("Stack size: %llu\n", neko_script_vector_size(c->stack));
    while (c) {
        for (int i = (int)neko_script_vector_size(c->stack) - 1; i >= 0; i--) {
            neko_script_value_t *v = c->stack[i];
            printf("[%2d] Type: %5d | \n", i, v->type);
        }
        c = c->parent;
    }

    c = ctx;

    printf("Vars size: %llu\n", neko_script_vector_size(c->vars));
    while (c) {
        for (int i = (int)neko_script_vector_size(c->vars) - 1; i >= 0; i--) {
            neko_script_var_t *v = c->vars[i];
            const char *type_name = neko_script_valuetypestr(v->val->type);
            printf("[%2d] Name: %24s | Type: %25s | Value: ", i, v->name, type_name);
            switch (v->val->type) {
                case NEKO_SC_VALUE_NUMBER:
                    printf("%lf\n", v->val->number);  // 输出字符串值
                    break;
                case NEKO_SC_VALUE_STRING:
                    printf("\"%s\"\n", v->val->string);  // 输出数字值
                    break;
                default:
                    printf("Unknown\n");
                    break;
            }
        }
        c = c->parent;
    }
}

#define ns_bind_func_local(name, ...) static void neko_binding_##name##(int argc, neko_script_ctx_t *ctx)
#define ns_bind_func_lambda(name, ...) auto neko_binding_##name## = [](int argc, neko_script_ctx_t *ctx)

template <typename RET>
auto neko_script_auto_args(neko_script_vector(neko_script_value_t *) stack) {
    using TT = std::decay_t<RET>;
    neko_script_value_t *v = neko_script_vector_pop(stack);
    if constexpr (std::same_as<TT, double> || std::same_as<TT, float> || std::is_integral_v<TT> || std::is_floating_point_v<TT>) {
        assert(v->type == NEKO_SC_VALUE_NUMBER);
        return v->number;
    } else if constexpr (std::is_same_v<TT, ns_userdata_t>) {
        assert(v->type == NEKO_SC_VALUE_NATIVE);
        return v->ref;
    } else if constexpr (std::is_same_v<TT, char *> || std::is_same_v<TT, const char *>) {
        assert(v->type == NEKO_SC_VALUE_STRING);
        return v->string;
    } else if constexpr (neko_is_vector<TT>::value) {  // TODO
        assert(v->type == NEKO_SC_VALUE_ARRAY);
        static_assert(false, "Unsupported type for neko_script_auto_args");
    } else if constexpr (neko_is_mappish<TT>::value) {
        assert(v->type == NEKO_SC_VALUE_DICT);
        static_assert(false, "Unsupported type for neko_script_auto_args");
    } else {
        static_assert(std::is_same_v<T, void>, "Unsupported type for neko_script_auto_args");
    }
}

#define ns_args(type, name) type name = neko_script_auto_args<type>(ctx->stack)
#define ns_ret(value) neko_script_vector_push(ctx->stack, neko_script_auto(value))

template <typename T, typename ARG1>
auto neko_script_auto_call(neko_script_ctx_t *ctx, const char *name, ARG1 &&arg1) {
    neko_script_fn_t *func = neko_script_ctx_getfn(ctx, const_cast<char *>(name));
    assert(func);

    neko_script_value_t *v = neko_script_vector_pop(ctx->stack);  // 先把函数顶弹出保留

    neko_script_vector_push(ctx->stack, neko_script_auto(arg1));

    neko_script_vector_push(ctx->stack, v);  // 推入函数顶

    neko_script_try { neko_script_exec(ctx, func->ctx, func->func_binary, func->address, ns_load_module, NULL); }
    neko_script_catch {}

    return neko_script_auto_args<std::decay_t<T>>(ctx->stack);
}

template <typename T, typename ARG1, typename ARG2>
auto neko_script_auto_call(neko_script_ctx_t *ctx, const char *name, ARG1 &&arg1, ARG2 &&arg2) {
    neko_script_fn_t *func = neko_script_ctx_getfn(ctx, const_cast<char *>(name));
    assert(func);

    neko_script_value_t *v = neko_script_vector_pop(ctx->stack);  // 先把函数顶弹出保留

    neko_script_vector_push(ctx->stack, neko_script_auto(arg2));
    neko_script_vector_push(ctx->stack, neko_script_auto(arg1));

    neko_script_vector_push(ctx->stack, v);  // 推入函数顶

    neko_script_try { neko_script_exec(ctx, func->ctx, func->func_binary, func->address, ns_load_module, NULL); }
    neko_script_catch {}

    return neko_script_auto_args<std::decay_t<T>>(ctx->stack);
}

template <typename T, typename ARG1, typename ARG2, typename ARG3>
auto neko_script_auto_call(neko_script_ctx_t *ctx, const char *name, ARG1 &&arg1, ARG2 &&arg2, ARG3 &&arg3) {
    neko_script_fn_t *func = neko_script_ctx_getfn(ctx, const_cast<char *>(name));
    assert(func);

    neko_script_value_t *v = neko_script_vector_pop(ctx->stack);  // 先把函数顶弹出保留

    neko_script_vector_push(ctx->stack, neko_script_auto(arg3));
    neko_script_vector_push(ctx->stack, neko_script_auto(arg2));
    neko_script_vector_push(ctx->stack, neko_script_auto(arg1));

    neko_script_vector_push(ctx->stack, v);  // 推入函数顶

    neko_script_try { neko_script_exec(ctx, func->ctx, func->func_binary, func->address, ns_load_module, NULL); }
    neko_script_catch {}

    return neko_script_auto_args<std::decay_t<T>>(ctx->stack);
}

template <typename T, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
auto neko_script_auto_call(neko_script_ctx_t *ctx, const char *name, ARG1 &&arg1, ARG2 &&arg2, ARG3 &&arg3, ARG4 &&arg4) {
    neko_script_fn_t *func = neko_script_ctx_getfn(ctx, const_cast<char *>(name));
    assert(func);

    neko_script_value_t *v = neko_script_vector_pop(ctx->stack);  // 先把函数顶弹出保留

    neko_script_vector_push(ctx->stack, neko_script_auto(arg4));
    neko_script_vector_push(ctx->stack, neko_script_auto(arg3));
    neko_script_vector_push(ctx->stack, neko_script_auto(arg2));
    neko_script_vector_push(ctx->stack, neko_script_auto(arg1));

    neko_script_vector_push(ctx->stack, v);  // 推入函数顶

    neko_script_try { neko_script_exec(ctx, func->ctx, func->func_binary, func->address, ns_load_module, NULL); }
    neko_script_catch {}

    return neko_script_auto_args<std::decay_t<T>>(ctx->stack);
}

#endif

#endif