#include "xml.hpp"

#include "base/common/util.hpp"
#include "base/common/vfs.hpp"

namespace Neko {

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
    Array<XMLNode> root = {};
    const_str end = start + length;

    for (const_str c = start; c < end && *c;) {
        if (*c == '<') {
            c++;
            if (c >= end || !*c) break;

            // 处理XML声明
            if (*c == '?') {
                while (c < end && *c && *c != '>') c++;
                if (c < end && *c == '>') c++;
                continue;
            }

            // 处理注释
            if (StringEqualN(c, 3, "!--")) {
                c += 3;
                while (c + 3 <= end && !StringEqualN(c, 3, "-->")) c++;
                if (c + 3 <= end) c += 3;
                continue;
            }

            // 处理结束标签
            if (*c == '/') {
                c++;
                while (c < end && *c && *c != '>') c++;
                if (c < end && *c == '>') c++;
                continue;
            }

            // 开始解析新节点
            XMLNode current_node = {};
            const_str node_name_start = c;
            u32 node_name_len = 0;

            // 获取节点名称
            while (c < end && *c && !is_whitespace(*c) && *c != '>' && *c != '/') {
                node_name_len++;
                c++;
            }

            current_node.name = StringCopy(node_name_start, node_name_len);

            // 解析属性
            while (c < end && *c && *c != '>' && *c != '/') {
                // 跳过空白
                while (c < end && *c && is_whitespace(*c)) c++;
                if (c >= end || !*c || *c == '>' || *c == '/') break;

                // 获取属性名
                const_str attrib_name_start = c;
                u32 attrib_name_len = 0;
                while (c < end && *c && (is_alpha(*c) || is_digit(*c) || *c == '_' || *c == '-')) {
                    attrib_name_len++;
                    c++;
                }

                // 跳过等号
                while (c < end && *c && *c != '=') c++;
                if (c < end && *c == '=') c++;

                // 跳过引号前的空白
                while (c < end && *c && is_whitespace(*c)) c++;

                // 获取属性值
                char quote = *c;
                if (quote != '"' && quote != '\'') break;
                c++;

                const_str attrib_value_start = c;
                u32 attrib_value_len = 0;
                while (c < end && *c && *c != quote) {
                    attrib_value_len++;
                    c++;
                }
                if (c < end && *c == quote) c++;

                // 处理属性值
                XMLAttribute attrib = {};
                attrib.name = StringCopy(attrib_name_start, attrib_name_len);

                // 处理 value 属性
                if (neko_string_is_decimal(attrib_value_start, attrib_value_len)) {
                    attrib.type = XMLAttribute::NEKO_XML_ATTRIBUTE_NUMBER;
                    attrib.value = strtod(attrib_value_start, NULL);
                } else if (StringEqualN(attrib_value_start, attrib_value_len, "true")) {
                    attrib.type = XMLAttribute::NEKO_XML_ATTRIBUTE_BOOLEAN;
                    attrib.value = true;
                } else if (StringEqualN(attrib_value_start, attrib_value_len, "false")) {
                    attrib.type = XMLAttribute::NEKO_XML_ATTRIBUTE_BOOLEAN;
                    attrib.value = false;
                } else {
                    attrib.type = XMLAttribute::NEKO_XML_ATTRIBUTE_STRING;
                    attrib.value = ProcessText(attrib_value_start, attrib_value_len);
                }

                current_node.attributes[fnv1a(attrib_name_start, attrib_name_len)] = attrib;
            }

            // 处理自闭合标签
            if (c < end && *c == '/') {
                c++;
                if (c < end && *c == '>') {
                    c++;
                    root.push(current_node);
                    continue;
                }
            }

            // 处理常规标签内容
            if (c < end && *c == '>') {
                c++;
                const_str content_start = c;
                u32 content_len = 0;

                // 查找结束标签
                while (c < end && *c) {
                    if (*c == '<' && (c + 1) < end && *(c + 1) == '/') {
                        // 检查是否匹配当前节点名
                        const_str tag_end = c + 2;
                        const_str node_name_ptr = current_node.name.data;

                        while (tag_end < end && *tag_end && *node_name_ptr && *tag_end == *node_name_ptr) {
                            tag_end++;
                            node_name_ptr++;
                        }

                        if (*node_name_ptr == '\0' && tag_end < end && *tag_end == '>') {
                            // 匹配成功
                            content_len = c - content_start;
                            c = tag_end + 1;  // 跳过结束标签
                            break;
                        }
                    }
                    c++;
                }

                // 处理内容
                if (content_len > 0) {
                    // 检查内容中是否包含子节点
                    bool has_children = false;
                    for (const_str p = content_start; p < content_start + content_len; p++) {
                        if (*p == '<') {
                            has_children = true;
                            break;
                        }
                    }

                    if (has_children) {
                        current_node.children = ParseBlock(content_start, content_len);
                    } else {
                        // 去除前后空白
                        const_str text_start = content_start;
                        const_str text_end = content_start + content_len;

                        while (text_start < text_end && is_whitespace(*text_start)) text_start++;
                        while (text_end > text_start && is_whitespace(*(text_end - 1))) text_end--;

                        if (text_end > text_start) {
                            current_node.text = ProcessText(text_start, text_end - text_start);
                        }
                    }
                }

                root.push(current_node);
            }
        } else {
            c++;
        }
    }

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

}  // namespace Neko