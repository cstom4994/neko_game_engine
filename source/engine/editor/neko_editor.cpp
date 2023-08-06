
#include "neko_editor.hpp"

#include "engine/gui/neko_imgui_utils.hpp"

namespace neko {

#define __neko_gl_to_string_def(x) \
    case x:                        \
        return #x;                 \
        break;

const char *neko_gpu_glenum_string(GLenum e) {
    switch (e) {
        // shader:
        __neko_gl_to_string_def(GL_VERTEX_SHADER);
        __neko_gl_to_string_def(GL_GEOMETRY_SHADER);
        __neko_gl_to_string_def(GL_FRAGMENT_SHADER);

        // buffer usage:
        __neko_gl_to_string_def(GL_STREAM_DRAW);
        __neko_gl_to_string_def(GL_STREAM_READ);
        __neko_gl_to_string_def(GL_STREAM_COPY);
        __neko_gl_to_string_def(GL_STATIC_DRAW);
        __neko_gl_to_string_def(GL_STATIC_READ);
        __neko_gl_to_string_def(GL_STATIC_COPY);
        __neko_gl_to_string_def(GL_DYNAMIC_DRAW);
        __neko_gl_to_string_def(GL_DYNAMIC_READ);
        __neko_gl_to_string_def(GL_DYNAMIC_COPY);

        // errors:
        __neko_gl_to_string_def(GL_NO_ERROR);
        __neko_gl_to_string_def(GL_INVALID_ENUM);
        __neko_gl_to_string_def(GL_INVALID_VALUE);
        __neko_gl_to_string_def(GL_INVALID_OPERATION);
        __neko_gl_to_string_def(GL_INVALID_FRAMEBUFFER_OPERATION);
        __neko_gl_to_string_def(GL_OUT_OF_MEMORY);
        __neko_gl_to_string_def(GL_STACK_UNDERFLOW);
        __neko_gl_to_string_def(GL_STACK_OVERFLOW);

        // types:
        __neko_gl_to_string_def(GL_BYTE);
        __neko_gl_to_string_def(GL_UNSIGNED_BYTE);
        __neko_gl_to_string_def(GL_SHORT);
        __neko_gl_to_string_def(GL_UNSIGNED_SHORT);
        __neko_gl_to_string_def(GL_FLOAT);
        __neko_gl_to_string_def(GL_FLOAT_VEC2);
        __neko_gl_to_string_def(GL_FLOAT_VEC3);
        __neko_gl_to_string_def(GL_FLOAT_VEC4);
        __neko_gl_to_string_def(GL_DOUBLE);
        __neko_gl_to_string_def(GL_DOUBLE_VEC2);
        __neko_gl_to_string_def(GL_DOUBLE_VEC3);
        __neko_gl_to_string_def(GL_DOUBLE_VEC4);
        __neko_gl_to_string_def(GL_INT);
        __neko_gl_to_string_def(GL_INT_VEC2);
        __neko_gl_to_string_def(GL_INT_VEC3);
        __neko_gl_to_string_def(GL_INT_VEC4);
        __neko_gl_to_string_def(GL_UNSIGNED_INT);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_VEC2);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_VEC3);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_VEC4);
        __neko_gl_to_string_def(GL_BOOL);
        __neko_gl_to_string_def(GL_BOOL_VEC2);
        __neko_gl_to_string_def(GL_BOOL_VEC3);
        __neko_gl_to_string_def(GL_BOOL_VEC4);
        __neko_gl_to_string_def(GL_FLOAT_MAT2);
        __neko_gl_to_string_def(GL_FLOAT_MAT3);
        __neko_gl_to_string_def(GL_FLOAT_MAT4);
        __neko_gl_to_string_def(GL_FLOAT_MAT2x3);
        __neko_gl_to_string_def(GL_FLOAT_MAT2x4);
        __neko_gl_to_string_def(GL_FLOAT_MAT3x2);
        __neko_gl_to_string_def(GL_FLOAT_MAT3x4);
        __neko_gl_to_string_def(GL_FLOAT_MAT4x2);
        __neko_gl_to_string_def(GL_FLOAT_MAT4x3);
        __neko_gl_to_string_def(GL_DOUBLE_MAT2);
        __neko_gl_to_string_def(GL_DOUBLE_MAT3);
        __neko_gl_to_string_def(GL_DOUBLE_MAT4);
        __neko_gl_to_string_def(GL_DOUBLE_MAT2x3);
        __neko_gl_to_string_def(GL_DOUBLE_MAT2x4);
        __neko_gl_to_string_def(GL_DOUBLE_MAT3x2);
        __neko_gl_to_string_def(GL_DOUBLE_MAT3x4);
        __neko_gl_to_string_def(GL_DOUBLE_MAT4x2);
        __neko_gl_to_string_def(GL_DOUBLE_MAT4x3);
        __neko_gl_to_string_def(GL_SAMPLER_1D);
        __neko_gl_to_string_def(GL_SAMPLER_2D);
        __neko_gl_to_string_def(GL_SAMPLER_3D);
        __neko_gl_to_string_def(GL_SAMPLER_CUBE);
        __neko_gl_to_string_def(GL_SAMPLER_1D_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_2D_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_1D_ARRAY);
        __neko_gl_to_string_def(GL_SAMPLER_2D_ARRAY);
        __neko_gl_to_string_def(GL_SAMPLER_1D_ARRAY_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_2D_ARRAY_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_SAMPLER_CUBE_SHADOW);
        __neko_gl_to_string_def(GL_SAMPLER_BUFFER);
        __neko_gl_to_string_def(GL_SAMPLER_2D_RECT);
        __neko_gl_to_string_def(GL_SAMPLER_2D_RECT_SHADOW);
        __neko_gl_to_string_def(GL_INT_SAMPLER_1D);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D);
        __neko_gl_to_string_def(GL_INT_SAMPLER_3D);
        __neko_gl_to_string_def(GL_INT_SAMPLER_CUBE);
        __neko_gl_to_string_def(GL_INT_SAMPLER_1D_ARRAY);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D_ARRAY);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_INT_SAMPLER_BUFFER);
        __neko_gl_to_string_def(GL_INT_SAMPLER_2D_RECT);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_1D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_3D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_CUBE);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_BUFFER);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_SAMPLER_2D_RECT);

        __neko_gl_to_string_def(GL_IMAGE_1D);
        __neko_gl_to_string_def(GL_IMAGE_2D);
        __neko_gl_to_string_def(GL_IMAGE_3D);
        __neko_gl_to_string_def(GL_IMAGE_2D_RECT);
        __neko_gl_to_string_def(GL_IMAGE_CUBE);
        __neko_gl_to_string_def(GL_IMAGE_BUFFER);
        __neko_gl_to_string_def(GL_IMAGE_1D_ARRAY);
        __neko_gl_to_string_def(GL_IMAGE_2D_ARRAY);
        __neko_gl_to_string_def(GL_IMAGE_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_IMAGE_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_INT_IMAGE_1D);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D);
        __neko_gl_to_string_def(GL_INT_IMAGE_3D);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D_RECT);
        __neko_gl_to_string_def(GL_INT_IMAGE_CUBE);
        __neko_gl_to_string_def(GL_INT_IMAGE_BUFFER);
        __neko_gl_to_string_def(GL_INT_IMAGE_1D_ARRAY);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D_ARRAY);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_1D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_3D);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D_RECT);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_CUBE);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_BUFFER);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        __neko_gl_to_string_def(GL_UNSIGNED_INT_ATOMIC_COUNTER);
    }

    static char buffer[32];
    std::sprintf(buffer, "Unknown GLenum: (0x%04x)", e);
    return buffer;
}

