#include "world.h"
#include "hash.h"
#include "stack.h"
#include "pn_object.h"
#include "pn_list.h"
#include "pn_hash.h"
#include "pn_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STACK_BASE 0

pn_world *
World_Create()
{
	pn_world *w = (pn_world *)pn_alloc(sizeof(pn_world));
	w->scope = Stack_Create();
	//w->tree = NULL;
	w->tree_nodes = NULL;
	w->len_tree_nodes = 0;
	
	World_StartScope(w, NULL);
	
	// 초기화 객체들을 집어넣는다.
	World_PutObject(w, PROTONAME_BOOL, PnBool_CreatePrototype(w), false);
	World_PutObject(w, PROTONAME_INTEGER, PnInteger_CreatePrototype(w), false);
	World_PutObject(w, PROTONAME_REAL, PnReal_CreatePrototype(w), false);
	World_PutObject(w, PROTONAME_STRING, PnString_CreatePrototype(w), false);
	
	World_PutObject(w, PROTONAME_LIST, PnList_CreatePrototype(w), false);
	World_PutObject(w, PROTONAME_HASH, PnHash_CreatePrototype(w), false);
	
	// 클론하지 않는 객체들
	World_PutObject(w, PROTONAME_NULL, PnNull_CreatePrototype(w), false);
	World_PutObject(w, PROTONAME_STDIO, PnStdio_CreatePrototype(w), false);
	
	return w;
}

static void
__RemoveHash(stack *s)
{
	while (!Stack_IsEmpty(s))
	{
		scope_item *item = Stack_Pop(s);
		PN_ASSERT(item != NULL);

		hash_itr *itr = Hash_Iterator(item->h);
		do {
			pn_object *o = Hash_Iterator_Value(itr);
			if (o != NULL)
				PnObject_Destroy(o);
		} while (Hash_Iterator_Advance(itr));
		free(itr);

		Hash_Destroy(item->h, false);
	}
	Stack_Destroy(s);
}

void
World_Destroy(pn_world *w)
{
	World_EndScope(w);
	__RemoveHash(w->scope);
	//__RemoveHash(w->except);
	free(w);
}

void
World_StartScope(pn_world *w, pn_node *pc)
{
	scope_item *item = pn_alloc(sizeof(scope_item));
	item->h = Hash_Create();
	item->l = List_Create();
	Stack_Push(w->scope, item);
}

void
World_EndScope(pn_world *w)
{
	scope_item *item = Stack_Pop(w->scope);
	hash *h = item->h;
	PN_ASSERT(item != NULL);
	
	hash_itr *itr = Hash_Iterator(h);
	if (Hash_Count(h) > 0)
	{
		do {
			pn_object *o = Hash_Iterator_Value(itr);
			if (o != NULL)
			{
				PnObject_Destroy(o);
			}
		} while (Hash_Iterator_Advance(itr));
	}
	free(itr);
	
	// FIXME: 현재 스코프별 변수 관리 리스트에 들어있는 객체들을 해제하지 않음(버그)
	//List_Destroy(item->l, (item_deleter)PnObject_Destroy);
	
	Hash_Destroy(h, false);
	
	free(item);
}

pn_object *
World_GetObject(pn_world *w, const char *name)
{
	scope_item *item = NULL;
	hash *h = NULL;
	pn_object *var = NULL;
	int size = Stack_Size(w->scope);
	while (var == NULL && size > 0)
	{
		item = Stack_Get(w->scope, --size);
		h = item->h;
		PN_ASSERT(h != NULL);
		var = Hash_Get(h, name);
	}
	return var;
}

/**
 * 해당 name 으로, 전체 스코프에서 찾아보고, 가장 가까운 곳에 v 를 업데이트한다.
 * replacement 가 true 이면, 무조껀 현재 scope에 넣는다.
 */
void
World_PutObject(pn_world *w, const char *name, pn_object *v, bool replacement)
{
	pn_object *var = World_GetObject(w, name);
	
	// replacement가 true 면 무조건 현재 스코프에 변수를 추가한다.
	if (replacement || var == NULL)
	{
		scope_item *item = Stack_Top(w->scope);
		hash *top = item->h;
		var = Hash_Remove(top, name);
		if (var != NULL)
		{
			PnObject_Destroy(var);
		}
		// 해시에 넣는 케이스는 100% 레퍼런스가 증가하기 때문에 무조건 레퍼런스를 증가시킨다.
		Hash_Put(top, name, PnObject_CreateFromReference(w, v));
	}
	else
	{
		int i;
		int size = Stack_Size(w->scope);
		for (i = size - 1; i >= 0; i--)
		{
			scope_item *item = Stack_Get(w->scope, i);
			hash *h = item->h;
			if (Hash_Get(h, name) != NULL)
			{
				var = Hash_Remove(h, name);
				Hash_Put(h, name, PnObject_CreateFromReference(w, v));
				PnObject_Destroy(var);
				break;
			}
		}
	}
}

void
World_PutManagedObject(pn_world *w, pn_object *v)
{
	scope_item *item = Stack_Top(w->scope);
	if (v->obj_val->ref_count == 0)
		v = PnObject_CreateFromReference(w, v);
	List_AppendItem(item->l, v);
}

/**
 * 스택의 바닥에서 전역함수 또는 클래스를 가져온다.
 */
pn_object *
World_GetObjectAtBase(pn_world *w, const char *name)
{
	pn_object *var = NULL;
	scope_item *item = Stack_Get(w->scope, STACK_BASE);
	PN_ASSERT(item != NULL);
	hash *h = item->h;
	PN_ASSERT(h != NULL);
	var = Hash_Get(h, name);
	return var;
}

/**
 * 스택의 바닥에다가 전역함수 또는 클래스를 삽입한다.
 */
void
World_PutObjectAtBase(pn_world *w, const char *name, pn_object *v)
{
	pn_object *var = World_GetObjectAtBase(w, name);
	
	if (var == NULL)
	{
		scope_item *item = Stack_Top(w->scope);
		Hash_Put(item->h, name, PnObject_CreateFromReference(w, v));
	}
	else
	{
		scope_item *item = Stack_Get(w->scope, STACK_BASE);
		if ((var = Hash_Get(item->h, name)) != NULL)
		{
			Hash_Remove(item->h, name);
			PnObject_Destroy(var);
			Hash_Put(item->h, name, PnObject_CreateFromReference(w, v));
		}
	}
}

void
World_RemoveObject(pn_world *w, const char *name)
{
	
	scope_item *item = Stack_Top(w->scope);
	
	pn_object *obj = (pn_object *)Hash_Remove(item->h, name);
	PnObject_Destroy(obj);
}
