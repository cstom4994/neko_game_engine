
#include "shader.h"

#include "base/common/string.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace Neko {

enum class TokenType {
    ATTRIBUTE,
    UNIFORM,
    VARYING,
    STRUCT,
    IDENTIFIER,
    TYPE,
    NUMBER,
    SEMICOLON,
    BEGIN,
    END,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    COMMA,
    DOT,
    DEFINE,
    VERSION,
    OUT,
    OTHER
};

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
};

class Lexer {
private:
    std::string source;
    size_t position = 0;
    size_t line = 1;
    size_t column = 1;

    void advance() {
        if (position >= source.length()) return;
        if (source[position] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
    }

    bool match(const std::string& keyword) {
        if (source.substr(position, keyword.size()) == keyword) {
            for (size_t i = 0; i < keyword.size(); ++i) advance();
            return true;
        }
        return false;
    }

    std::string parseNumber() {
        size_t start = position;
        bool has_dot = false;
        while (position < source.size() &&
               (std::isdigit(source[position]) || (source[position] == '.' && !has_dot) || (tolower(source[position]) == 'e' || source[position] == '+' || source[position] == '-'))) {
            if (source[position] == '.') has_dot = true;
            advance();
        }
        return source.substr(start, position - start);
    }

public:
    explicit Lexer(const std::string& src) : source(src) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (position < source.length()) {
            const size_t start_line = line;
            const size_t start_col = column;
            const char c = source[position];

            if (std::isspace(c)) {
                advance();
                continue;
            }

            if (c == '/') {
                if (position + 1 < source.length() && source[position + 1] == '/') {
                    while (position < source.length() && source[position] != '\n') advance();
                    continue;
                } else if (position + 1 < source.length() && source[position + 1] == '*') {
                    advance();
                    advance();
                    while (position < source.length()) {
                        if (source[position] == '*' && position + 1 < source.length() && source[position + 1] == '/') {
                            advance();
                            advance();
                            break;
                        }
                        advance();
                    }
                    continue;
                }
            }

            bool matched = false;
            const static std::vector<std::tuple<std::string, TokenType>> keywords = {{"@attribute", TokenType::ATTRIBUTE},
                                                                                     {"@uniform", TokenType::UNIFORM},
                                                                                     {"@varying", TokenType::VARYING},
                                                                                     {"@version", TokenType::VERSION},
                                                                                     {"@out", TokenType::OUT},
                                                                                     {"#define", TokenType::DEFINE},
                                                                                     {"#begin", TokenType::BEGIN},
                                                                                     {"#end", TokenType::END},
                                                                                     {"struct", TokenType::STRUCT},
                                                                                     {"void", TokenType::TYPE},
                                                                                     {"float", TokenType::TYPE},
                                                                                     {"int", TokenType::TYPE},
                                                                                     {"vec2", TokenType::TYPE},
                                                                                     {"vec3", TokenType::TYPE},
                                                                                     {"vec4", TokenType::TYPE},
                                                                                     {"mat3", TokenType::TYPE},
                                                                                     {"mat4", TokenType::TYPE},
                                                                                     {"sampler2D", TokenType::TYPE},
                                                                                     {"bool", TokenType::TYPE}};

            for (const auto& [kw, tt] : keywords) {
                if (match(kw)) {
                    tokens.push_back({tt, kw, start_line, start_col});
                    matched = true;
                    break;
                }
            }
            if (matched) continue;

            if (c == '{') {
                tokens.push_back({TokenType::LEFT_BRACE, "{", start_line, start_col});
                advance();
            } else if (c == '}') {
                tokens.push_back({TokenType::RIGHT_BRACE, "}", start_line, start_col});
                advance();
            } else if (c == '(') {
                tokens.push_back({TokenType::LEFT_PAREN, "(", start_line, start_col});
                advance();
            } else if (c == ')') {
                tokens.push_back({TokenType::RIGHT_PAREN, ")", start_line, start_col});
                advance();
            } else if (c == '[') {
                tokens.push_back({TokenType::LEFT_BRACKET, "[", start_line, start_col});
                advance();
            } else if (c == ']') {
                tokens.push_back({TokenType::RIGHT_BRACKET, "]", start_line, start_col});
                advance();
            } else if (c == ',') {
                tokens.push_back({TokenType::COMMA, ",", start_line, start_col});
                advance();
            } else if (c == '.') {
                tokens.push_back({TokenType::DOT, ".", start_line, start_col});
                advance();
            } else if (c == ';') {
                tokens.push_back({TokenType::SEMICOLON, ";", start_line, start_col});
                advance();
            } else if (std::isdigit(c) || c == '.') {
                tokens.push_back({TokenType::NUMBER, parseNumber(), start_line, start_col});
            } else if (std::isalpha(c) || c == '_' || (c & 0x80)) {
                size_t start = position;
                while (position < source.size() && (std::isalnum(source[position]) || source[position] == '_' || (source[position] & 0x80))) {
                    advance();
                }
                std::string id = source.substr(start, position - start);
                tokens.push_back({TokenType::IDENTIFIER, id, start_line, start_col});
            } else {
                tokens.push_back({TokenType::OTHER, std::string(1, c), start_line, start_col});
                advance();
            }
        }
        return tokens;
    }
};

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct VariableDeclaration final : ASTNode {
    std::string type;
    std::string name;
    std::string arraySize;
    TokenType qualifier;
    VariableDeclaration(std::string t, std::string n, TokenType q, std::string arr = "") : type(std::move(t)), name(std::move(n)), qualifier(q), arraySize(std::move(arr)) {}
};

