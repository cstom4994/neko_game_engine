#include "xml.hpp"

#include "engine/base/util.hpp"
#include "engine/base/vfs.hpp"

static const struct {
    char character;
    const_str name;
} g_xml_entities[] = {{'&', "&amp;"}, {'\'', "&apos;"}, {'"', "&quot;"}, {'<', "&lt;"}, {'>', "&gt;"}};

void XMLNode::Trash() {
    for (auto kv : this->attributes) {
        XMLAttribute *attrib = kv.value;
        if (attrib->type == XMLAttribute::NEKO_XML_ATTRIBUTE_STRING) {
            mem_free(std::get<String>(attrib->value).data);
        }
        mem_free(attrib->name.data);
    }

    for (u32 i = 0; i < this->children.len; i++) {
        this->children[i].Trash();
    }

    mem_free(this->name.data);
    mem_free(this->text.data);
    this->attributes.trash();
    this->children.trash();
}

String XMLDoc::ProcessText(const_str start, u32 length) {
    char *r = (char *)mem_alloc(length + 1);

    u32 len_sub = 0;

    for (u32 i = 0, ri = 0; i < length; i++, ri++) {
        bool changed = false;
        if (start[i] == '&') {
            for (u32 ii = 0; ii < sizeof(g_xml_entities) / sizeof(*g_xml_entities); ii++) {
                u32 ent_len = neko_strlen(g_xml_entities[ii].name);
                if (StringEqualN(start + i, ent_len, g_xml_entities[ii].name)) {
                    r[ri] = g_xml_entities[ii].character;
                    i += ent_len - 1;
                    len_sub += ent_len - 1;
                    changed = true;
                    break;
                }
            }
        }

        if (!changed) r[ri] = start[i];
    }

    r[length - len_sub] = '\0';

    return r;
}

