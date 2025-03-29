
#include "shader.h"

#include "base/common/string.hpp"
#include "base/common/logger.hpp"

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
#include <variant>

namespace Neko {

namespace shader {

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
    SWITCH,
    CASE,
    DEFAULT,
    BIT_AND,
    BIT_OR,
    BIT_SHIFT_LEFT,
    BIT_SHIFT_RIGHT,
    COLON,
    TAG,
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

    void advance();
    bool match(const std::string& keyword);
    std::string parseNumber();

public:
    explicit Lexer(const std::string& src) : source(src) {}

    std::vector<Token> tokenize();
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
    std::unordered_map<std::string, std::string> variableTypes;

    Token consume(TokenType expected, const std::string& msg);

    Token consumeType();

    bool match(TokenType type);

public:
    explicit Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}

    std::unordered_map<std::string, std::vector<std::shared_ptr<ASTNode>>> parse();

    std::vector<std::shared_ptr<ASTNode>> parseBlock();

private:
    size_t findEnd(const std::string& type);

    bool isFunction();

    void parseVarDecl(std::vector<std::shared_ptr<ASTNode>>& nodes);

    std::shared_ptr<StructDefinition> parseStruct();

    std::shared_ptr<FunctionDefinition> parseFunction();

    void parseMacro(std::vector<std::shared_ptr<ASTNode>>& nodes);

    void parseVersion(std::vector<std::shared_ptr<ASTNode>>& nodes);
};

class CodeGenerator {
    static void generateVarDecl(std::ostream& os, const VariableDeclaration& var, int& loc, const std::string& shaderType);

    static bool needsSpace(const Token& prev, const Token& curr);

public:
    static std::unordered_map<std::string, std::string> generate(const std::unordered_map<std::string, std::vector<std::shared_ptr<ASTNode>>>& blocks);
};

}  // namespace shader

}  // namespace Neko