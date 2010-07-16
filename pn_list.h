#ifndef PEANUT_PN_LIST_H
#define PEANUT_PN_LIST_H

#include "globals.h"
#include "world.h"

pn_object *PnList_Create(pn_world *world);
pn_object *PnList_CreatePrototype(pn_world *world);
pn_object *PnList_AddObject(pn_world *world, pn_object *pn_list, pn_object *object);

#endif//PEANUT_PN_LIST_H
