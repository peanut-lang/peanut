#include "hash.h"
#include "globals.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const unsigned int primes[] = {
	53, 97, 193, 389,
	769, 1543, 3079, 6151,
	12289, 24593, 49157, 98317,
	196613, 393241, 786433, 1572869,
	3145739, 6291469, 12582917, 25165843,
	50331653, 100663319, 201326611, 402653189,
	805306457, 1610612741
};

const unsigned int prime_table_length = sizeof(primes) / sizeof(primes[0]);
const float max_load_factor = 0.65;

hash *
Hash_Create() {
	hash *h;
	int minsize = 160;
    unsigned int pindex, size = primes[0];
    /* Check requested hashtable isn't too large */
    if (minsize > (1u << 30)) return NULL;
    /* Enforce size as prime */
    for (pindex=0; pindex < prime_table_length; pindex++) {
        if (primes[pindex] > minsize) { size = primes[pindex]; break; }
    }
    h = (hash *)pn_alloc(sizeof(hash));
    if (NULL == h) return NULL; /*oom*/
    h->table = (hash_entry **)pn_alloc(sizeof(hash_entry *) * size);
    if (NULL == h->table) { free(h); return NULL; } /*oom*/
    memset(h->table, 0, size *sizeof(hash_entry *));
    h->tablelength  = size;
    h->primeindex   = pindex;
    h->entrycount   = 0;
    h->eqfn         = strcmp;
    h->loadlimit    = (unsigned int) ceil(size *max_load_factor);
    return h;
}

static unsigned int
strToHash(const char *k) {
	int i;
	unsigned int hResult = 0;
	int len = strlen(k);
	for ( i=0; i < len; i++) {
		hResult = 31 * hResult + k[i];
	}
	return hResult;
}

static unsigned int
toHash(hash *h, const char *k) {
    /* Aim to protect against poor hash functions by adding logic here
     * - logic taken from java 1.4 hashtable source */
    unsigned int i = strToHash(k);
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); /* >>> */
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); /* >>> */
    return i;
}

static inline unsigned int
indexFor(unsigned int tablelength, unsigned int hashvalue) {
    return (hashvalue % tablelength);
}

static int
hashtable_expand(hash *h) {
	PN_ASSERT(h != NULL);
	
    /* Double the size of the table to accomodate more entries */
    hash_entry **newtable;
    hash_entry *e;
    hash_entry **pE;
    unsigned int newsize, i, index;
    /* Check we're not hitting max capacity */
    if (h->primeindex == (prime_table_length - 1)) return 0;
    newsize = primes[++(h->primeindex)];

    newtable = (hash_entry **)pn_alloc(sizeof(hash_entry*) * newsize);
    if (NULL != newtable)
    {
        memset(newtable, 0, newsize *sizeof(hash_entry *));
        /* This algorithm is not 'stable'. ie. it reverses the list
         * when it transfers entries between the tables */
        for (i = 0; i < h->tablelength; i++) {
            while (NULL != (e = h->table[i])) {
                h->table[i] = e->next;
                index = indexFor(newsize,e->h);
                e->next = newtable[index];
                newtable[index] = e;
            }
        }
        free(h->table);
        h->table = newtable;
    }
    /* Plan B: realloc instead */
    else 
    {
        newtable = (hash_entry **)
                   realloc(h->table, newsize *sizeof(hash_entry *));
        if (NULL == newtable) { (h->primeindex)--; return 0; }
        h->table = newtable;
        memset(newtable[h->tablelength], 0, newsize - h->tablelength);
        for (i = 0; i < h->tablelength; i++) {
            for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
                index = indexFor(newsize,e->h);
                if (index == i)
                {
                    pE = &(e->next);
                }
                else
                {
                    *pE = e->next;
                    e->next = newtable[index];
                    newtable[index] = e;
                }
            }
        }
    }
    h->tablelength = newsize;
    h->loadlimit   = (unsigned int) ceil(newsize *max_load_factor);
    return -1;
}

static int
__Hash_Put(hash *h, const char *k, void *v) {
	PN_ASSERT(h != NULL);
	
	// FIXME: duplicate key를 ASSERT로 막아놈
	PN_ASSERT(Hash_Get(h, k) == NULL);
	
    /* This method allows duplicate keys - but they shouldn't be used */
    unsigned int index;
    hash_entry *e;
    if (++(h->entrycount) > h->loadlimit)
    {
        /* Ignore the return value. If expand fails, we should
         * still try cramming just this value into the existing table
         * -- we may not have memory for a larger table, but one more
         * element may be ok. Next time we insert, we'll try expanding again.*/
        hashtable_expand(h);
    }
    e = (hash_entry *)pn_alloc(sizeof(hash_entry));
    if (NULL == e) { --(h->entrycount); return 0; } /*oom*/
    e->h = toHash(h,k);
    index = indexFor(h->tablelength,e->h);
    e->k = strdup((char *) k);
    e->v = v;
    e->next = h->table[index];
    h->table[index] = e;
    return -1;
}

void *
Hash_Put(hash *h, const char *k, void *v) {
	PN_ASSERT(h != NULL);
	void *old = NULL;
	
	// FIXME 여기에서, 해쉬에서 Remove 를 하면 안될듯????
	if (Hash_Get(h, k) != NULL)
		old = Hash_Remove(h, k);
	
	if (__Hash_Put(h, k, v) == 0)
		PN_FAIL("__Hash_Put() failed.");
	
	return old;
}

void * /* returns value associated with key */
Hash_Get(hash *h, const char *k) {
	PN_ASSERT(h != NULL);
	
    hash_entry *e;
    unsigned int hashvalue, index;
    hashvalue = toHash(h,k);
    index = indexFor(h->tablelength, hashvalue);
    e = h->table[index];
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == e->h) && (h->eqfn(k, e->k) == 0)) return e->v;
        e = e->next;
    }
    return NULL;
}

