#include "pn_integer.h"
#include "pn_object.h"
#include "pn_function.h"

#include <stdlib.h>
#include <string.h>

static pn_object *PnInteger_Add(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnObject_CreateInteger(world);
        result->int_val = object->int_val + other->int_val;
    } else if (IS_REAL(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->int_val + other->real_val;
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
        result = PnObject_CreateInteger(world);
        result->int_val = object->int_val - other->int_val;
    } else if (IS_REAL(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->int_val - other->real_val;
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
        result = PnObject_CreateInteger(world);
        result->int_val = object->int_val * other->int_val;
    } else if (IS_REAL(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->int_val * other->real_val;
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
        result = PnObject_CreateReal(world);
        result->real_val = object->int_val / (double) other->int_val;
    } else if (IS_REAL(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->int_val / other->real_val;
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
        result = PnObject_CreateInteger(world);
        result->int_val = object->int_val % other->int_val;
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
    char *s = pn_alloc(sizeof(char) * 11);  // maximum is 11
    sprintf(s, "%d", object->int_val);
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
    if (IS_INTEGER(other) && object->int_val == other->int_val)
        result->int_val = 1;
    return result;
}

static pn_object *PnInteger_NEqfn(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *result = PnInteger_Create(world, 0);
    pn_object *other = params[0];
    if (IS_INTEGER(other) && object->int_val != other->int_val)
        result->int_val = 1;
    return result;
}

pn_object *PnInteger_Create(pn_world *world, int value)
{
    pn_object *proto = World_GetObject(world, PROTONAME_INTEGER);
    PN_ASSERT(proto != NULL);
    pn_object *o = PnObject_Clone(world, proto);
    PN_ASSERT(o != NULL);
    o->int_val = value;
    return o;
}

pn_object *PnInteger_CreatePrototype(pn_world *world)
{
    pn_object *o = PnObject_CreateEmptyObject(world);
    o->type = TYPE_INTEGER;
    o->int_val = 0;

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
