#include "pn_hash.h"
#include "hash.h"
#include "pn_list.h"
#include "pn_string.h"
#include "pn_object.h"
#include "pn_function.h"
#include "pn_bool.h"

#include <stdlib.h>
#include <string.h>

static pn_object *PnHash_GetItem(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 1);
    PN_ASSERT(IS_STRING(params[0]));

    pn_object *o = Hash_Get(object->obj_val->extra_val, params[0]->str_val);
    return o;
}

static pn_object *PnHash_PutItem(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 2);
    PN_ASSERT(IS_STRING(params[0]));

    Hash_Put(object->obj_val->extra_val, params[0]->str_val, params[1]);

    return object;
}

static pn_object *PnHash_AddHash(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 1);

    hash *other = (hash *)params[0]->obj_val->extra_val;
    hash_itr *itr = Hash_Iterator(other);
    do {
        char *key = Hash_Iterator_Key(itr);
        pn_object *o = Hash_Iterator_Value(itr);
        Hash_Put(object->obj_val->extra_val, key, o);
    } while (Hash_Iterator_Advance(itr));
    free(itr);

    return object;
}

static pn_object *PnHash_RemoveItem(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 1);
    PN_ASSERT(IS_STRING(params[0]));

    pn_object *o = Hash_Remove((hash *)object, params[0]->str_val);
    PnObject_Destroy(o);
    return object;
}

static pn_object *PnHash_Clear(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 0);

    hash_itr *itr = Hash_Iterator(object->obj_val->extra_val);
    do {
        pn_object *o = Hash_Iterator_Value(itr);
        if (o != NULL)
            PnObject_Destroy(o);
    } while (Hash_Iterator_Advance(itr));
    free(itr);

    return object;
}

static pn_object *PnHash_IsEmpty(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 0);

    unsigned int size = Hash_Count((hash *)object);
    if (size == 0)
        return PnBool_Create(world, true);
    else
        return PnBool_Create(world, false);
}

static pn_object *PnHash_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 0);

    char *str = (char *)malloc(sizeof(char) * TO_STRING_BUF);
    strcpy(str, "{");

    int size = Hash_Count(object->obj_val->extra_val);

    if (size > 0) {
        hash_itr *itr = Hash_Iterator(object->obj_val->extra_val);
        do {
            char *k = Hash_Iterator_Key(itr);
            pn_object *v = Hash_Iterator_Value(itr);
            pn_object *s = PnFunction_ExecuteByObject("to_str", world, v, NULL, 0);
            PN_ASSERT(IS_STRING(s));
            strcat(str, "'");
            strcat(str, k);
            strcat(str, "' : ");
            strcat(str, s->str_val);
            strcat(str, ", ");
        } while (Hash_Iterator_Advance(itr));
        free(itr);
    }

    strcat(str, "}");

    pn_object *result = PnString_Create(world, str);

    return result;
}

static pn_object *PnHash_Delete(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    hash_itr *itr = Hash_Iterator(object->obj_val->extra_val);
    do {
        pn_object *o = Hash_Iterator_Value(itr);
        if (o != NULL)
            PnObject_Destroy(o);
    } while (Hash_Iterator_Advance(itr));
    free(itr);
    Hash_Destroy(object->obj_val->extra_val, false);

    return PnBool_Create(world, true);
}

static pn_object *PnHash_Keys(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    pn_object *keys = PnList_Create(world);

    if (Hash_Count(object->obj_val->extra_val) > 0) {
        hash_itr *itr = Hash_Iterator(object->obj_val->extra_val);
        do {
            char *key = Hash_Iterator_Key(itr);
            pn_object *key_object = PnString_Create(world, key);
            PnList_AddObject(world, keys, key_object);
        } while (Hash_Iterator_Advance(itr));
        free(itr);
    }

    return keys;
}

static pn_object *PnHash_Values(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    pn_object *values = PnList_Create(world);

    if (Hash_Count(object->obj_val->extra_val) > 0) {
        hash_itr *itr = Hash_Iterator(object->obj_val->extra_val);
        do {
            pn_object *value = Hash_Iterator_Value(itr);
            PnList_AddObject(world, values, PnObject_CreateFromReference(world, value));
        } while (Hash_Iterator_Advance(itr));
        free(itr);
    }

    return values;
}

pn_object *PnHash_Create(pn_world *world)
{
    pn_object *proto = World_GetObject(world, PROTONAME_HASH);
    pn_object *o = PnObject_Clone(world, proto);

    // TODO: deepcopy
    o->obj_val->extra_val = Hash_Create();

    return o;
}

pn_object *PnHash_CreatePrototype(pn_world *world)
{
    pn_object *o = PnObject_CreateEmptyObject(world);
    o->type = TYPE_OBJECT;
    o->obj_val->extra_val = Hash_Create();

    // methods
    PnObject_PutAttr(world, o, "get", PnFunction_CreateByNative(world, PnHash_GetItem));
    PnObject_PutAttr(world, o, "put", PnFunction_CreateByNative(world, PnHash_PutItem));
    PnObject_PutAttr(world, o, "add", PnFunction_CreateByNative(world, PnHash_AddHash));
    PnObject_PutAttr(world, o, "remove", PnFunction_CreateByNative(world, PnHash_RemoveItem));
    PnObject_PutAttr(world, o, "clear", PnFunction_CreateByNative(world, PnHash_Clear));
    PnObject_PutAttr(world, o, "empty?", PnFunction_CreateByNative(world, PnHash_IsEmpty));
    PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnHash_ToString));
    PnObject_PutAttr(world, o, "delete", PnFunction_CreateByNative(world, PnHash_Delete));

    // iterators
    PnObject_PutAttr(world, o, "keys", PnFunction_CreateByNative(world, PnHash_Keys));
    PnObject_PutAttr(world, o, "values", PnFunction_CreateByNative(world, PnHash_Values));

    return o;
}
