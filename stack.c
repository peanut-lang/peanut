#include "stack.h"
#include "globals.h"
#include <stdlib.h>


stack *
Stack_Create()
{
    stack *s = (stack *)pn_alloc(sizeof(stack));
    PN_ASSERT(s != NULL);
    s->data = List_Create();
    return s;
}

void
Stack_Destroy(stack *s)
{
    PN_ASSERT(s != NULL);
    List_Destroy(s->data, NULL);
    free(s);
}

void
Stack_Push(stack *s, void *value)
{
    PN_ASSERT(s != NULL);
    List_AppendItem(s->data, value);
}

void *
Stack_Pop(stack *s)
{
    PN_ASSERT(s != NULL);
    int last_index = List_Size(s->data) - 1;
    PN_ASSERT(last_index >= 0);
    void *data = List_Get(s->data, last_index);
    List_Remove(s->data, last_index, 1, NULL);
    return data;
}

void *
Stack_Top(stack *s)
{
    PN_ASSERT(s != NULL);
    int last_index = List_Size(s->data) - 1;
    if (last_index >= 0)
        return List_Get(s->data, last_index);
    else
        return NULL;
}

bool
Stack_IsEmpty(stack *s)
{
    PN_ASSERT(s != NULL);
    return List_IsEmpty(s->data);
}

void *
Stack_Get(stack *s, int which)
{
    PN_ASSERT(s != NULL);
    int size = List_Size(s->data);
    if (0 <= which && which < size)
        return List_Get(s->data, which);
    else
        return NULL;
}

int Stack_Size(stack *s)
{
    PN_ASSERT(s != NULL);
    return List_Size(s->data);
}
