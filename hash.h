#ifndef PEANUT_HASH_H
#define PEANUT_HASH_H

#include "globals.h"
#include <stdbool.h>

typedef struct _hash_entry {
    char *k;
    void *v;
    unsigned int h;
    struct _hash_entry *next;
} hash_entry;

typedef struct _hash {
    unsigned int tablelength;
    hash_entry **table;
    unsigned int entrycount;
    unsigned int loadlimit;
    unsigned int primeindex;
    //unsigned int (*hashfn) (void *k);
    int (*eqfn) (const char *k1, const char *k2);
} hash;

hash *Hash_Create();
void *Hash_Put(hash *h, const char *k, void *v);
void *Hash_Get(hash *h, const char *k);
void *Hash_Remove(hash *h, const char *k);
unsigned int Hash_Count(hash *h);
void Hash_Destroy(hash *h, bool free_values);

typedef struct _hash_itr {
    hash *h;
    hash_entry *e;
    hash_entry *parent;
    unsigned int index;
} hash_itr;

hash_itr *Hash_Iterator(hash *h);

char *Hash_Iterator_Key(hash_itr *i);
void *Hash_Iterator_Value(hash_itr *i);

int Hash_Iterator_Advance(hash_itr *itr);
int Hash_Iterator_Remove(hash_itr *itr);
int Hash_Iterator_Search(hash_itr *itr, hash *h, const char *k);

#endif//PEANUT_HASH_H

