#ifndef _PEANUT_API_H_
#define _PEANUT_API_H_

#include "globals.h"
#include "world.h"
#include <stdbool.h>

pn_world *Peanut_CreateWorld();
void Peanut_DestroyWorld(pn_world *world);

pn_object *Peanut_EvalFromFile(char *filename, pn_world *world, bool trace);
pn_object *Peanut_EvalFromString(char *code, pn_world *world, bool trace);

#endif//_PEANUT_API_H_
