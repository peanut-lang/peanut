#ifndef _PEANUT_STRTABLE_H_
#define _PEANUT_STRTABLE_H_

#include <stdbool.h>
#include <sys/types.h>

#define MAX_STRTABLE_SIZE 1000

// string table
typedef struct _strtab{
    const char *s[MAX_STRTABLE_SIZE];
    int len;
} strtab;

strtab *strtab_create();
int strtab_destroy(strtab *t);
int strtab_put(strtab *t, const char *str, bool duplicate);
const char *strtab_get(strtab *t, int index);
int strtab_errno(strtab *t, int index);

#endif//_PEANUT_STRTABLE_H_
