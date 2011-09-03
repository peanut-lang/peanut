#include "strtab.h"
#include "pnerrno.h"
#include <stdlib.h>
#include <string.h>

strtab *strtab_create()
{
    strtab *t = malloc(sizeof(strtab));
    if (t != 0)
        t->len = 0;
    return t;
}

int strtab_destroy(strtab *t)
{
    if (t == 0)
        return -EPFAULT;

    free(t);
    return 0;
}

int strtab_put(strtab *t, const char *str, bool duplicate)
{
    int i;

    if (t == 0)
        return -EPFAULT;

    if (t->len + 1 >= MAX_STRTABLE_SIZE)
        return -EPNOSPC;

    // if exists
    for (i = 0; i < t->len; i++) {
        if (strcmp(t->s[i], str) == 0)
            return i;
    }

    // or create new one if not exists
    t->s[t->len] = duplicate ? strdup(str) : str;
    return t->len++;
}

const char *strtab_get(strtab *t, int index)
{
    if (t)
        return t->s[index];
    else
        return 0;
}

int strtab_errno(strtab *t, int index)
{
    if (t == 0)
        return -EPFAULT;
    else if (index < 0 || index >= MAX_STRTABLE_SIZE)
        return -EPINVAL;
    else if (t->s[index] == 0)
        return -EPNOENT;
    else
        return 0;
}
