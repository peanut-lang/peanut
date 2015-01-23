#include "strtable.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

StrTable *StrTable_Create()
{
    StrTable *t = malloc(sizeof(StrTable));
    if (t != 0)
        t->len = 0;
    return t;
}

int StrTable_Destroy(StrTable *t)
{
    if (t == 0)
        return -EFAULT;

    free(t);
    return 0;
}

int StrTable_Put(StrTable *t, const char *str, bool duplicate)
{
    int i;

    if (t == 0)
        return -EFAULT;

    if (t->len + 1 >= MAX_STRTABLE_SIZE)
        return -ENOSPC;

    // if exists
    for (i = 0; i < t->len; i++) {
        if (strcmp(t->s[i], str) == 0)
            return i;
    }

    // or Create new one if not exists
    t->s[t->len] = duplicate ? strdup(str) : str;
    return t->len++;
}

const char *StrTable_Get(StrTable *t, int index)
{
    if (t)
        return t->s[index];
    else
        return 0;
}

int StrTable_GetError(StrTable *t, int index)
{
    if (t == 0)
        return -EFAULT;
    else if (index < 0 || index >= MAX_STRTABLE_SIZE)
        return -EINVAL;
    else if (t->s[index] == 0)
        return -ENOENT;
    else
        return 0;
}
