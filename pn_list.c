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
 * 해당 아이템의 레퍼런스 카운팅을 줄이고, 제거한다.
 */
static void
pn_list_item_deleter(void *item)
{
	PnObject_Destroy((pn_object *)item);
}

/**
 * 아이템 하나를 현재 리스트에 append 한다.
 */
static pn_object *
PnList_AppendItem(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(object->obj_val->extra_val != NULL);
	PN_ASSERT(length == 1);
	
	List_AppendItem(object->obj_val->extra_val, params[0]);
	
	return object;
}

/**
 * 다른 리스트의 노드를 clone 해서, 현재 리스트에 모두 append 한다.
 */
static pn_object *
PnList_AddList(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(object->obj_val->extra_val != NULL);
	
	// FIXME: 리스트를 통째로 복사해서 넣어야 하나??
	List_AppendList(object->obj_val->extra_val, (list *)params[0], false);
	
	return object;
}

/**
 * 리스트 곱하기 숫자. 리스트의 길이가 숫자만큼 늘어난다.
 */
static pn_object *
PnList_Mult(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(object->obj_val->extra_val != NULL);
	
	// TODO: 나중에 -_-
	
	return NULL;
}

/**
 * 특정 인덱스의 아이템을 삭제한다.
 */
static pn_object *
PnList_RemoveItem(pn_world *world, pn_object *object, pn_object *params[], int length)
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
 * 리스트를 clear 한다.
 */
static pn_object *
PnList_Clear(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(object->obj_val->extra_val != NULL);
	
	List_Clear(object->obj_val->extra_val, pn_list_item_deleter);
	
	return object;
}

/**
 * 특정 아이템을, 특정 인덱스에 insert 한다.
 */
static pn_object *
PnList_Insert(pn_world *world, pn_object *object, pn_object *params[], int length)
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
 * 특정 인덱스의 아이템을 가져온다.
 */
static pn_object *
PnList_Get(pn_world *world, pn_object *object, pn_object *params[], int length)
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
 * 특정 인덱스의 아이템을 업데이트 한다.
 */
static pn_object *
PnList_Put(pn_world *world, pn_object *object, pn_object *params[], int length)
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
 * 리스트가 비어있는지 알려준다.
 */
static pn_object *
PnList_IsEmpty(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(object->obj_val->extra_val != NULL);
	
	pn_object *o = PnBool_Create(world, List_IsEmpty(object->obj_val->extra_val));
	return o;
}

/**
 * 리스트를 문자열로 바꾼다.
 */
static pn_object *
PnList_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(object->obj_val->extra_val != NULL);
	PN_ASSERT(length == 0);
	
	char *str = (char *)pn_alloc(sizeof(char) * TO_STRING_BUF);	
	sprintf(str, "[");
	
	int size = List_Size(object->obj_val->extra_val);
	
	// 항목들을 모두 문자열로 담는다.
	size_t index = 0;
	for (index = 0; index < size; index++)
	{
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
 * 리스트를 제거한다.
 */
static pn_object *
PnList_Delete(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(length == 0);
	PN_ASSERT(object->obj_val->extra_val != NULL);
	
	// TODO: 이터레이터도 삭제해야 한다!! 레퍼런스 카운팅은 대체 어떻게 되는거지?????
	// 이터레이터가 생성될때, 이 리스트의 레퍼런스가 1 올라가고,
	// 이터레이터가 삭제될때, 이 리스트의 레퍼런스가 1 줄어드는게 맞는 것 같은데..
	// 리스트를 사용중일때, 리스트를 제거하면, 리스트의 레퍼런스가 1 줄어들고,
	// 이터레이터는 남아있으니, 레퍼런스는 1이고, 계속 사용 가능해야 할 것 같은데,
	// 실제로 그렇게 동작하나??????
	// 확인해봐야 할 것 같다.
	
	List_Destroy(object->obj_val->extra_val, pn_list_item_deleter);
	
	return PnBool_Create(world, true);
}


/**
 * 이터레이터가 끝인지 확인한다.
 */
static pn_object *
PnListIterator_HasNext(pn_world *world, pn_object *object, pn_object *params[], int length)
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
 * 이터레이터의 다음 노드를 돌려준다.
 */
static pn_object *
PnListIterator_Next(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(length == 0);
	
	// 인덱스를 가져와서 하나 증가시켜서 넣어놓고,
	pn_object *index = PnObject_GetAttr(object, "index");
	int i = index->int_val + 1;
	pn_object *nextIndex = PnInteger_Create(world, i);
	
	PnObject_PutAttr(world, object, "index", nextIndex);
	
	// 그놈의 인덱스로 아이템을 가져온다.
	pn_object *data = PnObject_GetAttr(object, "list");
	pn_object *item = List_Get(data->obj_val->extra_val, i);
	
	return item;
}

/**
 * 이터레이터를 지운다.
 */
static pn_object *
PnListIterator_Delete(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(length == 0);
	
	// TODO: 이터레이터를 지운다!!!!!
	
	return PnNull_Create(world);
}

/**
 * 이터레이터용 to_str() 함수
 */
static pn_object *
PnListIterator_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(length == 0);
	
	return PnObject_ToString(world, object);
}

/**
 * 이터레이터를 생성한다.
 */
static pn_object *
PnList_Iterator(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(length == 0);
	PN_ASSERT(object->obj_val->extra_val != NULL);
	
	pn_object *o = PnObject_CreateEmptyObject(world);
	o->type = TYPE_OBJECT;
	
	// 인덱스를 생성한다. 처음은 -1 이다.
	pn_object *index = PnInteger_Create(world, -1);
	
	// 리스트 데이터와, 현재 인덱스를 보관한다. (레퍼런스 증가)
	PnObject_PutAttr(world, o, "list", object);
	PnObject_PutAttr(world, o, "index", index);
	
	// 이터레이터의 메소드들..
	PnObject_PutAttr(world, o, "hasNext?", PnFunction_CreateByNative(world, PnListIterator_HasNext));
	PnObject_PutAttr(world, o, "next", PnFunction_CreateByNative(world, PnListIterator_Next));
	PnObject_PutAttr(world, o, "delete", PnFunction_CreateByNative(world, PnListIterator_Delete));
	PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnListIterator_ToString));
	
	return o;
}


/**
 * 리스트를 생성한다.
 */
pn_object *
PnList_Create(pn_world *world)
{
	pn_object *proto = World_GetObject(world, PROTONAME_LIST);
	PN_ASSERT(proto != NULL);
	pn_object *o = PnObject_Clone(world, proto);
	PN_ASSERT(o != NULL);
	
	// TODO 리스트의 요소를 deepcopy 한다?????
	o->obj_val->extra_val = List_Create();
	
	return o;
}

/**
 * 리스트를 생성한다.
 */
pn_object *
PnList_CreatePrototype(pn_world *world)
{
	pn_object *o = PnObject_CreateEmptyObject(world);
	o->type = TYPE_OBJECT;
	o->obj_val->extra_val = List_Create();
	
	// 메소드들..
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
	
	// 이터레이터
	PnObject_PutAttr(world, o, "iterator", PnFunction_CreateByNative(world, PnList_Iterator));
	
	return o;
}

pn_object *
PnList_AddObject(pn_world *world, pn_object *pn_list, pn_object *object)
{
	pn_object *params[1];
	params[0] = object;
	return PnList_AppendItem(world, pn_list, params, 1);
}
