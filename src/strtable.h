#ifndef _PEANUT_STRTABLE_H_
#define _PEANUT_STRTABLE_H_

#include <stdbool.h>
#include <sys/types.h>

#define MAX_STRTABLE_SIZE 1000

// string table
typedef struct _StrTable{
    const char *s[MAX_STRTABLE_SIZE];
    int len;
} StrTable;

StrTable *StrTable_Create();
int StrTable_Destory(StrTable *t);
int StrTable_Put(StrTable *t, const char *str, bool duplicate);
const char *StrTable_Get(StrTable *t, int index);
int StrTable_GetErrno(StrTable *t, int index);

#endif//_PEANUT_STRTABLE_H_
