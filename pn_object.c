#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "pn_object.h"
#include "pn_function.h"
#include "pn_string.h"
#include "pn_bool.h"

/**
 * 현재 객체를 복제한다. peanut 안에서 객체를 생성하는 유일한 방법이다.
 */
pn_object *
PnObject_Clone(pn_world *world, pn_object *object)
{
	pn_object *obj = NULL;
	
	PN_ASSERT(object != NULL);
	
	if (IS_NATIVE(object) || IS_FUNCTION(object))
	{
		obj = PnObject_CreateEmptyObjectByNotMembers(world);
	}
	else
	{
		obj = PnObject_CreateEmptyObject(world);
	}
	
	// 새로 생성해서, 현재 객체를 복제한다. 값도 복제해야 한다.
	if (IS_INTEGER(object))
	{
		obj->type = TYPE_INTEGER;
		obj->int_val = object->int_val;
	}
	else if (IS_REAL(object))
	{
		obj->type = TYPE_REAL;
		obj->real_val = object->real_val;
	}
	else if (IS_STRING(object))
	{
		obj->type = TYPE_STRING;
		obj->str_val = object->str_val;
	}
	else if (IS_NATIVE(object))
	{
		obj->type = TYPE_NATIVE;
		obj->func.body_pointer = object->func.body_pointer;
	}
	else if (IS_FUNCTION(object))
	{
		obj->type = TYPE_FUNCTION;
		obj->func.body_node = object->func.body_node;
	}
	else if (IS_OBJECT(object))
	{
		obj->type = TYPE_OBJECT;
	}
	else if (IS_BOOL(object))
	{
		obj->type = TYPE_BOOL;
	}
	else if (IS_NULL(object))
	{
		obj->type = TYPE_NULL;
	}
	else
	{
		ANDLOG("object->type = %d\n", object->type);
		PN_FAIL("PnObject_CloneObject failed.");
	}
	
	// obj_val의 member가 존재하는 경우에는 멤버들도 복제한다. (함수의 경우에는 존재하지 않음)
	if (object->obj_val->members != NULL)
	{
		hash_itr *itr = Hash_Iterator(object->obj_val->members);
		do {
			pn_object *o = Hash_Iterator_Value(itr);
			if (o != NULL)
			{
				char *key = Hash_Iterator_Key(itr);
				pn_object *clone_obj = PnObject_Clone(world, o);
				World_PutManagedObject(world, clone_obj);
				PnObject_PutAttr(world, obj, key, clone_obj);
			}
		} while (Hash_Iterator_Advance(itr));
		free(itr);
	}
	
	return obj;
}

/**
 * PnObject_Clone 에 대한, peanut 안에서 호출되는 함수
 */
static pn_object *
PnObject_CloneObject(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(length == 0);
	return PnObject_Clone(world, object);
}

/**
 * 현재 객체와 다른 객체가 같은 값을 갖고 있는지 비교하는 함수이다.
 */
static pn_object *
PnObject_EqualObject(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(length == 1);
	
	pn_object *eqfn = PnObject_GetAttr(object, "=="); // 오버라이딩되거나 구현된 == 함수가 있는지 찾는다.
	pn_object *result = NULL;
	
	if (eqfn == NULL)
	{
		if (IS_NATIVE(object) || IS_FUNCTION(object))
		{
			pn_object *other = params[0];
			if (object->type == other->type && object->obj_val == other->obj_val)
			{
				result = PnInteger_Create(world, 1);
			}
			else
			{
				result = PnInteger_Create(world, 0);
			}
		}
		else
		{
			PN_FAIL("PnObject_EqualObject failed. eqfn is invalid");
		}
	}
	else
	{
		if (IS_OBJECT(object) && IS_NATIVE(eqfn) && eqfn->func.body_pointer == PnObject_EqualObject)
		{
			pn_object *other = params[0];
			if (IS_OBJECT(other) && object->obj_val == other->obj_val)
			{
				result = PnInteger_Create(world, 1);
			}
			else
			{
				result = PnInteger_Create(world, 0);
			}
		}
		else
		{
			result = PnFunction_ExecuteByFuncObject(eqfn, world, object, NULL, 0);
		}
	}

	return result;
}