struct StructMember {
    std::string type;
    std::string name;
    std::string arraySize;
};

struct StructDefinition final : ASTNode {
    std::string name;
    std::vector<StructMember> members;
    StructDefinition(std::string n, std::vector<StructMember> m) : name(std::move(n)), members(std::move(m)) {}
};

struct Parameter {
    std::string type;
    std::string name;
};

struct FunctionDefinition final : ASTNode {
    std::string returnType;
    std::string name;
    std::vector<Parameter> parameters;
    std::vector<Token> body;
    FunctionDefinition(std::string rt, std::string n, std::vector<Parameter> params, std::vector<Token> b)
        : returnType(std::move(rt)), name(std::move(n)), parameters(std::move(params)), body(std::move(b)) {}
};

struct MacroDefinition final : ASTNode {
    std::string name;
    std::string value;
    MacroDefinition(std::string n, std::string v) : name(std::move(n)), value(std::move(v)) {}
};

struct VersionDirective final : ASTNode {
    std::string version;
    VersionDirective(std::string v) : version(std::move(v)) {}
};

class Parser {
    std::vector<Token> tokens;
    size_t index = 0;

    Token consume(TokenType expected, const std::string& msg) {
        if (index >= tokens.size()) throw std::runtime_error(msg + " at EOF");
        Token& t = tokens[index];
        if (t.type != expected) {
            throw std::runtime_error(msg + "\n  at line " + std::to_string(t.line) + ":" + std::to_string(t.column) + "\n  current token: " + t.value);
        }
        return tokens[index++];
    }

    Token consumeType() {
        if (index >= tokens.size()) throw std::runtime_error("Expected type at EOF");
        Token& t = tokens[index];
        if (t.type != TokenType::TYPE && t.type != TokenType::IDENTIFIER) {
            throw std::runtime_error("Expected type, got " + t.value + " at line " + std::to_string(t.line) + ":" + std::to_string(t.column));
        }
        return tokens[index++];
    }

    bool match(TokenType type) {
        if (index < tokens.size() && tokens[index].type == type) {
            index++;
            return true;
        }
        return false;
    }

public:
    explicit Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}

    std::unordered_map<std::string, std::vector<std::shared_ptr<ASTNode>>> parse() {
        std::unordered_map<std::string, std::vector<std::shared_ptr<ASTNode>>> blocks;
        while (index < tokens.size()) {
            if (match(TokenType::BEGIN)) {
                Token typeToken = consume(TokenType::IDENTIFIER, "Expected shader type after #begin");
                size_t end = findEnd(typeToken.value);
                std::vector<Token> block(tokens.begin() + index, tokens.begin() + end);
                Parser blockParser(block);
                blocks[typeToken.value] = blockParser.parseBlock();
                index = end + 2;  // Skip #end and identifier
            } else {
                index++;
            }
        }
        return blocks;
    }

    std::vector<std::shared_ptr<ASTNode>> parseBlock() {
        std::vector<std::shared_ptr<ASTNode>> nodes;
        while (index < tokens.size()) {
            if (match(TokenType::DEFINE)) {
                parseMacro(nodes);
            } else if (match(TokenType::VERSION)) {
                parseVersion(nodes);
            } else if (match(TokenType::OUT) || match(TokenType::ATTRIBUTE) || match(TokenType::UNIFORM) || match(TokenType::VARYING)) {
                parseVarDecl(nodes);
            } else if (match(TokenType::ATTRIBUTE) || match(TokenType::UNIFORM) || match(TokenType::VARYING)) {
                parseVarDecl(nodes);
            } else if (tokens[index].type == TokenType::STRUCT) {
                nodes.push_back(parseStruct());
            } else if (isFunction()) {
                nodes.push_back(parseFunction());
            } else {
                index++;
            }
        }
        return nodes;
    }

