#include <string.h>
#include <stdlib.h>

#include "pn_object.h"
#include "pn_function.h"

static pn_object *__concatenate_strings(pn_world *world, pn_object *object, pn_object *other)
{
    int str_length = strlen(object->str_val);
    int other_str_length = strlen(other->str_val);
    int total_length = str_length + other_str_length + 1;
    char *str = malloc(sizeof(char) * total_length);
    strncpy(str, object->str_val, str_length);
    str[str_length] = 0;
    strncat(str, other->str_val, other_str_length);
    str[total_length] = 0;
    pn_object *result = PnString_Create(world, str);
    return result;
}

pn_object *PnString_Add(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];

    if (IS_STRING(other)) {
        return __concatenate_strings(world, object, other);
    } else {
        pn_object *str_other = PnFunction_ExecuteByObject("to_str", world, other, NULL, 0);
        PN_ASSERT(str_other != NULL);
        PN_ASSERT(IS_STRING(str_other));
        return __concatenate_strings(world, object, str_other);
    }
}

static pn_object *PnString_Mult(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *other = params[0];
    pn_object *result = NULL;

    if (IS_INTEGER(other)) {
        int repeat = other->int_val;
        int str_length = strlen(object->str_val);
        char *str = malloc(str_length * repeat + 1);
        result = PnString_Create(world, str);
        int i;
        for (i = 0; i < repeat; i++) {
            strncat(result->str_val, object->str_val, str_length);
        }
    } else if (IS_REAL(other)) {
        //
    } else if (IS_STRING(other)) {
        //
    } else {
        //
    }

    return result;
}

static pn_object *PnString_ToInteger(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 0);
    // TODO: need check exception, with sscanf
    int ret_int = atoi(object->str_val);
    if (ret_int == 0 && strcmp(object->str_val, "0") != 0) {
        return NULL;
    }

    pn_object *result = PnObject_CreateInteger(world);
    result->int_val = ret_int;
    return result;
}

static pn_object *PnString_ToReal(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 0);
    // TODO: need check exception, with sscanf
    double ret_real = atof(object->str_val);

    pn_object *result = PnObject_CreateReal(world);
    result->real_val = ret_real;
    return result;
}

static pn_object *PnString_Substring(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 2);
    pn_object *begin_index_obj = params[0];
    pn_object *end_index_obj = params[1];
    PN_ASSERT(begin_index_obj->type == TYPE_INTEGER && end_index_obj->type == TYPE_INTEGER);
    int str_length = strlen(object->str_val);
    int begin_index = begin_index_obj->int_val;
    PN_ASSERT(begin_index >= 0);
    int end_index = end_index_obj->int_val;
    int result_str_length = end_index - begin_index;
    if (end_index > str_length) {
        return NULL;
    }

    char *str = malloc(result_str_length + 1);
    strncpy(str, object->str_val + begin_index, result_str_length);
    str[result_str_length] = 0;
    pn_object *result = PnString_Create(world, str);
    return result;
}

static pn_object *PnString_Reverse(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 0);
    int i;
    int str_length = strlen(object->str_val);
    char *str = malloc(str_length + 1);
    for (i = 0; i < str_length; i++)
        str[str_length - i - 1] = object->str_val[i];

    str[str_length] = 0;
    pn_object *result = PnString_Create(world, str);
    return result;
}

static pn_object *PnString_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 0);
    return PnObject_Clone(world, object);
}

static pn_object *PnString_Eqfn(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(length == 1);
    pn_object *result = PnObject_CreateInteger(world);
    pn_object *other = params[0];
    if (IS_STRING(other) && strcmp(object->str_val, other->str_val) == 0)
        result->int_val = 1;
    return result;
}

pn_object *PnString_Create(pn_world *world, char *str)
{
    pn_object *proto = World_GetObject(world, PROTONAME_STRING);
    PN_ASSERT(proto != NULL);
    pn_object *o = PnObject_Clone(world, proto);
    PN_ASSERT(o != NULL);
    o->str_val = strdup(str);
    return o;
}

pn_object *PnString_CreatePrototype(pn_world *world)
{
    pn_object *o = PnObject_CreateEmptyObject(world);
    o->type = TYPE_STRING;
    o->str_val = strdup("");

    // methods..
    PnObject_PutAttr(world, o, "+", PnFunction_CreateByNative(world, PnString_Add));
    PnObject_PutAttr(world, o, "*", PnFunction_CreateByNative(world, PnString_Mult));
    PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnString_ToString));
    PnObject_PutAttr(world, o, "to_i", PnFunction_CreateByNative(world, PnString_ToInteger));
    PnObject_PutAttr(world, o, "to_f", PnFunction_CreateByNative(world, PnString_ToReal));
    PnObject_PutAttr(world, o, "substr", PnFunction_CreateByNative(world, PnString_Substring));
    PnObject_PutAttr(world, o, "reverse", PnFunction_CreateByNative(world, PnString_Reverse));
    PnObject_PutAttr(world, o, "==", PnFunction_CreateByNative(world, PnString_Eqfn));

    return o;
}