// 解析XML块 返回块中的节点数组
Array<XMLNode> XMLDoc::ParseBlock(const_str start, u32 length) {

#define XMLExpectNotEnd(c_)                       \
    if (!*(c_)) {                                 \
        throw("xml_parse_block: Unexpected end"); \
        return {};                                \
    }

    Array<XMLNode> root = {};
    bool is_inside = false;
    for (const_str c = start; *c && c < start + length; c++) {
        if (*c == '<') {
            c++;
            XMLExpectNotEnd(c);
            if (*c == '?')  // 跳过XML头
            {
                c++;
                XMLExpectNotEnd(c);
                while (*c != '>') {
                    c++;
                    XMLExpectNotEnd(c);
                }
                continue;
            } else if (StringEqualN(c, 3, "!--"))  // 跳过注释
            {
                c++;
                XMLExpectNotEnd(c);
                c++;
                XMLExpectNotEnd(c);
                c++;
                XMLExpectNotEnd(c);
                while (!StringEqualN(c, 3, "-->")) {
                    c++;
                    XMLExpectNotEnd(c);
                }

                continue;
            }

            if (is_inside && *c == '/')
                is_inside = false;
            else
                is_inside = true;

            const_str node_name_start = c;
            u32 node_name_len = 0;

            XMLNode current_node = {};

            if (is_inside) {
                for (; *c != '>' && *c != ' ' && *c != '/'; c++) node_name_len++;

                if (*c != '>') {
                    while (*c != '>' && *c != '/') {
                        while (is_whitespace(*c)) c++;

                        const_str attrib_name_start = c;
                        u32 attrib_name_len = 0;

                        while (is_alpha(*c) || is_digit(*c) || *c == '_') {
                            c++;
                            attrib_name_len++;
                            XMLExpectNotEnd(c);
                        }

                        while (*c != '"') {
                            c++;
                            XMLExpectNotEnd(c);
                        }

                        c++;
                        XMLExpectNotEnd(c);

                        const_str attrib_text_start = c;
                        u32 attrib_text_len = 0;

                        while (*c != '"') {
                            c++;
                            attrib_text_len++;
                            XMLExpectNotEnd(c);
                        }

                        c++;
                        XMLExpectNotEnd(c);

                        XMLAttribute attrib = {};
                        attrib.name = StringCopy(attrib_name_start, attrib_name_len);

                        if (neko_string_is_decimal(attrib_text_start, attrib_text_len)) {
                            attrib.type = XMLAttribute::NEKO_XML_ATTRIBUTE_NUMBER;
                            attrib.value = strtod(attrib_text_start, NULL);
                        } else if (StringEqualN(attrib_text_start, attrib_text_len, "true")) {
                            attrib.type = XMLAttribute::NEKO_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value = true;
                        } else if (StringEqualN(attrib_text_start, attrib_text_len, "false")) {
                            attrib.type = XMLAttribute::NEKO_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value = false;
                        } else {
                            attrib.type = XMLAttribute::NEKO_XML_ATTRIBUTE_STRING;
                            attrib.value = XMLDoc::ProcessText(attrib_text_start, attrib_text_len);
                        }

                        current_node.attributes[fnv1a(attrib_name_start, attrib_name_len)] = attrib;
                    }
                }

                if (*c == '/')  // 对于没有任何文本的节点
                {
                    c++;
                    XMLExpectNotEnd(c);
                    current_node.name = StringCopy(node_name_start, node_name_len);
                    root.push(current_node);
                    is_inside = false;
                }
            } else {
                while (*c != '>') {
                    c++;
                    XMLExpectNotEnd(c);
                }
            }

            c++;
            XMLExpectNotEnd(c);

            if (is_inside) {
                const_str text_start = c;
                u32 text_len = 0;

                const_str end_start = c;
                u32 end_len = 0;

                current_node.name = StringCopy(node_name_start, node_name_len);

                for (u32 i = 0; i < length; i++) {
                    if (*c == '<' && *(c + 1) == '/') {
                        c++;
                        XMLExpectNotEnd(c);
                        c++;
                        XMLExpectNotEnd(c);
                        end_start = c;
                        end_len = 0;
                        while (*c != '>') {
                            end_len++;
                            c++;
                            XMLExpectNotEnd(c);
                        }

                        if (StringEqualN(end_start, end_len, current_node.name.data)) {
                            break;
                        } else {
                            text_len += end_len + 2;
                            continue;
                        }
                    }

                    c++;
                    text_len++;

                    XMLExpectNotEnd(c);
                }

                current_node.children = ParseBlock(text_start, text_len);
                if (current_node.children.len == 0)
                    current_node.text = XMLDoc::ProcessText(text_start, text_len);
                else
                    current_node.text = StringCopy(text_start, text_len);

                root.push(current_node);

                c--;
            }
        }
    }

#undef XMLExpectNotEnd

    return root;
}

void XMLDoc::ParseVFS(String path) {
    size_t size;
    const_str source = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, path.cstr(), &size);
    neko_assert(source);
    Parse(source);
    mem_free(source);
    return;
}

void XMLDoc::Parse(String source) {
    if (!source.data) return;
    try {
        this->nodes = ParseBlock(source.data, source.len);
    } catch (...) {
        this->Trash();
        return;
    }
    return;
}

void XMLDoc::Trash() {
    for (u32 i = 0; i < this->nodes.len; i++) {
        this->nodes[i].Trash();
    }

    this->nodes.trash();
}

XMLAttribute *XMLNode::FindAttribute(const_str name) {
    u64 key = fnv1a(name, neko_strlen(name));
    XMLAttribute *attr = this->attributes.get(key);
    return attr;
}

XMLNode *XMLDoc::FindNode(String name) {
    for (u32 i = 0; i < this->nodes.len; i++)
        if (neko_string_compare_equal(name, this->nodes[i].name)) return &this->nodes[i];
    return NULL;
}

XMLNode *XMLNode::FindChild(String name) {
    for (u32 i = 0; i < this->children.len; i++)
        if (neko_string_compare_equal(name, this->children[i].name)) return &this->children[i];
    return NULL;
}