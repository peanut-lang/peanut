#ifndef _PEANUT_PNOBJECT_H_
#define _PEANUT_PNOBJECT_H_

#include <stdbool.h>
#include "globals.h"
#include "pninteger.h"
#include "pnreal.h"
#include "pnstring.h"
#include "pnnull.h"
#include "pnbool.h"

#include "hash.h"

typedef struct _pn_object_val {
    unsigned int ref_count;
    hash *members;
    void *extra_val;    // for PnList, PnHash
} pn_object_val;

pn_object *PnObject_Clone(pn_world *world, pn_object *object);
pn_object *PnObject_ToString(pn_world *world, pn_object *object);

pn_object *PnObject_CreateEmptyObject(pn_world *world);
pn_object *PnObject_CreateEmptyObjectByNotMembers(pn_world *world);
pn_object *PnObject_CreateFromReference(pn_world *world, pn_object *other);

hash_itr *PnObject_GetAllAttributes(pn_object *object);
pn_object *PnObject_GetAttr(pn_object *obj, const char *name);
void PnObject_PutAttr(pn_world *world, pn_object *obj, const char *name, pn_object *value);
void PnObject_Destroy(pn_object *obj);

bool PnObject_IsTrue(pn_object *value);

pn_object *PnObject_CreateByNode(pn_world *world, pn_node *node);
pn_object *PnObject_CreateByObject(pn_world *world, pn_object *object);
pn_object *PnObject_Inherit(pn_world *world, pn_object *super, pn_object *child);

#endif//_PEANUT_PNOBJECT_H_
