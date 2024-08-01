
#ifndef NEKO_COMPONENT_H
#define NEKO_COMPONENT_H

#include "engine/neko_ecs.h"

typedef struct {
    float x, y;
} pos_t;

typedef struct {
    float vx, vy;
} vel_t;

typedef struct {
    int x, y, w, h;
} rect_t;

#endif