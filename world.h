#ifndef PEANUT_WORLD_H
#define PEANUT_WORLD_H

#include "globals.h"
#include "stack.h"
#include <stdbool.h>

#define NONAME_HASH_KEY "%noname_vars_count"

typedef struct _scope_item
{
	hash *h;
	list *l;
} scope_item;

pn_world *World_Create();
void World_Destroy(pn_world *w);
void World_StartScope(pn_world *w, pn_node *pc);
void World_EndScope(pn_world *w);
pn_object *World_GetObject(pn_world *w, const char *name);
void World_PutObject(pn_world *w, const char *name, pn_object *v, bool replacement);
void World_PutManagedObject(pn_world *w, pn_object *v);
pn_object *World_GetObjectAtBase(pn_world *w, const char *name);
void World_PutObjectAtBase(pn_world *w, const char *name, pn_object *v);
void World_RemoveObject(pn_world *w, const char *name);

#endif//PEANUT_WORLD_H

