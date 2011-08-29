#include "pn_list.h"
#include "list.h"
#include "pn_string.h"
#include "pn_object.h"
#include "pn_function.h"
#include "pn_bool.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


/**
 * decrease reference counting, and delete item
 */
static void pn_list_item_deleter(void *item)
{
    PnObject_Destroy((pn_object *)item);
}

/**
 * append an item to the list
 */
static pn_object *PnList_AppendItem(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 1);

    List_AppendItem(object->obj_val->extra_val, params[0]);

    return object;
}

/**
 * append a list to the list
 */
static pn_object *PnList_AddList(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    // FIXME: how about reference counter?
    List_AppendList(object->obj_val->extra_val, (list *)params[0], false);

    return object;
}

/**
 * list x number
 */
static pn_object *PnList_Mult(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    // TODO: not support in this version

    return NULL;
}

/**
 * remove an item by index
 */
static pn_object *PnList_RemoveItem(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 1);
    PN_ASSERT(IS_INTEGER(params[0]));

    int which = params[0]->int_val;
    List_Remove(object->obj_val->extra_val, which, 1, pn_list_item_deleter);

    return object;
}

/**
 * clear the list
 */
static pn_object *PnList_Clear(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    List_Clear(object->obj_val->extra_val, pn_list_item_deleter);

    return object;
}

/**
 * insert an item the the list where index
 */
static pn_object *PnList_Insert(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 2);
    PN_ASSERT(IS_INTEGER(params[0]));

    int which = params[0]->int_val;
    pn_object *item = params[1];

    List_InsertItem(object->obj_val->extra_val, which, item);

    return object;
}

/**
 * get an item by index
 */
static pn_object *PnList_Get(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 1);
    PN_ASSERT(IS_INTEGER(params[0]));

    int which = params[0]->int_val;
    pn_object *item = List_Get(object->obj_val->extra_val, which);

    PN_ASSERT(item != NULL);

    return item;
}

/**
 * update an item by index
 */
static pn_object *PnList_Put(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 2);
    PN_ASSERT(IS_INTEGER(params[0]));

    int which = params[0]->int_val;
    pn_object *item = params[1];

    List_Put((list *)object->obj_val->extra_val, which, item);

    return object;
}

/**
 * is empty?
 */
static pn_object *PnList_IsEmpty(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    pn_object *o = PnBool_Create(world, List_IsEmpty(object->obj_val->extra_val));
    return o;
}

/**
 * list to string
 */
static pn_object *PnList_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->obj_val->extra_val != NULL);
    PN_ASSERT(length == 0);

    char *str = (char *)pn_alloc(sizeof(char) * TO_STRING_BUF);
    sprintf(str, "[");

    int size = List_Size(object->obj_val->extra_val);

    size_t index = 0;
    for (index = 0; index < size; index++) {
        pn_object *item = List_Get(object->obj_val->extra_val, index);
        pn_object *child = PnFunction_ExecuteByObject("to_str", world, item, NULL, 0);
        PN_ASSERT(IS_STRING(child));
        strcat(str, child->str_val);
        strcat(str, ", ");
    }

    strcat(str, "]");

    pn_object *result = PnString_Create(world, str);

    return result;
}

/**
 * delete the list
 */
static pn_object *PnList_Delete(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    // TODO: delete the iterators. how about reference counting?
    // when iterator is created, list's reference is +1
    // when iterator is terminated, list's reference is -1
    // check.. deleting the list when still using iterator.

    List_Destroy(object->obj_val->extra_val, pn_list_item_deleter);

    return PnBool_Create(world, true);
}


/**
 * check has next
 */
static pn_object *PnListIterator_HasNext(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);

    pn_object *data = PnObject_GetAttr(object, "list");
    pn_object *index = PnObject_GetAttr(object, "index");
    int i = index->int_val;
    int size = List_Size(data->obj_val->extra_val);

    return PnBool_Create(world, i + 1 < size);
}

