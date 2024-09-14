#include "engine/component.h"

#include <stdbool.h>
#include <stdio.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/bootstrap.h"
#include "engine/edit.h"
#include "engine/entity.h"

// deps
#ifdef NEKO_BOX2D
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>
#endif


