#ifndef NEKO_XML_H
#define NEKO_XML_H

#include "engine/neko.h"
#include "engine/neko_containers.h"

typedef enum neko_xml_attribute_type_t {
    NEKO_XML_ATTRIBUTE_NUMBER,
    NEKO_XML_ATTRIBUTE_BOOLEAN,
    NEKO_XML_ATTRIBUTE_STRING,
} neko_xml_attribute_type_t;

typedef struct neko_xml_attribute_t {
    char* name;
    neko_xml_attribute_type_t type;

    union {
        double number;
        bool boolean;
        char* string;
    } value;
} neko_xml_attribute_t;

// neko_hash_table_decl(u64, neko_xml_attribute_t, neko_hash_u64, neko_hash_key_comp_std_type);

typedef struct neko_xml_node_t {
    char* name;
    char* text;

    neko_hash_table(u64, neko_xml_attribute_t) attributes;
    neko_dyn_array(struct neko_xml_node_t) children;

} neko_xml_node_t;

typedef struct neko_xml_document_t {
    neko_dyn_array(neko_xml_node_t) nodes;
} neko_xml_document_t;

typedef struct neko_xml_node_iter_t {
    neko_xml_document_t* doc;
    neko_xml_node_t* node;
    const_str name;
    u32 idx;

    neko_xml_node_t* current;
} neko_xml_node_iter_t;

NEKO_API_DECL neko_xml_document_t* neko_xml_parse(const_str source);
NEKO_API_DECL neko_xml_document_t* neko_xml_parse_file(const_str path);
NEKO_API_DECL void neko_xml_free(neko_xml_document_t* document);

NEKO_API_DECL neko_xml_attribute_t* neko_xml_find_attribute(neko_xml_node_t* node, const_str name);
NEKO_API_DECL neko_xml_node_t* neko_xml_find_node(neko_xml_document_t* doc, const_str name);
NEKO_API_DECL neko_xml_node_t* neko_xml_find_node_child(neko_xml_node_t* node, const_str name);

NEKO_API_DECL const_str neko_xml_get_error();

NEKO_API_DECL neko_xml_node_iter_t neko_xml_new_node_iter(neko_xml_document_t* doc, const_str name);
NEKO_API_DECL neko_xml_node_iter_t neko_xml_new_node_child_iter(neko_xml_node_t* node, const_str name);
NEKO_API_DECL bool neko_xml_node_iter_next(neko_xml_node_iter_t* iter);

#endif  // NEKO_XML_H
