#define PEANUT_DEBUG
#include "eval.h"
#include "world.h"
#include "pnfunction.h"
#include "pnlist.h"
#include "pnhash.h"

#include <stdlib.h>
#include <string.h>


static void print_value(pn_world *world, pn_object *value)
{
    PN_ASSERT(value != NULL);
    PN_ASSERT(value->type != TYPE_NOT_INITIALIZED);

    pn_object *str = NULL;

    if (IS_FUNCTION(value) || IS_NATIVE(value)) {
        str = PnObject_ToString(world, value);
    } else {
        str = PnFunction_ExecuteByObject("to_str", world, value, NULL, 0);
    }

    if (IS_NULL(value))
        printf(">> (null)\n");
    else if (IS_INTEGER(value))
        printf(">>(int) : %s\n", str->val.str_val);
    else if (IS_REAL(value))
        printf(">> (real): %s\n", str->val.str_val);
    else if (IS_STRING(value))
        printf(">> (str) : %d, '%s'\n", (int)strlen(str->val.str_val), str->val.str_val);
    else if (IS_OBJECT(value))
        printf(">> (obj) : %s\n", str->val.str_val);
    else if (IS_NATIVE(value))
        printf(">> (code native) : %s\n", str->val.str_val);
    else if (IS_FUNCTION(value))
        printf(">> (code) : %s\n", str->val.str_val);
    else if (IS_BOOL(value))
        printf(">> (bool) : %s\n", str->val.str_val);
    else
        printf(">> (undefined!! %d)\n", value->type);
}

static int count_siblings(pn_node *node)
{
    int count = 0;
    pn_node *cur = node;
    while (cur != NULL) {
        count++;
        cur = cur->sibling;
    }
    return count;
}

static pn_object *evaluate_literal(pn_world *world, pn_node *node)
{
    PN_ASSERT(node != NULL);
    pn_object *literal = PnObject_CreateByNode(world, node);
    //World_PutLiteralObject(world, literal);
    return literal;
}

static pn_object *evaluate_var_name(pn_world *world, pn_node *node)
{
    PN_ASSERT(node != NULL);
    pn_object *var = World_GetObject(world, node->var_name);
    return var;
}

static pn_object *evaluate_list(pn_world *world, pn_node *node)
{
    PN_ASSERT(node != NULL);
    pn_object *o = PnList_Create(world);
    list *l = o->obj_val->extra_val;

    pn_node *item_node = node->list_items;
    while (item_node != NULL) {
        //pn_object *item_obj = PnObject_CreateByNode(item_node);
        pn_object *item_obj = execute_one_step(world, item_node);
        List_AppendItem(l, item_obj);
        item_node = item_node->sibling;
    }

    return o;
}

static pn_object *evaluate_hash(pn_world *world, pn_node *node)
{
    PN_ASSERT(node != NULL);
    pn_object *o = PnHash_Create(world);
    hash *h = o->obj_val->extra_val;

    pn_hash_item *item_node = node->hash_items;
    while (item_node != NULL) {
        pn_object *value = execute_one_step(world, item_node->value);
        Hash_Put(h, item_node->key->value.val.str_val, value);
        item_node = item_node->next_item;
    }

    return o;
}