private:
    size_t findEnd(const std::string& type) {
        for (size_t i = index; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::END && i + 1 < tokens.size() && tokens[i + 1].type == TokenType::IDENTIFIER && tokens[i + 1].value == type) {
                return i;
            }
        }
        throw std::runtime_error("Missing #end for " + type);
    }

    bool isFunction() {
        if (index + 2 >= tokens.size()) return false;
        return (tokens[index].type == TokenType::TYPE || tokens[index].type == TokenType::IDENTIFIER) && tokens[index + 1].type == TokenType::IDENTIFIER &&
               tokens[index + 2].type == TokenType::LEFT_PAREN;
    }

    void parseVarDecl(std::vector<std::shared_ptr<ASTNode>>& nodes) {
        const TokenType qual = tokens[index - 1].type;
        Token type = consumeType();
        Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

        std::string arraySize;
        if (match(TokenType::LEFT_BRACKET)) {
            Token size = consume(TokenType::NUMBER, "Expected array size");
            arraySize = size.value;
            consume(TokenType::RIGHT_BRACKET, "Expected ']' after array size");
        }

        consume(TokenType::SEMICOLON, "Expected ';' after declaration");
        nodes.push_back(std::make_shared<VariableDeclaration>(type.value, name.value, qual, arraySize));
    }

    std::shared_ptr<StructDefinition> parseStruct() {
        consume(TokenType::STRUCT, "Expected 'struct' keyword");
        Token nameToken = consume(TokenType::IDENTIFIER, "Expected struct name");
        consume(TokenType::LEFT_BRACE, "Expected '{' after struct name");

        std::vector<StructMember> members;
        while (!match(TokenType::RIGHT_BRACE)) {
            Token typeToken = consumeType();
            Token nameToken = consume(TokenType::IDENTIFIER, "Expected member name");

            std::string arraySize;
            if (match(TokenType::LEFT_BRACKET)) {
                Token sizeToken = consume(TokenType::NUMBER, "Expected array size");
                arraySize = sizeToken.value;
                consume(TokenType::RIGHT_BRACKET, "Expected ']' after array size");
            }
            consume(TokenType::SEMICOLON, "Expected ';' after struct member");
            members.push_back({typeToken.value, nameToken.value, arraySize});
        }
        consume(TokenType::SEMICOLON, "Expected ';' after struct definition");
        return std::make_shared<StructDefinition>(StructDefinition{nameToken.value, members});
    }

    std::shared_ptr<FunctionDefinition> parseFunction() {
        Token retType = tokens[index++];
        Token name = consume(TokenType::IDENTIFIER, "Expected function name");

        consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
        std::vector<Parameter> params;
        while (!match(TokenType::RIGHT_PAREN)) {
            Token t = consumeType();
            Token n = consume(TokenType::IDENTIFIER, "Expected parameter name");
            params.push_back({t.value, n.value});
            if (!match(TokenType::COMMA) && tokens[index].type != TokenType::RIGHT_PAREN) {
                throw std::runtime_error("Expected ',' or ')' in function parameters");
            }
        }

        consume(TokenType::LEFT_BRACE, "Expected '{' before function body");
        std::vector<Token> body;
        int braceDepth = 1;
        while (braceDepth > 0 && index < tokens.size()) {
            if (tokens[index].type == TokenType::LEFT_BRACE)
                braceDepth++;
            else if (tokens[index].type == TokenType::RIGHT_BRACE)
                braceDepth--;

            if (braceDepth > 0) body.push_back(tokens[index]);
            index++;
        }
        return std::make_shared<FunctionDefinition>(retType.value, name.value, params, body);
    }

    void parseMacro(std::vector<std::shared_ptr<ASTNode>>& nodes) {
        Token nameToken = consume(TokenType::IDENTIFIER, "Expected macro name");
        std::string value;
        while (index < tokens.size() && tokens[index].line == nameToken.line /*&& tokens[index].type != TokenType::END_OF_LINE*/) {
            value += tokens[index++].value + " ";
        }
        if (!value.empty()) value.pop_back();  // 移除末尾空格
        nodes.push_back(std::make_shared<MacroDefinition>(nameToken.value, value));
    }

    void parseVersion(std::vector<std::shared_ptr<ASTNode>>& nodes) {
        Token versionToken = consume(TokenType::NUMBER, "Expected version number");
        nodes.push_back(std::make_shared<VersionDirective>(versionToken.value));
    }
};

class CodeGenerator {
    static void generateVarDecl(std::ostream& os, const VariableDeclaration& var, int& loc, const std::string& shaderType) {
        switch (var.qualifier) {
            case TokenType::ATTRIBUTE:
                os << "layout(location=" << loc++ << ") in ";
                break;
            case TokenType::UNIFORM:
                os << "uniform ";
                break;
            case TokenType::VARYING:
                os << (shaderType == "VERTEX" ? "out " : "in ");
                break;
            case TokenType::OUT:
                os << "out ";
                break;
            default:
                break;
        }
        os << var.type << " " << var.name;
        if (!var.arraySize.empty()) os << "[" << var.arraySize << "]";
        os << ";\n";
    }