/**
 * 현재 객체와 다른 객체가 같은 주소를 갖고 있는지 비교하는 함수이다.
 */
static pn_object *
PnObject_EqualReferenceObject(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(length == 1);
	pn_object *result = PnInteger_Create(world, 0);
	pn_object *other = params[0];
	if (object->obj_val == other->obj_val)
		result->int_val = 1;
	return result;
}

/**
 * 현재 객체를 PnString 으로 바꿔주는 함수이다. (함수 바인딩에 사용되는 함수)
 */
static pn_object *
PnObject_ToStr(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(length == 0);
	return PnObject_ToString(world, object);
}

/**
 * 현재 객체를 PnString 으로 바꿔주는 함수이다. (Public)
 */
pn_object *
PnObject_ToString(pn_world *world, pn_object *object)
{
	if (IS_STRING(object))
	{
		return object;
	}
	
	pn_object *result = NULL;
	pn_object *to_str = PnObject_GetAttr(object, "to_str");
	
	if (IS_NATIVE(object))
	{
		char *str = pn_alloc(20);
		sprintf(str, "NATIVE(%p)", object->func.body_pointer);
		result = PnString_Create(world, str);
		free(str);
	}
	else if(IS_FUNCTION(object))
	{
		char *str = pn_alloc(20);
		sprintf(str, "FUNC(%p)", object->func.body_node);
		result = PnString_Create(world, str);
		free(str);
	}
	else if (!IS_OBJECT(object) && to_str != NULL)
	{
		result = PnFunction_ExecuteByFuncObject(to_str, world, object, NULL, 0);
	}
	else if(IS_OBJECT(object))
	{
		// FIXME: 문자열의 길이가 TO_STRING_BUF 보다 작아야 하는데 바꿔야함
		char *buf = pn_alloc(TO_STRING_BUF);
		memset(buf, 0, TO_STRING_BUF);
		strcat(buf, "{");
		if (Hash_Count(object->obj_val->members) > 0)
		{
			hash_itr *itr = Hash_Iterator(object->obj_val->members);
			if (itr != NULL)
			{
				do {
					char *key = Hash_Iterator_Key(itr);
					pn_object *value = Hash_Iterator_Value(itr);
					pn_object *toStr = PnObject_ToString(world, value);
					PN_ASSERT(IS_STRING(toStr));
					strcat(buf, "'");
					strcat(buf, key);
					strcat(buf, "'");
					strcat(buf, " => ");
					strcat(buf, toStr->str_val);
					strcat(buf, ", ");
				} while(Hash_Iterator_Advance(itr));
			}
			free(itr);
		}
		strcat(buf, "}");
		
		int len = strlen(buf);
		char *str = pn_alloc(len + 1);
		strcpy(str, buf);
		result = PnString_Create(world, str);
		free(buf);
		free(str);
	}
	else
	{
		//PN_FAIL("PnObject_ToString failed. invalid type");
		result = PnString_Create(world, "<Undefined>");
	}
	return result;
}

/**
 * 현재 객체를 PnString 으로 바꿔주는 함수이다.
 */
static pn_object *
PnObject_Add(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(length == 1);
	pn_object *other = params[0];
	if (IS_STRING(object) || IS_STRING(other))
	{
		pn_object *str = object;
		if (!IS_STRING(object))
			str = PnObject_ToString(world, object);
		pn_object *params[1] = { other };
		return PnString_Add(world, str, params, 1);
	}
	else
	{
		PN_FAIL("PnObject_Add failed. invalid object, other type");
	}
}

