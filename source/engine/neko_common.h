

#ifndef NEKO_COMMON_H
#define NEKO_COMMON_H

#include "neko.h"

NEKO_API_DECL unsigned char* neko_base64_encode(unsigned char* str);
NEKO_API_DECL unsigned char* neko_base64_decode(unsigned char* code);

#endif
