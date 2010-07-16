#define PEANUT_DEBUG
#include "eval.h"
#include "world.h"
#include "pn_function.h"
#include "pn_list.h"
#include "pn_hash.h"

#include <stdlib.h>
#include <string.h>


static void
print_value(pn_world *world, pn_object *value)
{
	PN_ASSERT(value != NULL);
	PN_ASSERT(value->type != TYPE_NOT_INITIALIZED);
	
	pn_object *str = NULL;
	
	if (IS_FUNCTION(value) || IS_NATIVE(value))
	{
		str = PnObject_ToString(world, value);
	}
	else
	{
		str = PnFunction_ExecuteByObject("to_str", world, value, NULL, 0);
	}
	
	if (IS_NULL(value))
		printf(">> (null)\n");
	else if (IS_INTEGER(value))
		printf(">>(int) : %s\n", str->str_val);
	else if (IS_REAL(value))
		printf(">> (real): %s\n", str->str_val);
	else if (IS_STRING(value))
		printf(">> (str) : %d, '%s'\n", (int)strlen(str->str_val), str->str_val);
	else if (IS_OBJECT(value))
		printf(">> (obj) : %s\n", str->str_val);
	else if (IS_NATIVE(value))
		printf(">> (code native) : %s\n", str->str_val);
	else if (IS_FUNCTION(value))
		printf(">> (code) : %s\n", str->str_val);
	else if (IS_BOOL(value))
		printf(">> (bool) : %s\n", str->str_val);
	else
		printf(">> (undefined!! %d)\n", value->type);
}

static int
count_siblings(pn_node *node)
{
	int count = 0;
	pn_node *cur = node;
	while (cur != NULL)
	{
		count++;
		cur = cur->sibling;
	}
	return count;
}

static pn_object *
evaluate_literal(pn_world *world, pn_node *node)
{
	PN_ASSERT(node != NULL);
	pn_object *literal = PnObject_CreateByNode(world, node);
	//World_PutLiteralObject(world, literal);
	return literal;
}

static pn_object *
evaluate_var_name(pn_world *world, pn_node *node)
{
	PN_ASSERT(node != NULL);
	pn_object *var = World_GetObject(world, node->var_name);
	return var;
}

static pn_object *
evaluate_list(pn_world *world, pn_node *node)
{
	PN_ASSERT(node != NULL);
	pn_object *o = PnList_Create(world);
	list *l = o->obj_val->extra_val;
	
	pn_node *item_node = node->list_items;
	while (item_node != NULL)
	{
		//pn_object *item_obj = PnObject_CreateByNode(item_node);
		pn_object *item_obj = execute_one_step(world, item_node);
		List_AppendItem(l, item_obj);
		item_node = item_node->sibling;
	}
	
	return o;
}

static pn_object *
evaluate_hash(pn_world *world, pn_node *node)
{
	PN_ASSERT(node != NULL);
	pn_object *o = PnHash_Create(world);
	hash *h = o->obj_val->extra_val;

	pn_hash_item *item_node = node->hash_items;
	while (item_node != NULL)
	{
		pn_object *value = execute_one_step(world, item_node->value);
		Hash_Put(h, item_node->key->value.str_val, value);
		item_node = item_node->next_item;
	}
	
	return o;
}