pn_object *
PnObject_CreateEmptyObject(pn_world *world)
{
	pn_object *obj = PnObject_CreateEmptyObjectByNotMembers(world);
	obj->obj_val->members = Hash_Create();
	
	PnObject_PutAttr(world, obj, "clone", PnFunction_CreateByNative(world, PnObject_CloneObject));
	PnObject_PutAttr(world, obj, "==", PnFunction_CreateByNative(world, PnObject_EqualObject));
	PnObject_PutAttr(world, obj, "===", PnFunction_CreateByNative(world, PnObject_EqualReferenceObject));
	PnObject_PutAttr(world, obj, "to_str", PnFunction_CreateByNative(world, PnObject_ToStr));
	PnObject_PutAttr(world, obj, "+", PnFunction_CreateByNative(world, PnObject_Add));
	
	return obj;
}

pn_object *
PnObject_CreateEmptyObjectByNotMembers(pn_world *world)
{
	pn_object *obj = pn_alloc(sizeof(pn_object));
	PN_ASSERT(obj != NULL);
	memset(obj, 0, sizeof(pn_object));
	obj->type = TYPE_OBJECT;
	
	obj->obj_val = pn_alloc(sizeof(pn_object_val));
	obj->obj_val->ref_count = 0;
	obj->obj_val->members = NULL;
	
	World_PutManagedObject(world, obj);

	return obj;
}

pn_object *
PnObject_CreateFromReference(pn_world *world, pn_object *other)
{
	PN_ASSERT(other != NULL);
	
	pn_object *obj = pn_alloc(sizeof(pn_object));
	PN_ASSERT(obj != NULL);
	memset(obj, 0, sizeof(pn_object));

	if (IS_INTEGER(other))
	{
		obj->type = TYPE_INTEGER;
		obj->int_val = other->int_val;
	}
	else if (IS_REAL(other))
	{
		obj->type = TYPE_REAL;
		obj->real_val = other->real_val;
	}
	else if (IS_STRING(other))
	{
		obj->type = TYPE_STRING;
		obj->str_val = other->str_val;
	}
	else if (IS_NATIVE(other))
	{
		obj->type = TYPE_NATIVE;
		obj->func.body_pointer = other->func.body_pointer;
	}
	else if (IS_FUNCTION(other))
	{
		obj->type = TYPE_FUNCTION;
		obj->func.body_node = other->func.body_node;
	}
	else if (IS_OBJECT(other))
	{
		obj->type = TYPE_OBJECT;
	}
	else if (IS_BOOL(other))
	{
		obj->type = TYPE_BOOL;
		obj->bool_val = other->bool_val;
	}
	else if (IS_NULL(other))
	{
		obj->type = TYPE_NULL;
	}
	else
	{
		printf("type is invalid %d\n", other->type);
		PN_FAIL("PnObject_CreateFromReference failed");
	}
	
	pn_object_val *obj_val = other->obj_val;
	PN_ASSERT(obj_val != NULL);
	obj_val->ref_count++;
	obj->obj_val = obj_val;

	World_PutManagedObject(world, obj);
	
	return obj;
}

hash_itr *
PnObject_GetAllAttributes(pn_object *object)
{
	PN_ASSERT(object != NULL);
	PN_ASSERT(object->obj_val);
	PN_ASSERT(object->obj_val->members);
	return Hash_Iterator(object->obj_val->members);
}

pn_object *
PnObject_GetAttr(pn_object *obj, const char *name)
{
	if (obj->obj_val->members == NULL)
		return NULL;
	return (pn_object *) Hash_Get(obj->obj_val->members, name);
}

void
PnObject_PutAttr(pn_world *world, pn_object *obj, const char *name, pn_object *value)
{
	Hash_Put(obj->obj_val->members, name, value);
/*
	// FIXME 여기에서 삭제를 하면, 부모 클래스의 함수를 삭제하는 일이 생기지 않나?
	pn_object *val = PnObject_CreateFromReference(world, value);
	pn_object *old = Hash_Put(obj->obj_val->members, name, val);
	
	if (old != NULL)
		PnObject_Destroy(old);
*/
}

