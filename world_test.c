#include <stdlib.h>

#include "world.h"
#include "globals.h"
#include "pn_integer.h"
#include "pn_function.h"
#include "pn_string.h"
#include "stack.h"

int
main()
{
	/*pn_world *w = World_Create();
	
	pn_object *obj = pn_alloc(sizeof(pn_object));
	obj->type = TYPE_INTEGER;
	obj->int_val = 1234;
	World_PutObject(w, "jweb", obj);
	pn_object *obj2 = World_GetObject(w, "jweb");
	printf("&v2=%p\n", (void*)obj2);
	printf("v2=%d\n", obj2->int_val);
	
	World_StartScope(w);
	obj2 = World_GetObject(w, "jweb");
	printf("&v2=%p\n", (void *)obj2);
	
	pn_object obj3;
	obj3.type = TYPE_REAL;
	obj3.real_val = 3.14;
	
	World_PutObject(w, "jong10", &obj3);
	printf("v4 = %p\n", World_GetObject(w, "jong10"));
	World_EndScope(w);
	
	printf("v4 = %p\n", World_GetObject(w, "jong10"));
	
	pn_object *int1 = PnInteger_Create(3);
	pn_object *int2 = PnInteger_Create(10);
	pn_object *params[1] = { int2 };
	pn_object *ret = PnFunction_Execute_ByObject("%", w, int1, params, 1);
	printf("ret : %d\n", ret->int_val);
	ret = PnFunction_Execute_ByObject("to_str", w, int2, NULL, 0);
	printf("ret : %s\n", ret->str_val);

	pn_object *str = PnString_Create("jong10aws;elrk;walekr");
	ret = PnFunction_Execute_ByObject("*", w, str, params, 1);
	printf("str result : %s\n", ret->str_val);
	World_Destroy(w);*/
	stack *t = Stack_Create();
	char *test = "123";
	printf("%p\n", test);
	Stack_Push(t, test);
	void *p = Stack_Pop(t);
	printf("%p\n", p);
	//void *p2 = Stack_Pop(t);
	//printf("%p\n", p2);
	Stack_Destroy(t);
	return 0;
}

