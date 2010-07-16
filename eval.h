#ifndef PEANUT_EVAL_H_
#define PEANUT_EVAL_H_

#include "globals.h"
#include "world.h"

//typedef pn_object *(*FUNCTION)(pn_world *world, pn_object *object, pn_object **params, int length);

pn_object *execute_one_step(pn_world *world, pn_node *node);
pn_object *execute_statements(pn_world *world, pn_node *node);

pn_object *Eval_ExecuteTree(pn_world *world, pn_node *tree, bool repl);

#endif//PEANUT_EVAL_H_

