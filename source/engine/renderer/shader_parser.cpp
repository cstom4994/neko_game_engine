
#include "shader_parser.h"

namespace Neko {

namespace shader {

// clang-format off
const static std::vector<std::variant<
    std::tuple<std::string, TokenType>, 
    std::tuple<std::string, TokenType, std::string>
>> shader_keywords = {
    std::make_tuple("@attribute", TokenType::ATTRIBUTE),
    std::make_tuple("@uniform", TokenType::UNIFORM),
    std::make_tuple("@varying", TokenType::VARYING),
    std::make_tuple("@version", TokenType::VERSION),
    std::make_tuple("@out", TokenType::OUT),
    std::make_tuple("@define", TokenType::DEFINE, "#define"),
    std::make_tuple("#nekoshader", TokenType::TAG),
    std::make_tuple("#begin", TokenType::BEGIN),
    std::make_tuple("#end", TokenType::END),
    std::make_tuple("struct", TokenType::STRUCT),
    std::make_tuple("switch", TokenType::SWITCH),
    std::make_tuple("case", TokenType::CASE),
    std::make_tuple("default", TokenType::DEFAULT),
    std::make_tuple("void", TokenType::TYPE),
    std::make_tuple("float", TokenType::TYPE),
    std::make_tuple("int", TokenType::TYPE),
    std::make_tuple("vec2", TokenType::TYPE),
    std::make_tuple("vec3", TokenType::TYPE),
    std::make_tuple("vec4", TokenType::TYPE),
    std::make_tuple("mat3", TokenType::TYPE),
    std::make_tuple("mat4", TokenType::TYPE),
    std::make_tuple("sampler2D", TokenType::TYPE),
    std::make_tuple("bool", TokenType::TYPE), 
    std::make_tuple("uint", TokenType::TYPE), 
    std::make_tuple("uvec2", TokenType::TYPE), 
    std::make_tuple("uvec3", TokenType::TYPE),
    std::make_tuple("uvec4", TokenType::TYPE),
};
// clang-format on

void Lexer::advance() {
    if (position >= source.length()) return;
    if (source[position] == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    position++;
}

bool Lexer::match(const std::string& keyword) {
    if (source.substr(position, keyword.size()) == keyword) {
        for (size_t i = 0; i < keyword.size(); ++i) advance();
        return true;
    }
    return false;
}

std::string Lexer::parseNumber() {
    size_t start = position;
    bool has_dot = false;
    while (position < source.size() &&
           (std::isdigit(source[position]) || (source[position] == '.' && !has_dot) || (tolower(source[position]) == 'e' || source[position] == '+' || source[position] == '-'))) {
        if (source[position] == '.') has_dot = true;
        advance();
    }
    return source.substr(start, position - start);
}

std::vector<Token> Lexer::tokenize() {
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
        for (const auto& keyword : shader_keywords) {
            if (std::holds_alternative<std::tuple<std::string, TokenType>>(keyword)) {
                const auto& [kw, tt] = std::get<std::tuple<std::string, TokenType>>(keyword);
                if (match(kw)) {
                    tokens.push_back({tt, kw, start_line, start_col});
                    matched = true;
                    break;
                }
            } else if (std::holds_alternative<std::tuple<std::string, TokenType, std::string>>(keyword)) {
                const auto& [kw, tt, extra] = std::get<std::tuple<std::string, TokenType, std::string>>(keyword);
                if (match(kw)) {
                    tokens.push_back({tt, extra, start_line, start_col});
                    matched = true;
                    break;
                }
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
        } else if (c == ':') {
            tokens.push_back({TokenType::COLON, ":", start_line, start_col});
            advance();
        } else if (c == '<') {
            if (position + 1 < source.size() && source[position + 1] == '<') {
                tokens.push_back({TokenType::BIT_SHIFT_LEFT, "<<", start_line, start_col});
                advance();
                advance();
            } else {
                tokens.push_back({TokenType::OTHER, "<", start_line, start_col});
                advance();
            }
        } else if (c == '>') {
            if (position + 1 < source.size() && source[position + 1] == '>') {
                tokens.push_back({TokenType::BIT_SHIFT_RIGHT, ">>", start_line, start_col});
                advance();
                advance();
            } else {
                tokens.push_back({TokenType::OTHER, ">", start_line, start_col});
                advance();
            }
        } else if (c == '&') {
            tokens.push_back({TokenType::BIT_AND, "&", start_line, start_col});
            advance();
        } else if (c == '|') {
            tokens.push_back({TokenType::BIT_OR, "|", start_line, start_col});
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

Token Parser::consume(TokenType expected, const std::string& msg) {
    if (index >= tokens.size()) throw std::runtime_error(msg + " at EOF");
    Token& t = tokens[index];
    if (t.type != expected) {
        throw std::runtime_error(msg + "\n  at line " + std::to_string(t.line) + ":" + std::to_string(t.column) + "\n  current token: " + t.value);
    }
    return tokens[index++];
}

Token Parser::consumeType() {
    if (index >= tokens.size()) throw std::runtime_error("Expected type at EOF");
    Token& t = tokens[index];
    if (t.type != TokenType::TYPE && t.type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected type, got " + t.value + " at line " + std::to_string(t.line) + ":" + std::to_string(t.column));
    }
    return tokens[index++];
}

bool Parser::match(TokenType type) {
    if (index < tokens.size() && tokens[index].type == type) {
        index++;
        return true;
    }
    return false;
}

std::unordered_map<std::string, std::vector<std::shared_ptr<ASTNode>>> Parser::parse() {
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

std::vector<std::shared_ptr<ASTNode>> Parser::parseBlock() {
    std::vector<std::shared_ptr<ASTNode>> nodes;
    while (index < tokens.size()) {
        if (match(TokenType::DEFINE)) {
            parseMacro(nodes);
        } else if (match(TokenType::VERSION)) {
            parseVersion(nodes);
        } else if (match(TokenType::OUT) || match(TokenType::ATTRIBUTE) || match(TokenType::UNIFORM) || match(TokenType::VARYING)) {
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

size_t Parser::findEnd(const std::string& type) {
    for (size_t i = index; i < tokens.size(); ++i) {
        if (tokens[i].type == TokenType::END && i + 1 < tokens.size() && tokens[i + 1].type == TokenType::IDENTIFIER && tokens[i + 1].value == type) {
            return i;
        }
    }
    throw std::runtime_error("Missing #end for " + type);
}

bool Parser::isFunction() {
    if (index + 2 >= tokens.size()) return false;
    return (tokens[index].type == TokenType::TYPE || tokens[index].type == TokenType::IDENTIFIER) && tokens[index + 1].type == TokenType::IDENTIFIER && tokens[index + 2].type == TokenType::LEFT_PAREN;
}

void Parser::parseVarDecl(std::vector<std::shared_ptr<ASTNode>>& nodes) {
    const TokenType qual = tokens[index - 1].type;
    Token type = consumeType();
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    variableTypes[name.value] = type.value;

    std::string arraySize;
    if (match(TokenType::LEFT_BRACKET)) {
        Token size = consume(TokenType::NUMBER, "Expected array size");
        arraySize = size.value;
        consume(TokenType::RIGHT_BRACKET, "Expected ']' after array size");
    }

    consume(TokenType::SEMICOLON, "Expected ';' after declaration");
    nodes.push_back(std::make_shared<VariableDeclaration>(type.value, name.value, qual, arraySize));
}

std::shared_ptr<StructDefinition> Parser::parseStruct() {
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

std::shared_ptr<FunctionDefinition> Parser::parseFunction() {
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

void Parser::parseMacro(std::vector<std::shared_ptr<ASTNode>>& nodes) {
    Token nameToken = consume(TokenType::IDENTIFIER, "Expected macro name");
    std::string value;
    while (index < tokens.size() && tokens[index].line == nameToken.line /*&& tokens[index].type != TokenType::END_OF_LINE*/) {
        value += tokens[index++].value + " ";
    }
    if (!value.empty()) value.pop_back();  // 移除末尾空格
    nodes.push_back(std::make_shared<MacroDefinition>(nameToken.value, value));
}

void Parser::parseVersion(std::vector<std::shared_ptr<ASTNode>>& nodes) {
    Token versionToken = consume(TokenType::NUMBER, "Expected version number");
    nodes.push_back(std::make_shared<VersionDirective>(versionToken.value));
}

void CodeGenerator::generateVarDecl(std::ostream& os, const VariableDeclaration& var, int& loc, const std::string& shaderType) {
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

bool CodeGenerator::needsSpace(const Token& prev, const Token& curr) {
    const static std::string noSpaceBefore = ".,)]};:";
    const static std::string noSpaceAfter = "([{";
    bool needs = !(noSpaceBefore.find(curr.value[0]) != std::string::npos || noSpaceAfter.find(prev.value[0]) != std::string::npos);
    return needs && !(prev.type == TokenType::BIT_SHIFT_LEFT && curr.type == TokenType::NUMBER);
}

std::unordered_map<std::string, std::string> CodeGenerator::generate(const std::unordered_map<std::string, std::vector<std::shared_ptr<ASTNode>>>& blocks) {
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

std::unordered_map<std::string, std::string> ShaderParse(String src) {
    Lexer lexer(src.cstr());
    try {
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();
        auto glsl = CodeGenerator::generate(ast);
        return glsl;
    } catch (const std::exception& e) {
        LOG_ERROR("shader parser error: {}", e.what());
    }
    return {};
#if 0
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
#endif
}

}  // namespace shader

}  // namespace Neko