static pn_object *evaluate_expression(pn_world *world, pn_node *node)
{
    PN_ASSERT(node != NULL);

    pn_object *result = NULL;
    pn_object *object = NULL;
    char * func_name = node->expr.func_name;

    // evaluate left to right
    // a.b.c.d() is equal (((a.b).c).d())
    if (node->expr.object != NULL)
        object = execute_one_step(world, node->expr.object);

    // getting all parameters
    int i, size = count_siblings(node->expr.params);
    pn_object **params = (pn_object **)malloc(sizeof(pn_object *) * size);
    pn_node *cur = node->expr.params;
    pn_node *var_name_cur = NULL;

    // evaluate paramters before call function
    for (i = 0; cur != NULL && i < size; i++) {
        params[i] = execute_one_step(world, cur);
        cur = cur->sibling;
    }

    // call.
    if (node->expr.object == NULL) {
        /*
           if global call?
        */
        pn_object *pn_func = World_GetObject(world, func_name);
        if (pn_func != NULL) {
            World_StartScope(world, node);
            //printf("expression StartScope\n");
            if (pn_func->val.func.body_node->node_type == NODE_DEF_FUNC)
                var_name_cur = pn_func->val.func.body_node->def_func.simple_var_list;
            else if (pn_func->val.func.body_node->node_type == NODE_LAMBDA)
                var_name_cur = pn_func->val.func.body_node->lambda.simple_var_list;
            else
                PN_FAIL("error. bad pn_func");

            size = count_siblings(var_name_cur);

            for (i = 0; var_name_cur != NULL && i < size; i++) {
                World_PutObject(world, var_name_cur->var_name, params[i], true);
                var_name_cur = var_name_cur->sibling;
            }
            PN_ASSERT(node->expr.object == NULL);
            result = PnObject_CreateFromReference(world, PnFunction_ExecuteByFuncObject(pn_func, world, NULL, params, size));
            //printf("expression EndScope\n");
            World_EndScope(world);
        } else {
            printf("error: undefined function: %s\n", func_name);
            PN_ASSERT(!"fail. undefined function.");
            // TODO: not assert, return NULL object
        }
    } else if (strcmp("=", func_name) == 0) {
        /*
           assignment statement
        */
        PN_ASSERT(size == 1);    // actually, in grammar, cannot 2+ parameters in assignment statement
        object = params[0];

        // FIXME: if object is one of literal, list, hash.. cannot assignment
        // FIXME: return ErrorObject with error message, line no

        World_PutObject(world, node->expr.object->var_name, object, false);
        result = object;
    } else if (strcmp("@", func_name) == 0) {
        /*
           property
        */
        result = PnObject_GetAttr(object, node->expr.params->var_name);
    } else {
        /*
           normal expression, or function of a object
        */
        World_StartScope(world, node);
        pn_object *pn_func = PnObject_GetAttr(object, func_name);
        PN_ASSERT(pn_func != NULL);
        if (IS_NATIVE(pn_func)) {
            // do nothing..
        } else if (IS_FUNCTION(pn_func)) {
            var_name_cur = pn_func->val.func.body_node->def_func.simple_var_list;
            size = count_siblings(var_name_cur);

            for (i = 0; var_name_cur != NULL && i < size; i++) {
                World_PutObject(world, var_name_cur->var_name, params[i], true);
                var_name_cur = var_name_cur->sibling;
            }
        } else {
            PN_FAIL("pn_func is not a function.");
        }
        pn_object *executed = PnFunction_ExecuteByFuncObject(pn_func, world, object, params, size);
        PN_ASSERT(executed != NULL);
        result = PnObject_CreateFromReference(world, executed);
        World_EndScope(world);
    }

    return result;
}

/*
   if, elseif, else
*/
static pn_object *evaluate_if_stmt(pn_world *world, pn_node *node)
{
    PN_ASSERT(node != NULL);

    pn_node *cur = node;
    pn_object *last = PnNull_Create(world);

    while (cur != NULL) {
        pn_node *expr = cur->if_stmt.expr;
        pn_node *stmt_list = cur->if_stmt.stmt_list;

        // impossible situation
        PN_ASSERT(expr != NULL || stmt_list != NULL);

        if (expr == NULL && stmt_list != NULL) {
            /* if not condition and has expression: ELSE statement.
               return evaluated result, and terminate if stetement.
            */
            World_StartScope(world, node);
            last = execute_statements(world, stmt_list);
            PN_ASSERT(cur->if_stmt.next == NULL);
            World_EndScope(world);
            break;
        } else if (expr != NULL && stmt_list == NULL) {
            /* if has condition, evaluate expression
               if condition is true, terminate IF expression, or keep going
            */
            pn_object *condition = execute_one_step(world, expr);
            if (PnObject_IsTrue(condition))
            {
                last = PnNull_Create(world);
                break;
            }
        } else {
            /* if has expression and has statements,
            */
            pn_object *condition = execute_one_step(world, expr);
            if (PnObject_IsTrue(condition))
            {
                World_StartScope(world, node);
                last = execute_statements(world, stmt_list);
                World_EndScope(world);
                break;
            }
        }

        // next if_stmt
        cur = cur->if_stmt.next;
    }

    return last;
}

static pn_object *evaluate_while_stmt(pn_world *world, pn_node *node)
{
    PN_ASSERT(node != NULL);
    pn_node *expr = node->while_stmt.expr;
    pn_node *stmt_list = node->while_stmt.stmt_list;
    PN_ASSERT(expr != NULL);

    pn_object *last = PnNull_Create(world);
    pn_object *condition = NULL;

    // for scope of variables in conditional expression,
    // use double scope here.

    World_StartScope(world, NULL);
    while ((condition = execute_one_step(world, expr))
            && PnObject_IsTrue(condition)) {
        if (stmt_list != NULL) {
            World_StartScope(world, NULL);
            last = execute_statements(world, stmt_list);
            World_EndScope(world);
        }
    }
    World_EndScope(world);

    return last;
}

