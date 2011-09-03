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

pn_world *World_Create()
{
    pn_world *w = (pn_world *)malloc(sizeof(pn_world));
    w->scope = Stack_Create();
    //w->tree = NULL;
    w->tree_nodes = NULL;
    w->len_tree_nodes = 0;

    World_StartScope(w, NULL);

    // initialize primitive object
    World_PutObject(w, PROTONAME_BOOL, PnBool_CreatePrototype(w), false);
    World_PutObject(w, PROTONAME_INTEGER, PnInteger_CreatePrototype(w), false);
    World_PutObject(w, PROTONAME_REAL, PnReal_CreatePrototype(w), false);
    World_PutObject(w, PROTONAME_STRING, PnString_CreatePrototype(w), false);

    World_PutObject(w, PROTONAME_LIST, PnList_CreatePrototype(w), false);
    World_PutObject(w, PROTONAME_HASH, PnHash_CreatePrototype(w), false);

    World_PutObject(w, PROTONAME_NULL, PnNull_CreatePrototype(w), false);
    World_PutObject(w, PROTONAME_STDIO, PnStdio_CreatePrototype(w), false);

    World_PutObject(w, PROTONAME_TRUE, PnBool_Create(w, true), false);
    World_PutObject(w, PROTONAME_FALSE, PnBool_Create(w, false), false);

    return w;
}

static void __RemoveHash(stack *s)
{
    while (!Stack_IsEmpty(s)) {
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

void World_Destroy(pn_world *w)
{
    World_EndScope(w);
    __RemoveHash(w->scope);
    //__RemoveHash(w->except);
    free(w);
}

void World_StartScope(pn_world *w, pn_node *pc)
{
    scope_item *item = malloc(sizeof(scope_item));
    item->h = Hash_Create();
    item->l = List_Create();
    Stack_Push(w->scope, item);
}

void World_EndScope(pn_world *w)
{
    scope_item *item = Stack_Pop(w->scope);
    hash *h = item->h;
    PN_ASSERT(item != NULL);

    hash_itr *itr = Hash_Iterator(h);
    if (Hash_Count(h) > 0) {
        do {
            pn_object *o = Hash_Iterator_Value(itr);
            if (o != NULL) {
                PnObject_Destroy(o);
            }
        } while (Hash_Iterator_Advance(itr));
    }
    free(itr);

    // FIXME: BUG, need debug
    //List_Destroy(item->l, (item_deleter)PnObject_Destroy);

    Hash_Destroy(h, false);

    free(item);
}

pn_object *World_GetObject(pn_world *w, const char *name)
{
    scope_item *item = NULL;
    hash *h = NULL;
    pn_object *var = NULL;
    int size = Stack_Size(w->scope);
    while (var == NULL && size > 0) {
        item = Stack_Get(w->scope, --size);
        h = item->h;
        PN_ASSERT(h != NULL);
        var = Hash_Get(h, name);
    }
    return var;
}

/**
 * find all scope the name, and update `v` in the nearest scope
 * if replacement is true, it must put to current scope
 */
void World_PutObject(pn_world *w, const char *name, pn_object *v, bool replacement)
{
    pn_object *var = World_GetObject(w, name);

    if (replacement || var == NULL) {
        scope_item *item = Stack_Top(w->scope);
        hash *top = item->h;
        var = Hash_Remove(top, name);
        if (var != NULL) {
            PnObject_Destroy(var);
        }
        Hash_Put(top, name, PnObject_CreateFromReference(w, v));
    } else {
        int i;
        int size = Stack_Size(w->scope);
        for (i = size - 1; i >= 0; i--) {
            scope_item *item = Stack_Get(w->scope, i);
            hash *h = item->h;
            if (Hash_Get(h, name) != NULL) {
                var = Hash_Remove(h, name);
                Hash_Put(h, name, PnObject_CreateFromReference(w, v));
                PnObject_Destroy(var);
                break;
            }
        }
    }
}

void World_PutManagedObject(pn_world *w, pn_object *v)
{
    scope_item *item = Stack_Top(w->scope);
    if (v->obj_val->ref_count == 0)
        v = PnObject_CreateFromReference(w, v);
    List_AppendItem(item->l, v);
}

/**
 * get global function or class in base of world stack
 */
pn_object *World_GetObjectAtBase(pn_world *w, const char *name)
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
 * put a function or class to base of world stack
 */
void World_PutObjectAtBase(pn_world *w, const char *name, pn_object *v)
{
    pn_object *var = World_GetObjectAtBase(w, name);

    if (var == NULL) {
        scope_item *item = Stack_Top(w->scope);
        Hash_Put(item->h, name, PnObject_CreateFromReference(w, v));
    } else {
        scope_item *item = Stack_Get(w->scope, STACK_BASE);
        if ((var = Hash_Get(item->h, name)) != NULL) {
            Hash_Remove(item->h, name);
            PnObject_Destroy(var);
            Hash_Put(item->h, name, PnObject_CreateFromReference(w, v));
        }
    }
}

void World_RemoveObject(pn_world *w, const char *name) {

    scope_item *item = Stack_Top(w->scope);

    pn_object *obj = (pn_object *)Hash_Remove(item->h, name);
    PnObject_Destroy(obj);
}
