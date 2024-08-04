#ifndef TEXTURE_H
#define TEXTURE_H

#include "engine/neko_base.h"

bool texture_load(const char *filename);
void texture_bind(const char *filename);
CVec2 texture_get_size(const char *filename);  // (width, height)

void texture_init();
void texture_deinit();
void texture_update();

#endif