/**
 * get the next item
 */
static pn_object *PnListIterator_Next(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);

    // index += 1
    pn_object *index = PnObject_GetAttr(object, "index");
    int i = index->int_val + 1;
    pn_object *nextIndex = PnInteger_Create(world, i);

    PnObject_PutAttr(world, object, "index", nextIndex);

    // get the list by index
    pn_object *data = PnObject_GetAttr(object, "list");
    pn_object *item = List_Get(data->obj_val->extra_val, i);

    return item;
}

/**
 * delete iterator
 */
static pn_object *PnListIterator_Delete(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);

    // TODO: delete iterator

    return PnNull_Create(world);
}

/**
 * iterator to string
 */
static pn_object *PnListIterator_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);

    return PnObject_ToString(world, object);
}

/**
 * create a iterator
 */
static pn_object *PnList_Iterator(pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(object != NULL);
    PN_ASSERT(length == 0);
    PN_ASSERT(object->obj_val->extra_val != NULL);

    pn_object *o = PnObject_CreateEmptyObject(world);
    o->type = TYPE_OBJECT;

    // create 'index', at creating time, index is -1
    pn_object *index = PnInteger_Create(world, -1);

    PnObject_PutAttr(world, o, "list", object);
    PnObject_PutAttr(world, o, "index", index);

    PnObject_PutAttr(world, o, "hasNext?", PnFunction_CreateByNative(world, PnListIterator_HasNext));
    PnObject_PutAttr(world, o, "next", PnFunction_CreateByNative(world, PnListIterator_Next));
    PnObject_PutAttr(world, o, "delete", PnFunction_CreateByNative(world, PnListIterator_Delete));
    PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnListIterator_ToString));

    return o;
}


/**
 * create a list
 */
pn_object *PnList_Create(pn_world *world)
{
    pn_object *proto = World_GetObject(world, PROTONAME_LIST);
    PN_ASSERT(proto != NULL);
    pn_object *o = PnObject_Clone(world, proto);
    PN_ASSERT(o != NULL);

    // TODO deepcopy?
    o->obj_val->extra_val = List_Create();

    return o;
}

/**
 * craete list prototype
 */
pn_object *PnList_CreatePrototype(pn_world *world)
{
    pn_object *o = PnObject_CreateEmptyObject(world);
    o->type = TYPE_OBJECT;
    o->obj_val->extra_val = List_Create();

    // methods..
    PnObject_PutAttr(world, o, "append", PnFunction_CreateByNative(world, PnList_AppendItem));
    PnObject_PutAttr(world, o, "add", PnFunction_CreateByNative(world, PnList_AddList));
    PnObject_PutAttr(world, o, "*", PnFunction_CreateByNative(world, PnList_Mult));
    PnObject_PutAttr(world, o, "remove", PnFunction_CreateByNative(world, PnList_RemoveItem));
    PnObject_PutAttr(world, o, "clear", PnFunction_CreateByNative(world, PnList_Clear));
    PnObject_PutAttr(world, o, "insert", PnFunction_CreateByNative(world, PnList_Insert));
    PnObject_PutAttr(world, o, "get", PnFunction_CreateByNative(world, PnList_Get));
    PnObject_PutAttr(world, o, "put", PnFunction_CreateByNative(world, PnList_Put));
    PnObject_PutAttr(world, o, "empty?", PnFunction_CreateByNative(world, PnList_IsEmpty));
    PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnList_ToString));
    PnObject_PutAttr(world, o, "delete", PnFunction_CreateByNative(world, PnList_Delete));

    // iterator
    PnObject_PutAttr(world, o, "iterator", PnFunction_CreateByNative(world, PnList_Iterator));

    return o;
}

pn_object *PnList_AddObject(pn_world *world, pn_object *pn_list, pn_object *object)
{
    pn_object *params[1];
    params[0] = object;
    return PnList_AppendItem(world, pn_list, params, 1);
}