    static bool needsSpace(const Token& prev, const Token& curr) {
        const static std::string noSpaceBefore = ".,)]};";
        const static std::string noSpaceAfter = "([{";
        return !(noSpaceBefore.find(curr.value[0]) != std::string::npos || noSpaceAfter.find(prev.value[0]) != std::string::npos);
    }

public:
    static std::unordered_map<std::string, std::string> generate(const std::unordered_map<std::string, std::vector<std::shared_ptr<ASTNode>>>& blocks) {
        std::unordered_map<std::string, std::string> outputs;
        for (const auto& [shaderType, nodes] : blocks) {
            std::ostringstream oss;
            int attrLoc = 0;

            for (const auto& node : nodes) {
                if (auto ver = std::dynamic_pointer_cast<VersionDirective>(node)) {  // 处理版本声明
                    oss << "#version " << ver->version << " core\n\n";
                }
                if (auto macro = std::dynamic_pointer_cast<MacroDefinition>(node)) {  // 处理宏定义
                    oss << "#define " << macro->name << " " << macro->value << "\n";
                }
                if (auto var = std::dynamic_pointer_cast<VariableDeclaration>(node)) {  // 处理变量声明
                    generateVarDecl(oss, *var, attrLoc, shaderType);
                }
                if (auto s = std::dynamic_pointer_cast<StructDefinition>(node)) {  // 处理结构体
                    oss << "struct " << s->name << " {\n";
                    for (const auto& m : s->members) {
                        oss << "    " << m.type << " " << m.name;
                        if (!m.arraySize.empty()) oss << "[" << m.arraySize << "]";
                        oss << ";\n";
                    }
                    oss << "};\n\n";
                }
                if (auto func = std::dynamic_pointer_cast<FunctionDefinition>(node)) {  // 处理函数体
                    oss << "\n" << func->returnType << " " << func->name << "(";
                    for (size_t i = 0; i < func->parameters.size(); ++i) {
                        oss << func->parameters[i].type << " " << func->parameters[i].name;
                        if (i < func->parameters.size() - 1) oss << ", ";
                    }
                    oss << ") {\n";

                    std::string line;
                    const Token* prev = nullptr;
                    for (const auto& token : func->body) {
                        if (prev && needsSpace(*prev, token)) line += " ";
                        line += token.value;

                        if (token.type == TokenType::SEMICOLON || token.type == TokenType::LEFT_BRACE || token.type == TokenType::RIGHT_BRACE) {
                            oss << "    " << line << "\n";
                            line.clear();
                        }
                        prev = &token;
                    }
                    if (!line.empty()) oss << "    " << line << "\n";
                    oss << "}\n";
                }
            }

            outputs[shaderType] = oss.str();
        }
        return outputs;
    }
};

int ShaderParse(String src) {

    Lexer lexer(src.cstr());
    try {
        auto tokens = lexer.tokenize();
        Parser parser(tokens);

        std::cout << "Tokens:\n";
        for (const auto& token : tokens) {
            std::cout << "Type: " << static_cast<int>(token.type) << ", Value: \t" << token.value << "\n";
        }

        auto ast = parser.parse();

        std::cout << "\nAST:\n";
        for (const auto& shader_block : ast) {
            std::cout << "=== " << shader_block.first << " SHADER AST ===\n";
            for (const auto& node : shader_block.second) {
                if (auto varDecl = std::dynamic_pointer_cast<VariableDeclaration>(node)) {
                    std::string qualifier;
                    switch (varDecl->qualifier) {
                        case TokenType::ATTRIBUTE:
                            qualifier = "attribute";
                            break;
                        case TokenType::UNIFORM:
                            qualifier = "uniform";
                            break;
                        case TokenType::VARYING:
                            qualifier = "varying";
                            break;
                        default:
                            qualifier = "unknown";
                            break;
                    }
                    std::cout << "VarDecl: " << qualifier << " " << varDecl->type << " " << varDecl->name << "\n";
                } else if (auto funcDef = std::dynamic_pointer_cast<FunctionDefinition>(node)) {
                    std::cout << "FuncDef: " << funcDef->name << "\nBody: ";
                    for (const auto& line : funcDef->body) {
                        std::cout << line.value << " ";
                    }
                    std::cout << "\n";
                }
            }
            std::cout << "\n";
        }

        auto glsl = CodeGenerator::generate(ast);

        std::cout << "Generated GLSL:\n";
        for (const auto& [type, code] : glsl) {
            std::cout << "==== " << type << " SHADER ====\n" << code << "\n\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Compilation Error:\n" << e.what() << "\n";
    }
}

}  // namespace Neko