void
PnObject_Destroy(pn_object *obj)
{
	PN_ASSERT(obj != NULL);
	
	if (obj->obj_val != NULL)
	{
		obj->obj_val->ref_count--;
		PN_ASSERT(obj->obj_val->ref_count >= 0);
		
		if (obj->obj_val->ref_count == 0)
		{
			if (obj->obj_val->members != NULL)
			{
				//PN_ASSERT(Hash_Count(obj->obj_val->members) > 0);
				hash_itr *itr = Hash_Iterator(obj->obj_val->members);
				do {
					pn_object *o = Hash_Iterator_Value(itr);
					if (o != NULL)
						PnObject_Destroy(o);
				} while (Hash_Iterator_Advance(itr));
				free(itr);

				Hash_Destroy(obj->obj_val->members, false);
			}
			free(obj->obj_val);
			
			if (obj->type == TYPE_STRING)
			{
				free(obj->str_val);
			}
		}
	}
	free(obj);
}

/**
 * 해당 객체가 참인지 확인한다.
 */
bool
PnObject_IsTrue(pn_object *value)
{
	PN_ASSERT(value != NULL);
	
	if (IS_NULL(value))
		return false;
	
	else if (IS_INTEGER(value) && value->int_val == 0)
		return false;
	
	else if (IS_REAL(value) && value->real_val == 0.0)
		return false;
	
	else if (IS_STRING(value) && value->str_val == NULL)
		return false;
	
	else if (IS_BOOL(value) && value->bool_val == false)
		return false;
	
	return true;
}

pn_object *
PnObject_CreateByNode(pn_world *world, pn_node *node)
{
	PN_ASSERT(node != NULL);
	PN_ASSERT(node->node_type == NODE_LITERAL);
	return PnObject_CreateByObject(world, &node->value);
}

pn_object *
PnObject_CreateByObject(pn_world *world, pn_object *object)
{
	pn_object *result = NULL;
	
	if (IS_INTEGER(object))
		result = PnInteger_Create(world, object->int_val);
	else if (IS_REAL(object))
		result = PnReal_Create(world, object->real_val);
	else if (IS_STRING(object))
		result = PnString_Create(world, object->str_val);
	else if (IS_NULL(object))
		result = PnNull_Create(world);
	else
		PN_FAIL("never run code here");
	
	PN_ASSERT(result != NULL);
	return result;
}

/**
 * child 가 super 를 상속한다.
 * 실제로는, super의 모든 항목을 clone()해서, child에 복사한다.
 * 이미 존재하는 항목이면, 덮어쓴다. (일종의 Override)
 */
pn_object *
PnObject_Inherit(pn_world *world, pn_object *super, pn_object *child)
{
	// super 에 있는 해쉬 목록을 하나씩 가져와서, child 에 덮어쓴다.
	hash_itr *iterator = PnObject_GetAllAttributes(super);
	PN_ASSERT(iterator != NULL);
	do {
		char *key = Hash_Iterator_Key(iterator);
		pn_object *value = (pn_object *)Hash_Iterator_Value(iterator);
		
		// 새 객체를 deep copy 해서, 복사한다.
		pn_object *cloned = PnObject_Clone(world, value);
		
		// TODO: 이미 해당 key가 있는지 검사해야 한다. 있으면, 지우고 넣어야 하는데.. Hash_Put 을 수정하는게 나을듯?
		
		PnObject_PutAttr(world, child, key, cloned);
	} while (Hash_Iterator_Advance(iterator));
	
	return child;
}

pn_object *
PnObject_CreateBool(pn_world *world)
{
	return PnBool_Create(world, false);
}

pn_object *
PnObject_CreateInteger(pn_world *world)
{
	return PnInteger_Create(world, 0);
}

pn_object *
PnObject_CreateReal(pn_world *world)
{
	return PnReal_Create(world, 0.0);
}

pn_object *
PnObject_CreateString(pn_world *world)
{
	return PnString_Create(world, "");
}

pn_object *
PnObject_CreateNull(pn_world *world)
{
	return PnNull_Create(world);
}
