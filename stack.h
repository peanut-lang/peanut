#ifndef _PEANUT_STACK_H_
#define _PEANUT_STACK_H_

#include <stdbool.h>
#include "globals.h"
#include "list.h"

typedef struct _stack {
    list *data;
} stack;

stack *Stack_Create();
void Stack_Destroy(stack *s);

void Stack_Push(stack *s, void *value);
void *Stack_Pop(stack *s);
void *Stack_Top(stack *s);
bool Stack_IsEmpty(stack *s);

void *Stack_Get(stack *s, int which);
int Stack_Size(stack *s);

#endif//_PEANUT_STACK_H_
