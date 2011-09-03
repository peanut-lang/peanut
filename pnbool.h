#ifndef _PEANUT_PNBOOL_H_
#define _PEANUT_PNBOOL_H_

#include "globals.h"
#include "world.h"
#include <stdbool.h>
#include "pnobject.h"

pn_object *PnBool_Create(pn_world *world, bool value);
pn_object *PnBool_CreatePrototype(pn_world *world);

#endif//_PEANUT_PNBOOL_H_
