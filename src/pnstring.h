#ifndef _PEANUT_PNSTRING_H_
#define _PEANUT_PNSTRING_H_

#include "globals.h"
#include "world.h"

pn_object *PnString_Create(pn_world *world, char *str);
pn_object *PnString_CreatePrototype(pn_world *world);
pn_object *PnString_Add(pn_world *world, pn_object *object, pn_object *params[], int length);

#endif//_PEANUT_PNSTRING_H_
