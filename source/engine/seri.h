
#ifndef NEKO_SERI_H
#define NEKO_SERI_H

#include "engine/base.h"
#include "engine/prelude.h"

inline bool neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }
inline bool neko_token_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_token_is_end_of_line(c)); }
inline bool neko_token_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
inline bool neko_token_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

typedef enum neko_xml_attribute_type_t {
    NEKO_XML_ATTRIBUTE_NUMBER,
    NEKO_XML_ATTRIBUTE_BOOLEAN,
    NEKO_XML_ATTRIBUTE_STRING,
} neko_xml_attribute_type_t;

typedef struct neko_xml_attribute_t {
    const_str name;
    neko_xml_attribute_type_t type;

    union {
        double number;
        bool boolean;
        const_str string;
    } value;
} neko_xml_attribute_t;

// neko_hash_table_decl(u64, neko_xml_attribute_t, neko_hash_u64, neko_hash_key_comp_std_type);

typedef struct neko_xml_node_t {
    const_str name;
    const_str text;

    neko_hash_table(u64, neko_xml_attribute_t) attributes;
    neko_dyn_array(struct neko_xml_node_t) children;

} neko_xml_node_t;

typedef struct neko_xml_document_t {
    neko_dyn_array(neko_xml_node_t) nodes;
} neko_xml_document_t;

typedef struct neko_xml_node_iter_t {
    neko_xml_document_t *doc;
    neko_xml_node_t *node;
    const_str name;
    u32 idx;

    neko_xml_node_t *current;
} neko_xml_node_iter_t;

neko_xml_document_t *neko_xml_parse(const_str source);
neko_xml_document_t *neko_xml_parse_file(const_str path);
void neko_xml_free(neko_xml_document_t *document);

neko_xml_attribute_t *neko_xml_find_attribute(neko_xml_node_t *node, const_str name);
neko_xml_node_t *neko_xml_find_node(neko_xml_document_t *doc, const_str name);
neko_xml_node_t *neko_xml_find_node_child(neko_xml_node_t *node, const_str name);

const_str neko_xml_get_error();

neko_xml_node_iter_t neko_xml_new_node_iter(neko_xml_document_t *doc, const_str name);
neko_xml_node_iter_t neko_xml_new_node_child_iter(neko_xml_node_t *node, const_str name);
bool neko_xml_node_iter_next(neko_xml_node_iter_t *iter);

enum JSONKind : i32 {
    JSONKind_Null,
    JSONKind_Object,
    JSONKind_Array,
    JSONKind_String,
    JSONKind_Number,
    JSONKind_Boolean,
};

struct JSONObject;
struct JSONArray;
struct JSON {
    union {
        JSONObject *object;
        JSONArray *array;
        String string;
        double number;
        bool boolean;
    };
    JSONKind kind;

    JSON lookup(String key, bool *ok);
    JSON index(i32 i, bool *ok);

    JSONObject *as_object(bool *ok);
    JSONArray *as_array(bool *ok);
    String as_string(bool *ok);
    double as_number(bool *ok);

    JSONObject *lookup_object(String key, bool *ok);
    JSONArray *lookup_array(String key, bool *ok);
    String lookup_string(String key, bool *ok);
    double lookup_number(String key, bool *ok);

    double index_number(i32 i, bool *ok);
};

struct JSONObject {
    JSON value;
    String key;
    JSONObject *next;
    u64 hash;
};

struct JSONArray {
    JSON value;
    JSONArray *next;
    u64 index;
};

struct JSONDocument {
    JSON root;
    String error;
    Arena arena;

    void parse(String contents);
    void trash();
};

struct StringBuilder;
void json_write_string(StringBuilder *sb, JSON *json);
void json_print(JSON *json);

struct lua_State;
void json_to_lua(lua_State *L, JSON *json);
String lua_to_json_string(lua_State *L, i32 arg, String *contents, i32 width);

#endif