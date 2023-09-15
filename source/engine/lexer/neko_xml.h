#ifndef NEKO_XML_H
#define NEKO_XML_H

#include "engine/common/neko_containers.h"
#include "engine/common/neko_types.h"
#include "engine/neko.h"

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

neko_hash_table_decl(u64, neko_xml_attribute_t, neko_hash_u64, neko_hash_key_comp_std_type);

typedef struct neko_xml_node_t {
    char* name;
    char* text;

    neko_hash_table(u64, neko_xml_attribute_t) attributes;
    neko_dyn_array(neko_xml_node_t) children;

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

neko_xml_document_t* neko_xml_parse(const_str source);
neko_xml_document_t* neko_xml_parse_file(const_str path);
void neko_xml_free(neko_xml_document_t* document);

neko_xml_attribute_t* neko_xml_find_attribute(neko_xml_node_t* node, const_str name);
neko_xml_node_t* neko_xml_find_node(neko_xml_document_t* doc, const_str name);
neko_xml_node_t* neko_xml_find_node_child(neko_xml_node_t* node, const_str name);

const_str neko_xml_get_error();

neko_xml_node_iter_t neko_xml_new_node_iter(neko_xml_document_t* doc, const_str name);
neko_xml_node_iter_t neko_xml_new_node_child_iter(neko_xml_node_t* node, const_str name);
bool neko_xml_node_iter_next(neko_xml_node_iter_t* iter);

#endif  // NEKO_XML_H