static pn_object *evaluate_for_stmt(pn_world *world, pn_node *node)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(node != NULL);

    char *var_name = node->for_stmt.var_name;
    pn_object *object = execute_one_step(world, node->for_stmt.expr);
    pn_object *last = PnNull_Create(world);

    pn_object *iterator = PnFunction_ExecuteByObject("iterator", world, object, NULL, 0);

    // it need double scope
    // FIXME but, need debug
    //World_StartScope(world, NULL);

    while (true) {
        pn_object *has_next = PnFunction_ExecuteByObject("hasNext?", world, iterator, NULL, 0);
        PN_ASSERT(IS_BOOL(has_next));

        if (!PnObject_IsTrue(has_next))
            break;

        pn_object *cur = PnFunction_ExecuteByObject("next", world, iterator, NULL, 0);
        World_PutObject(world, var_name, cur, true);

        // FIXME reference counting `last`?

        //World_StartScope(world, NULL);
        last = execute_statements(world, node->for_stmt.stmt_list);
        //World_EndScope(world);
    }

    // FIXME need debug
    //World_EndScope(world);

    return last;
}

static pn_object *evaluate_import_stmt(pn_world *world, pn_node *node)
{
    // todo: evaluate_import_stmt
    return NULL;
}

static pn_object *evaluate_lambda(pn_world *world, pn_node *node)
{
    pn_object *function = PnFunction_CreateByNode(world, node);
    return function;
}

static pn_object *evaluate_def_func(pn_world *world, pn_node *node)
{
    pn_object *function = PnFunction_CreateByNode(world, node);
    World_PutObjectAtBase(world, node->def_func.func_id, function);
    return function;
}

static pn_object *evaluate_def_class(pn_world *world, pn_node *node)
{
    pn_object *object = World_GetObject(world, node->def_class.name);

    // if not the object for class name, create one
    if (object == NULL) {
        object = PnObject_CreateEmptyObject(world);
        object->type = TYPE_OBJECT;
    }

    // inheritance
    pn_node *super = node->def_class.super_list;
    while (super != NULL) {
        pn_object *superclass = World_GetObject(world, super->var_name);
        object = PnObject_Inherit(world, superclass, object);
        super = super->sibling;
    }

    // all functions to hash
    pn_node *func_node = node->def_class.func_list;
    while (func_node != NULL) {
        pn_object *function = PnFunction_CreateByNode(world, func_node);
        PnObject_PutAttr(world, object, func_node->def_func.func_id, function);
        func_node = func_node->sibling;
    }

    // put a object to current scope (last scope)
    World_PutObject(world, node->def_class.name, object, false);

    // get a object again, cause it pushed by name.. -_-;;
    object = World_GetObject(world, node->def_class.name);

    return object;
}

static pn_object *evaluate_return_stmt(pn_world *world, pn_node *node)
{
    PN_ASSERT(world != NULL);
    PN_ASSERT(node != NULL);

    int size = Stack_Size(world->scope);
    int i;

    pn_node *return_addr = NULL;
    pn_object *return_value = NULL;

    // evaluate return expression
    if (node->return_stmt.expr == NULL)
        return_value = PnNull_Create(world);
    else
        return_value = execute_one_step(world, node->return_stmt.expr);

    // increase reference count
    return_value = PnObject_CreateFromReference(world, return_value);

    int scope_depth = 0;

    // find return addr, from outside of scope
    for (i = size - 1; i >= 0; i--) {
        scope_depth++;

        pn_node *addr = (pn_node *)Stack_Get(world->scope, i);
        if (addr != NULL) {
            return_addr = addr;
            break;
        }
    }

    // end counted scopes
    for (i = 0; i < scope_depth; i++)
        World_EndScope(world);

    return return_value;
}

// CAUTION : this order must same with enum _NODE_TYPE in globals.h
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

pn_object *execute_statements(pn_world *world, pn_node *node)
{
    pn_node *cur = node;
    pn_object *result = NULL;
    while (cur != NULL) {
        result = execute_one_step(world, cur);
        cur = cur->sibling;
    }
    return result;
}

pn_object *execute_one_step(pn_world *world, pn_node *node)
{
    pn_object *return_value = NULL;
    PN_ASSERT(node != NULL);

    //pn_debug("node type = %d\tsibling = %p\n", node->node_type, node->sibling);

    // execute a statement for the type
    if (node->node_type && node->node_type < NODE_MAX_COUNT)
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
 * evaluate codes
 */
pn_object *Eval_ExecuteTree(pn_world *world, pn_node *tree, bool trace)
{
    pn_node *cur = tree;
    pn_object *result = NULL;
    while (cur != NULL) {
        result = execute_one_step(world, cur);

        if (trace)
            print_value(world, result);

        cur = cur->sibling;
    }
    return result;
}

