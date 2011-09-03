#ifndef _PEANUT_PNLIST_H_
#define _PEANUT_PNLIST_H_

#include "globals.h"
#include "world.h"

pn_object *PnList_Create(pn_world *world);
pn_object *PnList_CreatePrototype(pn_world *world);
pn_object *PnList_AddObject(pn_world *world, pn_object *pn_list, pn_object *object);

#endif//_PEANUT_PNLIST_H_