static pn_object *
evaluate_expression(pn_world *world, pn_node *node)
{
	PN_ASSERT(node != NULL);
	
	pn_object *result = NULL;
	pn_object *object = NULL;
	char * func_name = node->expr.func_name;
	
	// 왼쪽 부분을 재귀적으로 계산한다.
	// a.b.c.d() 이면, (((a.b).c).d()) 의 순서로 계산된다.
	if (node->expr.object != NULL)
		object = execute_one_step(world, node->expr.object);
	
	// 여기서, 파라미터를 모아서, 던져준다.
	int i, size = count_siblings(node->expr.params);
	pn_object **params = (pn_object **)pn_alloc(sizeof(pn_object *) * size);
	pn_node *cur = node->expr.params;
	pn_node *var_name_cur = NULL;
	
	// 파라미터들을 넘기기 전에 실행한다.
	for (i = 0; cur != NULL && i < size; i++)
	{
		params[i] = execute_one_step(world, cur);
		cur = cur->sibling;
	}
	
	// 호출하고, 전역함수 호출일 경우!
	if (node->expr.object == NULL)
	{
		pn_object *pn_func = World_GetObject(world, func_name);
		if (pn_func != NULL)
		{
			World_StartScope(world, node);
			//printf("expression StartScope\n");
			if (pn_func->func.body_node->node_type == NODE_DEF_FUNC)
				var_name_cur = pn_func->func.body_node->def_func.simple_var_list;
			else if (pn_func->func.body_node->node_type == NODE_LAMBDA)
				var_name_cur = pn_func->func.body_node->lambda.simple_var_list;
			else
				PN_FAIL("error. bad pn_func");
			
			size = count_siblings(var_name_cur);
			
			for (i = 0; var_name_cur != NULL && i < size; i++)
			{
				World_PutObject(world, var_name_cur->var_name, params[i], true);
				var_name_cur = var_name_cur->sibling;
			}
			PN_ASSERT(node->expr.object == NULL);
			result = PnObject_CreateFromReference(world, PnFunction_ExecuteByFuncObject(pn_func, world, NULL, params, size));
			//printf("expression EndScope\n");
			World_EndScope(world);
		}
		else
		{
			printf("error: undefined function: %s\n", func_name);
			PN_ASSERT(!"fail. undefined function.");
			// TODO: 나중에 여긴 사실, NullObject 를 리턴하도록 고쳐야 한다.
		}
	}
	
	// 대입문
	else if (strcmp("=", func_name) == 0)
	{
		PN_ASSERT(size == 1);	// 사실, 문법적으로, 대입문에서 파라미터가 여러개 올 수 없다.
		object = params[0];
		
		// FIXME: object 가 literal, list, hash 등이면, 대입이 안되야 한다. -_-;;
		// FIXME; NullObject 처럼, ErrorObject 같은걸 리턴해서, 에러라고 출력해줘야 할듯. 줄번호 표시해주고.
		
		World_PutObject(world, node->expr.object->var_name, object, false);
		result = object;
	}
	
	// 프로퍼티
	else if (strcmp("@", func_name) == 0)
	{
		result = PnObject_GetAttr(object, node->expr.params->var_name);
	}
	
	// 일반 수식, 객체를 이용한 함수 호출
	else
	{
		World_StartScope(world, node);
		pn_object *pn_func = PnObject_GetAttr(object, func_name);
		PN_ASSERT(pn_func != NULL);
		if (IS_NATIVE(pn_func))
		{
			// do nothing..
		}
		else if (IS_FUNCTION(pn_func))
		{
			var_name_cur = pn_func->func.body_node->def_func.simple_var_list;
			size = count_siblings(var_name_cur);
			
			for (i = 0; var_name_cur != NULL && i < size; i++)
			{
				World_PutObject(world, var_name_cur->var_name, params[i], true);
				var_name_cur = var_name_cur->sibling;
			}
		}
		else
		{
			PN_FAIL("pn_func is not a function.");
		}
		pn_object *executed = PnFunction_ExecuteByFuncObject(pn_func, world, object, params, size);
		PN_ASSERT(executed != NULL);
		result = PnObject_CreateFromReference(world, executed);
		World_EndScope(world);
	}
	
	return result;
}

static pn_object *
evaluate_if_stmt(pn_world *world, pn_node *node)
{
	// if, elseif, else 는, if_stmt.next 로 연결되어 있다.
	// 이 함수는, if_stmt 전체 노드를 처리한다.
	
	PN_ASSERT(node != NULL);
	
	pn_node *cur = node;
	pn_object *last = PnObject_CreateNull(world);
	
	while (cur != NULL)
	{
		pn_node *expr = cur->if_stmt.expr;
		pn_node *stmt_list = cur->if_stmt.stmt_list;
		
		// 둘 다 없을리는 없다.
		PN_ASSERT(expr != NULL || stmt_list != NULL);
		
		// 조건은 없고, 실행할 식이 있으면, ELSE문이다. 식을 실행한 결과를 리턴하고, if문을 끝낸다.
		if (expr == NULL && stmt_list != NULL)
		{
			World_StartScope(world, node);
			last = execute_statements(world, stmt_list);
			PN_ASSERT(cur->if_stmt.next == NULL);
			World_EndScope(world);
			break;
		}
		
		// 조건이 있으면, 실행할 식이 없더라도, 조건을 실행한다.
		// 조건이 참이면, if 문을 끝내고, 아니면, 계속 간다.
		else if (expr != NULL && stmt_list == NULL)
		{
			pn_object *condition = execute_one_step(world, expr);
			if (PnObject_IsTrue(condition))
			{
				last = PnObject_CreateNull(world);
				break;
			}
		}
		
		// 다 있으면, 조건이 참이면, 식을 실행하고, 결과를 리턴한다.
		// 거짓이면, 계속 간다.
		else
		{
			pn_object *condition = execute_one_step(world, expr);
			if (PnObject_IsTrue(condition))
			{
				World_StartScope(world, node);
				last = execute_statements(world, stmt_list);
				World_EndScope(world);
				break;
			}
		}
		
		// 다음 연결된 if_stmt 이 있으면 계속
		cur = cur->if_stmt.next;
	}

	return last;
}

