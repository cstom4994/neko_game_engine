
#include "engine/renderer/shader_parser.h"
#include "base/common/string.hpp"
#include "base/common/reflection.hpp"

using namespace Neko;
using namespace Neko::shader;
using namespace Neko::reflection;

int Test_Shader() {

    HashMap<String> tokenTypeNames;
    guess_enum_range<TokenType, 0>(tokenTypeNames, std::make_integer_sequence<int, (int)TokenType::OTHER + 1>());

    std::string source = R"(
#nekoshader version 1
#begin VERTEX
@version 330

@attribute vec2 position;
@attribute vec2 uv;

@varying vec2 out_uv;

@uniform mat4 projection;
@uniform mat4 transform;

//@varying VS_OUT {
//	vec4 color;
//	vec2 uv;
//	float use_texture;
//} vs_out;

struct MyStruct {
    vec2 uv;
    float f;
} myStruct;

int myfunc(float f, mat3 m3) {
	return 114514;
}

void main() {
	gl_Position = projection * transform * vec4(position, 0.0, 1.0);
	out_uv = uv;
}

#end VERTEX

#begin FRAGMENT
@version 330

@varying vec2 out_uv;
@out vec4 color;

@uniform sampler2D image;

void main() {
	color = texture(image, out_uv);
}

#end FRAGMENT

    )";

    Lexer lexer(source);
    try {
        auto tokens = lexer.tokenize();
        Parser parser(tokens);

        std::cout << "Tokens:\n";
        for (const auto& token : tokens) {
            std::string name = tokenTypeNames[(int)token.type].cstr();
            name = name.substr(14, name.length());
            std::cout << std::setw(20) << std::left << name         //
                      << std::setw(20) << std::left << token.value  //
                      << "\n";
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
                    std::cout << "FuncDef: " << funcDef->name << " -> " << funcDef->returnType << "\nBody: ";
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
            std::cout << "=== " << type << " SHADER ===\n" << code << "\n\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Compilation Error:\n" << e.what() << "\n";
    }

    return 0;
}