#ifndef PEANUT_PN_STRING_H
#define PEANUT_PN_STRING_H

#include "globals.h"
#include "world.h"

pn_object *PnString_Create(pn_world *world, char *str);
pn_object *PnString_CreatePrototype(pn_world *world);
pn_object *PnString_Add(pn_world *world, pn_object *object, pn_object *params[], int length);

#endif//PEANUT_PN_STRING_H