static pn_object *
evaluate_while_stmt(pn_world *world, pn_node *node)
{
	PN_ASSERT(node != NULL);
	pn_node *expr = node->while_stmt.expr;
	pn_node *stmt_list = node->while_stmt.stmt_list;
	PN_ASSERT(expr != NULL);
	
	pn_object *last = PnObject_CreateNull(world);
	pn_object *condition = NULL;
	
	// 조건식에 들어가는 대입문에 쓰이는 변수의 스코프를 위해서,
	// 스코프를 2겹으로 한다.
	
	World_StartScope(world, NULL);
	while ((condition = execute_one_step(world, expr))
		   && PnObject_IsTrue(condition))
	{
		if (stmt_list != NULL)
		{
			World_StartScope(world, NULL);
			last = execute_statements(world, stmt_list);
			World_EndScope(world);
		}
	}
	World_EndScope(world);
	
	return last;
}

static pn_object *
evaluate_for_stmt(pn_world *world, pn_node *node)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(node != NULL);
	
	char *var_name = node->for_stmt.var_name;
	pn_object *object = execute_one_step(world, node->for_stmt.expr);
	pn_object *last = PnObject_CreateNull(world);
	
	pn_object *iterator = PnFunction_ExecuteByObject("iterator", world, object, NULL, 0);
	
	// var 는 for문 전체 스코프에서 사용되고,
	// for문에서 블럭 안의 내용은 그 블럭 안의 스코프에서 사용되므로,
	// 스코프를 두겹으로 해준다.
	// ..인데, EndScope 에서 죽는다. -_-;;;
	
	//World_StartScope(world, NULL);
	
	while (true)
	{
		pn_object *has_next = PnFunction_ExecuteByObject("hasNext?", world, iterator, NULL, 0);
		PN_ASSERT(IS_BOOL(has_next));
		
		if (!PnObject_IsTrue(has_next))
			break;
		
		pn_object *cur = PnFunction_ExecuteByObject("next", world, iterator, NULL, 0);
		World_PutObject(world, var_name, cur, true);
		
		// FIXME 여기에서, last 부분은, 레퍼런스 카운팅을 어떻게 해야 함?? -_-;;
		
		// 블럭 안의 부분을 실행한다.
		//World_StartScope(world, NULL);
		last = execute_statements(world, node->for_stmt.stmt_list);
		//World_EndScope(world);
	}
	
	//World_EndScope(world);
	
	return last;
}

static pn_object *
evaluate_import_stmt(pn_world *world, pn_node *node)
{
	// todo: evaluate_import_stmt
	return NULL;
}

static pn_object *
evaluate_lambda(pn_world *world, pn_node *node)
{
	pn_object *function = PnFunction_CreateByNode(world, node);
	return function;
}

static pn_object *
evaluate_def_func(pn_world *world, pn_node *node)
{
	pn_object *function = PnFunction_CreateByNode(world, node);
	World_PutObjectAtBase(world, node->def_func.func_id, function);
	return function;
}