void neko_editor_render_uniform_variable(GLuint program, GLenum type, const char *name, GLint location) {
    static bool is_color = false;
    switch (type) {
        case GL_FLOAT:
            neko_editor_shader_inspector_gen_var(GLfloat, 1, GL_FLOAT, glGetUniformfv, glProgramUniform1fv, ImGui::DragFloat);
            break;

        case GL_FLOAT_VEC2:
            neko_editor_shader_inspector_gen_var(GLfloat, 2, GL_FLOAT_VEC2, glGetUniformfv, glProgramUniform2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_VEC3: {
            ImGui::Checkbox("##is_color", &is_color);
            ImGui::SameLine();
            ImGui::Text("GL_FLOAT_VEC3 %s", name);
            ImGui::SameLine();
            float value[3];
            glGetUniformfv(program, location, &value[0]);
            if ((!is_color && ImGui::DragFloat3("", &value[0])) || (is_color && ImGui::ColorEdit3("Color", &value[0], ImGuiColorEditFlags_NoLabel)))
                glProgramUniform3fv(program, location, 1, &value[0]);
        } break;

        case GL_FLOAT_VEC4: {
            ImGui::Checkbox("##is_color", &is_color);
            ImGui::SameLine();
            ImGui::Text("GL_FLOAT_VEC4 %s", name);
            ImGui::SameLine();
            float value[4];
            glGetUniformfv(program, location, &value[0]);
            if ((!is_color && ImGui::DragFloat4("", &value[0])) ||
                (is_color && ImGui::ColorEdit4("Color", &value[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)))
                glProgramUniform4fv(program, location, 1, &value[0]);
        } break;

        case GL_INT:
            neko_editor_shader_inspector_gen_var(GLint, 1, GL_INT, glGetUniformiv, glProgramUniform1iv, ImGui::DragInt);
            break;

        case GL_INT_VEC2:
            neko_editor_shader_inspector_gen_var(GLint, 2, GL_INT, glGetUniformiv, glProgramUniform2iv, ImGui::DragInt2);
            break;

        case GL_INT_VEC3:
            neko_editor_shader_inspector_gen_var(GLint, 3, GL_INT, glGetUniformiv, glProgramUniform3iv, ImGui::DragInt3);
            break;

        case GL_INT_VEC4:
            neko_editor_shader_inspector_gen_var(GLint, 4, GL_INT, glGetUniformiv, glProgramUniform4iv, ImGui::DragInt4);
            break;

        case GL_UNSIGNED_INT: {
            ImGui::Text("GL_UNSIGNED_INT %s:", name);
            ImGui::SameLine();
            GLuint value[1];
            glGetUniformuiv(program, location, &value[0]);
            if (ImGui::DragScalar("", ImGuiDataType_U32, &value[0], 0.25f)) glProgramUniform1uiv(program, location, 1, &value[0]);
        } break;

        case GL_UNSIGNED_INT_VEC3: {
            ImGui::Text("GL_UNSIGNED_INT_VEC3 %s:", name);
            ImGui::SameLine();
            GLuint value[1];
            glGetUniformuiv(program, location, &value[0]);
            if (ImGui::DragScalarN("", ImGuiDataType_U32, &value[0], 3, 0.25f)) glProgramUniform3uiv(program, location, 1, &value[0]);
        } break;

        case GL_SAMPLER_2D:
            neko_editor_shader_inspector_gen_var(GLint, 1, GL_SAMPLER_2D, glGetUniformiv, glProgramUniform1iv, ImGui::DragInt);
            break;

        case GL_FLOAT_MAT2:
            neko_editor_shader_inspector_gen_mat(GLfloat, 2, 2, GL_FLOAT_MAT2, glGetUniformfv, glProgramUniformMatrix2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_MAT3:
            neko_editor_shader_inspector_gen_mat(GLfloat, 3, 3, GL_FLOAT_MAT3, glGetUniformfv, glProgramUniformMatrix3fv, ImGui::DragFloat3);
            break;

        case GL_FLOAT_MAT4:
            neko_editor_shader_inspector_gen_mat(GLfloat, 4, 4, GL_FLOAT_MAT4, glGetUniformfv, glProgramUniformMatrix4fv, ImGui::DragFloat4);
            break;

        case GL_FLOAT_MAT2x3:
            neko_editor_shader_inspector_gen_mat(GLfloat, 3, 2, GL_FLOAT_MAT2x3, glGetUniformfv, glProgramUniformMatrix2x3fv, ImGui::DragFloat3);
            break;

        case GL_FLOAT_MAT2x4:
            neko_editor_shader_inspector_gen_mat(GLfloat, 4, 2, GL_FLOAT_MAT2x4, glGetUniformfv, glProgramUniformMatrix2x4fv, ImGui::DragFloat4);
            break;

        case GL_FLOAT_MAT3x2:
            neko_editor_shader_inspector_gen_mat(GLfloat, 2, 3, GL_FLOAT_MAT3x2, glGetUniformfv, glProgramUniformMatrix3x2fv, ImGui::DragFloat2);
            break;

        case GL_FLOAT_MAT3x4:
            neko_editor_shader_inspector_gen_mat(GLfloat, 4, 3, GL_FLOAT_MAT3x4, glGetUniformfv, glProgramUniformMatrix3x2fv, ImGui::DragFloat4);
            break;

        case GL_BOOL: {
            ImGui::Text("GL_BOOL %s:", name);
            ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            if (ImGui::Checkbox("", (bool *)&value)) glProgramUniform1uiv(program, location, 1, &value);
        } break;

        case GL_IMAGE_2D: {
            ImGui::Text("GL_IMAGE_2D %s:", name);
            // ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            // if (ImGui::Checkbox("", (bool*)&value)) glProgramUniform1iv(program, location, 1, &value);
            ImGui::Image((void *)(intptr_t)value, ImVec2(256, 256));
        } break;

        case GL_SAMPLER_CUBE: {
            ImGui::Text("GL_SAMPLER_CUBE %s:", name);
            // ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            ImGui::Image((void *)(intptr_t)value, ImVec2(256, 256));
        } break;

        default:
            ImGui::TextColored(neko_rgba2imvec(255, 64, 64), "%s has type %s, which isn't supported yet!", name, neko_gpu_glenum_string(type));
            break;
    }
}

neko_inline float get_scrollable_height() { return ImGui::GetTextLineHeight() * 16; }

void neko_editor_inspect_shader(const char *label, GLuint program) {
    neko_assert(label != nullptr, ("The label supplied with program: {} is nullptr", program));
    neko_assert(glIsProgram(program), ("The program: {} is not a valid shader program", program));

    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
        // Uniforms
        ImGui::Indent();
        if (ImGui::CollapsingHeader("Uniforms", ImGuiTreeNodeFlags_DefaultOpen)) {
            GLint uniform_count;
            glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

            // Read the length of the longest active uniform.
            GLint max_name_length;
            glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_length);

            static std::vector<char> name;
            name.resize(max_name_length);

            for (int i = 0; i < uniform_count; i++) {
                GLint ignored;
                GLenum type;
                glGetActiveUniform(program, i, max_name_length, nullptr, &ignored, &type, name.data());

                const auto location = glGetUniformLocation(program, name.data());
                ImGui::Indent();
                ImGui::PushID(i);
                ImGui::PushItemWidth(-1.0f);
                neko_editor_render_uniform_variable(program, type, name.data(), location);
                ImGui::PopItemWidth();
                ImGui::PopID();
                ImGui::Unindent();
            }
        }
        ImGui::Unindent();

        // Shaders
        ImGui::Indent();
        if (ImGui::CollapsingHeader("Shaders")) {
            GLint shader_count;
            glGetProgramiv(program, GL_ATTACHED_SHADERS, &shader_count);

            static std::vector<GLuint> attached_shaders;
            attached_shaders.resize(shader_count);
            glGetAttachedShaders(program, shader_count, nullptr, attached_shaders.data());

            for (const auto &shader : attached_shaders) {
                GLint source_length = 0;
                glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &source_length);
                static std::vector<char> source;
                source.resize(source_length);
                glGetShaderSource(shader, source_length, nullptr, source.data());

                GLint type = 0;
                glGetShaderiv(shader, GL_SHADER_TYPE, &type);

                ImGui::Indent();
                auto string_type = neko_gpu_glenum_string(type);
                ImGui::PushID(string_type);
                if (ImGui::CollapsingHeader(string_type)) {
                    auto y_size = std::min(ImGui::CalcTextSize(source.data()).y, get_scrollable_height());
                    ImGui::InputTextMultiline("", source.data(), source.size(), ImVec2(-1.0f, y_size), ImGuiInputTextFlags_ReadOnly);
                }
                ImGui::PopID();
                ImGui::Unindent();
            }
        }
        ImGui::Unindent();
    }
    ImGui::PopID();
}

