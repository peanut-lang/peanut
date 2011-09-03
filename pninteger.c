#include "pninteger.h"
#include "pnobject.h"
#include "pnfunction.h"

#include <stdlib.h>
#include <string.h>

static pn_object *PnInteger_Add(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnInteger_Create(world, 0);
        result->val.int_val = object->val.int_val + other->val.int_val;
    } else if (IS_REAL(other)) {
        result = PnReal_Create(world, 0.0);
        result->val.real_val = object->val.int_val + other->val.real_val;
    } else {
        // TODO ??
    }

    return result;
}

static pn_object *PnInteger_Sub(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnInteger_Create(world, 0);
        result->val.int_val = object->val.int_val - other->val.int_val;
    } else if (IS_REAL(other)) {
        result = PnReal_Create(world, 0.0);
        result->val.real_val = object->val.int_val - other->val.real_val;
    } else {
        // TODO ??
    }

    return result;
}

static pn_object *PnInteger_Mult(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnInteger_Create(world, 0);
        result->val.int_val = object->val.int_val * other->val.int_val;
    } else if (IS_REAL(other)) {
        result = PnReal_Create(world, 0.0);
        result->val.real_val = object->val.int_val * other->val.real_val;
    } else {
        // TODO ??
    }

    return result;
}

static pn_object *PnInteger_Div(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnReal_Create(world, 0.0);
        result->val.real_val = object->val.int_val / (double) other->val.int_val;
    } else if (IS_REAL(other)) {
        result = PnReal_Create(world, 0.0);
        result->val.real_val = object->val.int_val / other->val.real_val;
    } else {
        // TODO ??
    }

    return result;
}

static pn_object *PnInteger_Mod(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnInteger_Create(world, 0);
        result->val.int_val = object->val.int_val % other->val.int_val;
    } else if (IS_REAL(other)) {
        // TODO ??
    } else {
        // TODO ??
    }

    return result;
}

static pn_object *PnInteger_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 0);
    char *s = malloc(sizeof(char) * 11);  // maximum is 11
    sprintf(s, "%d", object->val.int_val);
    pn_object *result = PnString_Create(world, s);
    free(s);
    PN_ASSERT(result != 0);
    return result;
}

static pn_object *PnInteger_Eqfn(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *result = PnInteger_Create(world, 0);
    pn_object *other = params[0];
    if (IS_INTEGER(other) && object->val.int_val == other->val.int_val)
        result->val.int_val = 1;
    return result;
}

static pn_object *PnInteger_NEqfn(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *result = PnInteger_Create(world, 0);
    pn_object *other = params[0];
    if (IS_INTEGER(other) && object->val.int_val != other->val.int_val)
        result->val.int_val = 1;
    return result;
}

pn_object *PnInteger_Create(pn_world *world, int value)
{
    pn_object *proto = World_GetObject(world, PROTONAME_INTEGER);
    PN_ASSERT(proto != NULL);
    pn_object *o = PnObject_Clone(world, proto);
    PN_ASSERT(o != NULL);
    o->val.int_val = value;
    return o;
}

pn_object *PnInteger_CreatePrototype(pn_world *world)
{
    pn_object *o = PnObject_CreateEmptyObject(world);
    o->type = TYPE_INTEGER;
    o->val.int_val = 0;

    // methods..
    PnObject_PutAttr(world, o, "+", PnFunction_CreateByNative(world, PnInteger_Add));
    PnObject_PutAttr(world, o, "-", PnFunction_CreateByNative(world, PnInteger_Sub));
    PnObject_PutAttr(world, o, "*", PnFunction_CreateByNative(world, PnInteger_Mult));
    PnObject_PutAttr(world, o, "/", PnFunction_CreateByNative(world, PnInteger_Div));
    PnObject_PutAttr(world, o, "%", PnFunction_CreateByNative(world, PnInteger_Mod));
    PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnInteger_ToString));
    PnObject_PutAttr(world, o, "==", PnFunction_CreateByNative(world, PnInteger_Eqfn));
    PnObject_PutAttr(world, o, "!=", PnFunction_CreateByNative(world, PnInteger_NEqfn));

    return o;
}
