#include "pn_null.h"
#include "pn_object.h"
#include "pn_function.h"
#include "pn_string.h"

pn_object *
PnNull_IsNull(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(length == 0);
	pn_object *result = PnInteger_Create(world, 1);
	return result;
}

pn_object *
PnNull_ToString(pn_world *world, pn_object *object, pn_object *params[], int length)
{
	PN_ASSERT(length == 0);
	return PnString_Create(world, "(null)");
}

pn_object *
PnNull_Create(pn_world *world)
{
	pn_object *proto = World_GetObjectAtBase(world, PROTONAME_NULL);
	PN_ASSERT(proto != NULL);
	pn_object *o = PnObject_Clone(world, proto);
	PN_ASSERT(o != NULL);
	return o;
}

pn_object *
PnNull_CreatePrototype(pn_world *world)
{
	pn_object *o = PnObject_CreateEmptyObject(world);
	o->type = TYPE_NULL;
	
	// 메소드 추가.
	PnObject_PutAttr(world, o, "null?", PnFunction_CreateByNative(world, PnNull_IsNull));
	PnObject_PutAttr(world, o, "to_str", PnFunction_CreateByNative(world, PnNull_ToString));
	
	return o;
}