void * /* returns value associated with key */
Hash_Remove(hash *h, const char *k) {
	PN_ASSERT(h != NULL);
	
    /* TODO: consider compacting the table when the load factor drops enough,
     *       or provide a 'compact' method. */

    hash_entry *e;
    hash_entry **pE;
    void *v;
    unsigned int hashvalue, index;

    hashvalue = toHash(h,k);
    index = indexFor(h->tablelength, toHash(h,k));
    pE = &(h->table[index]);
    e = *pE;
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == e->h) && (h->eqfn(k, e->k) == 0))
        {
            *pE = e->next;
            h->entrycount--;
            v = e->v;
            free(e->k);
            free(e);
            return v;
        }
        pE = &(e->next);
        e = e->next;
    }
    return NULL;
}

unsigned int
Hash_Count(hash *h) {
	PN_ASSERT(h != NULL);
    return h->entrycount;
}
	
void
Hash_Destroy(hash *h, bool free_values) {
	PN_ASSERT(h != NULL);
	
    unsigned int i;
    hash_entry *e, *f;
    hash_entry **table = h->table;
    if (free_values)
    {
        for (i = 0; i < h->tablelength; i++)
        {
            e = table[i];
            while (NULL != e)
            { f = e; e = e->next; free(f->k); free(f->v); free(f); }
        }
    }
    else
    {
        for (i = 0; i < h->tablelength; i++)
        {
            e = table[i];
            while (NULL != e)
            { f = e; e = e->next; free(f->k); free(f); }
        }
    }
    free(h->table);
    free(h);
}

/***************************************************************************************/

hash_itr *
Hash_Iterator(hash *h) {
	PN_ASSERT(h != NULL);
	
    unsigned int i, tablelength;
    hash_itr *itr = (hash_itr *)pn_alloc(sizeof(hash_itr));
    if (NULL == itr) return NULL;
    itr->h = h;
    itr->e = NULL;
    itr->parent = NULL;
    tablelength = h->tablelength;
    itr->index = tablelength;
    if (0 == h->entrycount) return itr;

    for (i = 0; i < tablelength; i++)
    {
        if (NULL != h->table[i])
        {
            itr->e = h->table[i];
            itr->index = i;
            break;
        }
    }
    return itr;
}

/*****************************************************************************/
/* key      - return the key of the (key,value) pair at the current position */
/* value    - return the value of the (key,value) pair at the current position */

char *
Hash_Iterator_Key(hash_itr *i) {
	PN_ASSERT(i != NULL);
	if (i->e != NULL)
		return i->e->k;
	else
		return NULL;
}

void *
Hash_Iterator_Value(hash_itr *i) {
	PN_ASSERT(i != NULL);
	if (i->e != NULL)
		return i->e->v;
	else
		return NULL;
}

/*****************************************************************************/
/* advance - advance the iterator to the next element
 *           returns zero if advanced to end of table */

int
Hash_Iterator_Advance(hash_itr *itr) {
	PN_ASSERT(itr != NULL);
	unsigned int j,tablelength;
    hash_entry **table;
    hash_entry *next;
    if (NULL == itr->e) return 0; /* stupidity check */

    next = itr->e->next;
    if (NULL != next)
    {
        itr->parent = itr->e;
        itr->e = next;
        return -1;
    }
    tablelength = itr->h->tablelength;
    itr->parent = NULL;
    if (tablelength <= (j = ++(itr->index)))
    {
        itr->e = NULL;
        return 0;
    }
    table = itr->h->table;
    while (NULL == (next = table[j]))
    {
        if (++j >= tablelength)
        {
            itr->index = tablelength;
            itr->e = NULL;
            return 0;
        }
    }
    itr->index = j;
    itr->e = next;
    return -1;
}

/*****************************************************************************/
/* remove - remove the entry at the current iterator position
 *          and advance the iterator, if there is a successive
 *          element.
 *          If you want the value, read it before you remove:
 *          beware memory leaks if you don't.
 *          Returns zero if end of iteration. */

int
Hash_Iterator_Remove(hash_itr *itr) {
	PN_ASSERT(itr != NULL);
    hash_entry *remember_e, *remember_parent;
    int ret;

    /* Do the removal */
    if (NULL == (itr->parent))
    {
        /* element is head of a chain */
        itr->h->table[itr->index] = itr->e->next;
    } else {
        /* element is mid-chain */
        itr->parent->next = itr->e->next;
    }
    /* itr->e is now outside the hashtable */
    remember_e = itr->e;
    itr->h->entrycount--;
    free(remember_e->k);

    /* Advance the iterator, correcting the parent */
    remember_parent = itr->parent;
    ret = Hash_Iterator_Advance(itr);
    if (itr->parent == remember_e) { itr->parent = remember_parent; }
    free(remember_e);
    return ret;
}

/*****************************************************************************/
int /* returns zero if not found */
Hash_Iterator_Search(hash_itr *itr, hash *h, const char *k) {
	PN_ASSERT(itr != NULL);
	PN_ASSERT(h != NULL);
    hash_entry *e, *parent;
    unsigned int hashvalue, index;

    hashvalue = toHash(h,k);
    index = indexFor(h->tablelength,hashvalue);

    e = h->table[index];
    parent = NULL;
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == e->h) && (h->eqfn(k, e->k) == 0))
        {
            itr->index = index;
            itr->e = e;
            itr->parent = parent;
            itr->h = h;
            return -1;
        }
        parent = e;
        e = e->next;
    }
    return 0;
}

