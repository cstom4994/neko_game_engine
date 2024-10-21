#pragma once

#include <variant>

#include "engine/base/array.hpp"
#include "engine/base/base.hpp"
#include "engine/base/hashmap.hpp"
#include "engine/base/string.hpp"

struct XMLAttribute {
    String name;
    enum XMLAttributeType {
        NEKO_XML_ATTRIBUTE_NUMBER,
        NEKO_XML_ATTRIBUTE_BOOLEAN,
        NEKO_XML_ATTRIBUTE_STRING,
    } type;
    std::variant<double, bool, String> value;
};

struct XMLDoc;
struct XMLNode;

struct XMLDoc {
    Array<XMLNode> nodes;

    void Parse(String source);
    void ParseVFS(String path);
    void Trash();

    XMLNode* FindNode(String name);

    static String ProcessText(const_str start, u32 length);
    static Array<XMLNode> ParseBlock(const_str start, u32 length);
};

struct XMLNode {
    String name;
    String text;

    HashMap<XMLAttribute> attributes;
    Array<XMLNode> children;

    struct XMLNodeIter {
        XMLDoc* doc;
        XMLNode* node;
        String name;
        u32 idx;

        XMLNode* current;

        inline bool Next() {
            XMLNodeIter* iter = this;
            if (iter->node) {
                for (u32 i = iter->idx; i < iter->node->children.len; i++) {
                    if (neko_string_compare_equal(iter->name, iter->node->children[i].name)) {
                        iter->current = &iter->node->children[i];
                        iter->idx = i + 1;
                        return true;
                    }
                }

                return false;
            } else {
                for (u32 i = iter->idx; i < iter->doc->nodes.len; i++) {
                    if (neko_string_compare_equal(iter->name, iter->doc->nodes[i].name)) {
                        iter->current = &iter->doc->nodes[i];
                        iter->idx = i + 1;
                        return true;
                    }
                }
                return false;
            }
        }
    };

    inline XMLNodeIter MakeIter(XMLDoc* doc, const_str name) {
        XMLNodeIter it = {.doc = doc, .name = name, .idx = 0};
        return it;
    }

    inline XMLNodeIter MakeChildIter(const_str name) {
        XMLNodeIter it = {.node = this, .name = name, .idx = 0};
        return it;
    }

    XMLAttribute* FindAttribute(const_str name);

    template <typename T>
    auto Attribute(const_str name) {
        XMLAttribute* attr = this->FindAttribute(name);
        if (attr) {
            return std::get<T>(attr->value);
        }
        return T{};
    }

    XMLNode* FindChild(String name);

    void Trash();
};
