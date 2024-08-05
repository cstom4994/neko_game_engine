#ifndef PREFAB_H
#define PREFAB_H

#include "engine/ecs.h"
#include "engine/base.h"
#include "engine/prelude.h"

NEKO_SCRIPT(prefab,

       // 将所有过滤的实体保存为预制件 并将root作为根
       NEKO_EXPORT void prefab_save(const char *filename, Entity root);

       // 加载已保存的预制件 返回已保存的根实体
       NEKO_EXPORT Entity prefab_load(const char *filename);

)

void prefab_save_all(Store *s);
void prefab_load_all(Store *s);

#endif
