#include "pnfunction.h"

#include <stdlib.h>
#include <string.h>
#include "eval.h"

pn_object *PnFunction_CreateByNative(pn_world *world, pn_function_native func_body)
{
    pn_object *obj = PnObject_CreateEmptyObjectByNotMembers(world);
    obj->type = TYPE_NATIVE;
    obj->val.func.body_pointer = func_body;
    return obj;
}

/**
 * create by function node
 * func_code is must function node, not function's contents node
 */
pn_object *PnFunction_CreateByNode(pn_world *world, pn_node *func_code)
{
    pn_object *obj = PnObject_CreateEmptyObjectByNotMembers(world);
    obj->type = TYPE_FUNCTION;
    obj->val.func.body_node = func_code;
    return obj;
}

pn_object *PnFunction_ExecuteByObject(const char *name, pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(object != NULL);
    PN_ASSERT(object->type != TYPE_NOT_INITIALIZED);
    pn_object *pn_func = PnObject_GetAttr(object, name);
    PN_ASSERT(pn_func != NULL);
    return PnFunction_ExecuteByFuncObject(pn_func, world, object, params, length);
}

pn_object *PnFunction_ExecuteByFuncObject(pn_object* pn_func, pn_world *world, pn_object *object, pn_object *params[], int length)
{
    PN_ASSERT(pn_func != NULL);

    pn_object *value = NULL;
    if (IS_NATIVE(pn_func)) {
        PN_ASSERT(pn_func->val.func.body_pointer != NULL);
        value = (*(pn_func->val.func.body_pointer))(world, object, params, length);
    } else if (IS_FUNCTION(pn_func)) {
        PN_ASSERT(pn_func->val.func.body_node != NULL);
        pn_node *stmt_list = NULL;

        if (pn_func->val.func.body_node->node_type == NODE_DEF_FUNC)
            stmt_list = pn_func->val.func.body_node->def_func.stmt_list;
        else if (pn_func->val.func.body_node->node_type == NODE_LAMBDA)
            stmt_list = pn_func->val.func.body_node->lambda.stmt_list;
        else
            PN_FAIL("bad pn_func");

        value = execute_statements(world, stmt_list);
    } else {
        fprintf(stderr, "pn_func->type = %d\n", pn_func->type);
        PN_FAIL("never run here");
    }

    // TODO: need check error, when function is none

    return value;
}
