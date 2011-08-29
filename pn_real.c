#include <stdlib.h>
#include <string.h>

#include "pn_real.h"
#include "pn_object.h"
#include "pn_function.h"

static pn_object *PnReal_Add(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->real_val + other->int_val;
    } else if (IS_REAL(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->real_val + other->real_val;
    } else {
    }

    return result;
}

static pn_object *PnReal_Sub(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->real_val - other->int_val;
    } else if (IS_REAL(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->real_val - other->real_val;
    } else {
    }

    return result;
}

static pn_object *PnReal_Mult(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->real_val * other->int_val;
    } else if (IS_REAL(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->real_val * other->real_val;
    } else {
    }

    return result;
}

static pn_object *PnReal_Div(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->real_val / other->int_val;
    } else if (IS_REAL(other)) {
        result = PnObject_CreateReal(world);
        result->real_val = object->real_val / other->real_val;
    } else {
    }

    return result;
}

static pn_object *PnReal_Mod(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        return NULL;
    } else if (IS_REAL(other)) {
        return NULL;
    } else {
        return NULL;
    }

    return result;
}

static pn_object *PnReal_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 0);
    char ch[101]; // TODO: maximum?
    sprintf(ch, "%.10f", object->real_val);
    int str_length = strlen(ch);
    pn_object *result = PnObject_CreateString(world);
    result->str_val = pn_alloc(str_length + 1);
    strncpy(result->str_val, ch, str_length);
    return result;
}

static pn_object *PnReal_Eqfn(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *result = PnObject_CreateInteger(0);
    pn_object *other = params[0];
    if (IS_REAL(other) && object->real_val == other->real_val)
        result->int_val = 1;
    return result;
}

static pn_object *PnReal_Ltfn(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *result = PnObject_CreateInteger(0);
    pn_object *other = params[0];
    if (IS_REAL(other) && object->real_val > other->real_val)
        result->int_val = 1;
    return result;
}

pn_object *PnReal_Create(pn_world *world, double value)
{
    pn_object *proto = World_GetObject(world, PROTONAME_REAL);
    PN_ASSERT(proto != NULL);
    pn_object *o = PnObject_Clone(world, proto);
    PN_ASSERT(o != NULL);
    o->real_val = value;
    return o;
}

pn_object *PnReal_CreatePrototype(pn_world *world)
{
    pn_object *o = PnObject_CreateEmptyObject(world);
    o->type = TYPE_REAL;
    o->real_val = 0.0;

    // methods..
    PnObject_PutAttr(world, o, "+", PnFunction_CreateByNative(world, PnReal_Add));
    PnObject_PutAttr(world, o, "-", PnFunction_CreateByNative(world, PnReal_Sub));
    PnObject_PutAttr(world, o, "*", PnFunction_CreateByNative(world, PnReal_Mult));
    PnObject_PutAttr(world, o, "/", PnFunction_CreateByNative(world, PnReal_Div));
    PnObject_PutAttr(world, o, "%", PnFunction_CreateByNative(world, PnReal_Mod));
    PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnReal_ToString));
    PnObject_PutAttr(world, o, "==", PnFunction_CreateByNative(world, PnReal_Eqfn));
    PnObject_PutAttr(world, o, ">", PnFunction_CreateByNative(world, PnReal_Ltfn));

    return o;
}

