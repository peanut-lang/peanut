#ifndef PEANUT_BOOL_H
#define PEANUT_BOOL_H

#include "globals.h"
#include "world.h"
#include <stdbool.h>
#include "pn_object.h"

pn_object *PnBool_Create(pn_world *world, bool value);
pn_object *PnBool_CreatePrototype(pn_world *world);

#endif//PEANUT_BOOL_H
