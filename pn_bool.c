#include "pn_bool.h"
#include "pn_function.h"

static pn_object *
PnBool_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(length == 0);
	
	return PnString_Create(world, object->bool_val ? "true" : "false");
}

pn_object *
PnBool_Create(pn_world *world, bool value)
{
	pn_object *proto = World_GetObject(world, PROTONAME_BOOL);
	PN_ASSERT(proto != NULL);
	pn_object *o = PnObject_Clone(world, proto);
	PN_ASSERT(o != NULL);
	o->bool_val = value;
	return o;
}

pn_object *
PnBool_CreatePrototype(pn_world *world)
{
	pn_object *o = PnObject_CreateEmptyObject(world);
	o->type = TYPE_BOOL;
	o->bool_val = false;
	
	// TODO: 함수 목록을 넣는다.
	PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnBool_ToString));
	
	return o;
}

