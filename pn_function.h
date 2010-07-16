#ifndef PEANUT_PN_FUNCTION_H
#define PEANUT_PN_FUNCTION_H

#include "globals.h"
#include "pn_object.h"
#include "world.h"

typedef pn_object *(* pn_function_native)(pn_world *, pn_object *, pn_object **, int);

pn_object *PnFunction_CreateByNative(pn_world *world, pn_function_native func_body);
pn_object *PnFunction_CreateByNode(pn_world *world, pn_node *func_code);
pn_object *PnFunction_ExecuteByObject(const char *name, pn_world *world, pn_object *object, pn_object *params[], int length);
pn_object *PnFunction_ExecuteByFuncObject(pn_object* pn_func, pn_world *world, pn_object *object, pn_object *params[], int length);

#endif//PEANUT_PN_FUNCTION_H
