#ifndef GLEW_GLFW_H
#define GLEW_GLFW_H

// 确保在 GLFW 标头之前包含 GLEW 标头
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>
#include <stdio.h>

static const char* neko_opengl_string(GLenum e) {

#define XX(x)      \
    case x:        \
        return #x; \
        break;

    switch (e) {
        // shader:
        XX(GL_VERTEX_SHADER);
        XX(GL_GEOMETRY_SHADER);
        XX(GL_FRAGMENT_SHADER);

        // buffer usage:
        XX(GL_STREAM_DRAW);
        XX(GL_STREAM_READ);
        XX(GL_STREAM_COPY);
        XX(GL_STATIC_DRAW);
        XX(GL_STATIC_READ);
        XX(GL_STATIC_COPY);
        XX(GL_DYNAMIC_DRAW);
        XX(GL_DYNAMIC_READ);
        XX(GL_DYNAMIC_COPY);

        // errors:
        XX(GL_NO_ERROR);
        XX(GL_INVALID_ENUM);
        XX(GL_INVALID_VALUE);
        XX(GL_INVALID_OPERATION);
        XX(GL_INVALID_FRAMEBUFFER_OPERATION);
        XX(GL_OUT_OF_MEMORY);

#if !defined(NEKO_IS_APPLE)
        XX(GL_STACK_UNDERFLOW);
        XX(GL_STACK_OVERFLOW);
#endif

        // types:
        XX(GL_BYTE);
        XX(GL_UNSIGNED_BYTE);
        XX(GL_SHORT);
        XX(GL_UNSIGNED_SHORT);
        XX(GL_FLOAT);
        XX(GL_FLOAT_VEC2);
        XX(GL_FLOAT_VEC3);
        XX(GL_FLOAT_VEC4);
        XX(GL_DOUBLE);
        XX(GL_DOUBLE_VEC2);
        XX(GL_DOUBLE_VEC3);
        XX(GL_DOUBLE_VEC4);
        XX(GL_INT);
        XX(GL_INT_VEC2);
        XX(GL_INT_VEC3);
        XX(GL_INT_VEC4);
        XX(GL_UNSIGNED_INT);
        XX(GL_UNSIGNED_INT_VEC2);
        XX(GL_UNSIGNED_INT_VEC3);
        XX(GL_UNSIGNED_INT_VEC4);
        XX(GL_BOOL);
        XX(GL_BOOL_VEC2);
        XX(GL_BOOL_VEC3);
        XX(GL_BOOL_VEC4);
        XX(GL_FLOAT_MAT2);
        XX(GL_FLOAT_MAT3);
        XX(GL_FLOAT_MAT4);
        XX(GL_FLOAT_MAT2x3);
        XX(GL_FLOAT_MAT2x4);
        XX(GL_FLOAT_MAT3x2);
        XX(GL_FLOAT_MAT3x4);
        XX(GL_FLOAT_MAT4x2);
        XX(GL_FLOAT_MAT4x3);
        XX(GL_DOUBLE_MAT2);
        XX(GL_DOUBLE_MAT3);
        XX(GL_DOUBLE_MAT4);
        XX(GL_DOUBLE_MAT2x3);
        XX(GL_DOUBLE_MAT2x4);
        XX(GL_DOUBLE_MAT3x2);
        XX(GL_DOUBLE_MAT3x4);
        XX(GL_DOUBLE_MAT4x2);
        XX(GL_DOUBLE_MAT4x3);
        XX(GL_SAMPLER_1D);
        XX(GL_SAMPLER_2D);
        XX(GL_SAMPLER_3D);
        XX(GL_SAMPLER_CUBE);
        XX(GL_SAMPLER_1D_SHADOW);
        XX(GL_SAMPLER_2D_SHADOW);
        XX(GL_SAMPLER_1D_ARRAY);
        XX(GL_SAMPLER_2D_ARRAY);
        XX(GL_SAMPLER_1D_ARRAY_SHADOW);
        XX(GL_SAMPLER_2D_ARRAY_SHADOW);
        XX(GL_SAMPLER_2D_MULTISAMPLE);
        XX(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_SAMPLER_CUBE_SHADOW);
        XX(GL_SAMPLER_BUFFER);
        XX(GL_SAMPLER_2D_RECT);
        XX(GL_SAMPLER_2D_RECT_SHADOW);
        XX(GL_INT_SAMPLER_1D);
        XX(GL_INT_SAMPLER_2D);
        XX(GL_INT_SAMPLER_3D);
        XX(GL_INT_SAMPLER_CUBE);
        XX(GL_INT_SAMPLER_1D_ARRAY);
        XX(GL_INT_SAMPLER_2D_ARRAY);
        XX(GL_INT_SAMPLER_2D_MULTISAMPLE);
        XX(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_INT_SAMPLER_BUFFER);
        XX(GL_INT_SAMPLER_2D_RECT);
        XX(GL_UNSIGNED_INT_SAMPLER_1D);
        XX(GL_UNSIGNED_INT_SAMPLER_2D);
        XX(GL_UNSIGNED_INT_SAMPLER_3D);
        XX(GL_UNSIGNED_INT_SAMPLER_CUBE);
        XX(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_BUFFER);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_RECT);

#if !defined(NEKO_IS_APPLE)
        XX(GL_IMAGE_1D);
        XX(GL_IMAGE_2D);
        XX(GL_IMAGE_3D);
        XX(GL_IMAGE_2D_RECT);
        XX(GL_IMAGE_CUBE);
        XX(GL_IMAGE_BUFFER);
        XX(GL_IMAGE_1D_ARRAY);
        XX(GL_IMAGE_2D_ARRAY);
        XX(GL_IMAGE_2D_MULTISAMPLE);
        XX(GL_IMAGE_2D_MULTISAMPLE_ARRAY);
        XX(GL_INT_IMAGE_1D);
        XX(GL_INT_IMAGE_2D);
        XX(GL_INT_IMAGE_3D);
        XX(GL_INT_IMAGE_2D_RECT);
        XX(GL_INT_IMAGE_CUBE);
        XX(GL_INT_IMAGE_BUFFER);
        XX(GL_INT_IMAGE_1D_ARRAY);
        XX(GL_INT_IMAGE_2D_ARRAY);
        XX(GL_INT_IMAGE_2D_MULTISAMPLE);
        XX(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        XX(GL_UNSIGNED_INT_IMAGE_1D);
        XX(GL_UNSIGNED_INT_IMAGE_2D);
        XX(GL_UNSIGNED_INT_IMAGE_3D);
        XX(GL_UNSIGNED_INT_IMAGE_2D_RECT);
        XX(GL_UNSIGNED_INT_IMAGE_CUBE);
        XX(GL_UNSIGNED_INT_IMAGE_BUFFER);
        XX(GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
        XX(GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
        XX(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
        XX(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        XX(GL_UNSIGNED_INT_ATOMIC_COUNTER);
#endif
    }

#undef XX

    static char buffer[32];
    sprintf(buffer, "Unknown GLenum: (0x%04x)", e);
    return buffer;
}

#endif