void neko_editor_inspect_vertex_array(const char *label, GLuint vao) {
    neko_assert(label != nullptr, ("The label supplied with VAO: %u is nullptr", vao));
    neko_assert(glIsVertexArray(vao), ("The VAO: %u is not a valid vertex array object", vao));

    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
        ImGui::Indent();

        // Get current bound vertex buffer object so we can reset it back once we are finished.
        GLint current_vbo = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_vbo);

        // Get current bound vertex array object so we can reset it back once we are finished.
        GLint current_vao = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
        glBindVertexArray(vao);

        // Get the maximum number of vertex attributes,
        // minimum is 4, I have 16, means that whatever number of attributes is here, it should be reasonable to iterate over.
        GLint max_vertex_attribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);

        GLint ebo = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo);

        // EBO Visualization
        char buffer[128];
        std::sprintf(buffer, "Element Array Buffer: %d", ebo);
        ImGui::PushID(buffer);
        if (ImGui::CollapsingHeader(buffer)) {
            ImGui::Indent();
            // Assuming unsigned int atm, as I have not found a way to get out the type of the element array buffer.
            int size = 0;
            glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
            size /= sizeof(GLuint);
            ImGui::Text("Size: %d", size);

            if (ImGui::TreeNode("Buffer Contents")) {
                // TODO: Find a better way to put this out on screen, because this solution will probably not scale good when we get a lot of indices.
                //       Possible solution: Make it into columns, like the VBO's, and present the indices as triangles.
                auto ptr = (GLuint *)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
                for (int i = 0; i < size; i++) {
                    ImGui::Text("%u", ptr[i]);
                    ImGui::SameLine();
                    if ((i + 1) % 3 == 0) ImGui::NewLine();
                }

                glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

                ImGui::TreePop();
            }

            ImGui::Unindent();
        }
        ImGui::PopID();

        // VBO Visualization
        for (intptr_t i = 0; i < max_vertex_attribs; i++) {
            GLint enabled = 0;
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

            if (!enabled) continue;

            std::sprintf(buffer, "Attribute: %" PRIdPTR "", i);
            ImGui::PushID(buffer);
            if (ImGui::CollapsingHeader(buffer)) {
                ImGui::Indent();
                // Display meta data
                GLint buffer = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
                ImGui::Text("Buffer: %d", buffer);

                GLint type = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
                ImGui::Text("Type: %s", neko_gpu_glenum_string(type));

                GLint dimensions = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &dimensions);
                ImGui::Text("Dimensions: %d", dimensions);

                // Need to bind buffer to get access to parameteriv, and for mapping later
                glBindBuffer(GL_ARRAY_BUFFER, buffer);

                GLint size = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
                ImGui::Text("Size in bytes: %d", size);

                GLint stride = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
                ImGui::Text("Stride in bytes: %d", stride);

                GLvoid *offset = nullptr;
                glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &offset);
                ImGui::Text("Offset in bytes: %" PRIdPTR "", (intptr_t)offset);

                GLint usage = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_USAGE, &usage);
                ImGui::Text("Usage: %s", neko_gpu_glenum_string(usage));

                // Create table with indexes and actual contents
                if (ImGui::TreeNode("Buffer Contents")) {
                    ImGui::BeginChild(ImGui::GetID("vbo contents"), ImVec2(-1.0f, get_scrollable_height()), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                    ImGui::Columns(dimensions + 1);
                    const char *descriptors[] = {"index", "x", "y", "z", "w"};
                    for (int j = 0; j < dimensions + 1; j++) {
                        ImGui::Text("%s", descriptors[j]);
                        ImGui::NextColumn();
                    }
                    ImGui::Separator();

                    auto ptr = (char *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY) + (intptr_t)offset;
                    for (int j = 0, c = 0; j < size; j += stride, c++) {
                        ImGui::Text("%d", c);
                        ImGui::NextColumn();
                        for (int k = 0; k < dimensions; k++) {
                            switch (type) {
                                case GL_BYTE:
                                    ImGui::Text("% d", *(GLbyte *)&ptr[j + k * sizeof(GLbyte)]);
                                    break;
                                case GL_UNSIGNED_BYTE:
                                    ImGui::Text("%u", *(GLubyte *)&ptr[j + k * sizeof(GLubyte)]);
                                    break;
                                case GL_SHORT:
                                    ImGui::Text("% d", *(GLshort *)&ptr[j + k * sizeof(GLshort)]);
                                    break;
                                case GL_UNSIGNED_SHORT:
                                    ImGui::Text("%u", *(GLushort *)&ptr[j + k * sizeof(GLushort)]);
                                    break;
                                case GL_INT:
                                    ImGui::Text("% d", *(GLint *)&ptr[j + k * sizeof(GLint)]);
                                    break;
                                case GL_UNSIGNED_INT:
                                    ImGui::Text("%u", *(GLuint *)&ptr[j + k * sizeof(GLuint)]);
                                    break;
                                case GL_FLOAT:
                                    ImGui::Text("% f", *(GLfloat *)&ptr[j + k * sizeof(GLfloat)]);
                                    break;
                                case GL_DOUBLE:
                                    ImGui::Text("% f", *(GLdouble *)&ptr[j + k * sizeof(GLdouble)]);
                                    break;
                            }
                            ImGui::NextColumn();
                        }
                    }
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                    ImGui::EndChild();
                    ImGui::TreePop();
                }
                ImGui::Unindent();
            }
            ImGui::PopID();
        }

        // Cleanup
        glBindVertexArray(current_vao);
        glBindBuffer(GL_ARRAY_BUFFER, current_vbo);

        ImGui::Unindent();
    }
    ImGui::PopID();
}

}  // namespace neko