static pn_object *
evaluate_def_class(pn_world *world, pn_node *node)
{
	// 이미 해당 이름으로, 클래스가 선언되어 있는지 확인한다.
	pn_object *object = World_GetObject(world, node->def_class.name);
	
	// 없으면, PnObject 를 새로 하나 만들고,
	if (object == NULL)
	{
		object = PnObject_CreateEmptyObject(world);
		object->type = TYPE_OBJECT;
	}
	
	// 상속받는 애들꺼를 순서대로 복사하고,
	pn_node *super = node->def_class.super_list;
	while (super != NULL)
	{
		pn_object *superclass = World_GetObject(world, super->var_name);
		object = PnObject_Inherit(world, superclass, object);
		super = super->sibling;
	}
	
	// 선언된 함수 목록들을 모두 해쉬에 넣고,
	pn_node *func_node = node->def_class.func_list;
	while (func_node != NULL)
	{
		pn_object *function = PnFunction_CreateByNode(world, func_node);
		PnObject_PutAttr(world, object, func_node->def_func.func_id, function);
		func_node = func_node->sibling;
	}
	
	// 현재 스코프 (당연히 최하단 스코프임.)에 Put 한다.
	World_PutObject(world, node->def_class.name, object, false);
	
	// name 에 기반해서, push하는 것이기 때문에, 다시 가져와야 함. -_-;;
	object = World_GetObject(world, node->def_class.name);
	
	return object;
}

static pn_object *
evaluate_return_stmt(pn_world *world, pn_node *node)
{
	PN_ASSERT(world != NULL);
	PN_ASSERT(node != NULL);
	
	int size = Stack_Size(world->scope);
	int i;
	
	pn_node *return_addr = NULL;
	pn_object *return_value = NULL;
	
	// 리턴할 값을 계산한다.
	if (node->return_stmt.expr == NULL)
		return_value = PnObject_CreateNull(world);
	else
		return_value = execute_one_step(world, node->return_stmt.expr);
	
	// 레퍼런스 증가
	return_value = PnObject_CreateFromReference(world, return_value);
	
	// 빠져나가야 하는 스코프 개수
	int scope_depth = 0;
	
	// 스코프의 맨 바깥부터, 리턴할 주소를 찾는다.
	for (i = size - 1; i >= 0; i--)
	{
		scope_depth++;
		
		pn_node *addr = (pn_node *)Stack_Get(world->scope, i);
		if (addr != NULL)
		{
			return_addr = addr;
			break;
		}
	}
	
	// 해당 스코프 개수를 종료
	for (i = 0; i < scope_depth; i++)
		World_EndScope(world);
	
	// FIXME: 리턴할 값은 어떻게 전달한담?
	
	return return_value;
}


// CAUTION !! : globals.h 의 enum _NODE_TYPE 과 순서가 같아야 한다.
static pn_object *(*evaluate_functions[])(pn_world *, pn_node *) = {
	evaluate_literal,
	evaluate_var_name,
	evaluate_list,
	evaluate_hash,
	evaluate_expression,
	evaluate_if_stmt,
	evaluate_while_stmt,
	evaluate_for_stmt,
	evaluate_import_stmt,
	evaluate_lambda,
	evaluate_def_func,
	evaluate_def_class,
	evaluate_return_stmt,
};

pn_object *
execute_statements(pn_world *world, pn_node *node)
{
	pn_node *cur = node;
	pn_object *result = NULL;
	while (cur != NULL)
	{
		result = execute_one_step(world, cur);
		cur = cur->sibling;
	}
	return result;
}

pn_object *
execute_one_step(pn_world *world, pn_node *node)
{
	pn_object *return_value = NULL;
	PN_ASSERT(node != NULL);
	
	//pn_debug("node type = %d\tsibling = %p\n", node->node_type, node->sibling);
	
	// 해당 타입에 맞는 statement 를 실행한다.
	if (0 <= node->node_type && node->node_type < NODE_MAX_COUNT)
		return_value = evaluate_functions[node->node_type](world, node);
	else
		printf("bad node type: %d\n", (int)node->node_type);
	
	//#ifdef PEANUT_DEBUG
	//print_value(return_value);
	//#endif
	
	return return_value;
}

///////////////////////////////////////////////////////////////

/**
 * 코드를 실행하는 함수
 */
pn_object *
Eval_ExecuteTree(pn_world *world, pn_node *tree, bool trace)
{
	pn_node *cur = tree;
	pn_object *result = NULL;
	while (cur != NULL)
	{
		result = execute_one_step(world, cur);
		
		if (trace)
			print_value(world, result);
		
		cur = cur->sibling;
	}
	return result;
}

