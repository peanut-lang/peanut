#include "globals.h"
#include "list.h"
#include <stdlib.h>

#define MIN_POW2 5
#define MAX_POW2 20
#define MIN_LIST_SIZE (2 << MIN_POW2)
#define MAX_LIST_SIZE (2 << MAX_POW2)
#define TOO_LARGE ((size_t)0xffffffff)

#define MIN(a, b) ((a)<(b)?(a):(b))

static void expand_if_need(list *l, int count)
{
    if (l->size + count > l->reserved_size)
        List_Reserve(l, l->size + count);
}

static void delete_all_items(list *l, size_t start, size_t end, item_deleter deleter)
{
    if (deleter != NULL) {
        size_t i;
        for (i = (start); i < (end); i++)
            deleter(l->data[i]);
    }
}

/**
 * smallest 2's pow, but, greater than size
 */
static size_t make_pow2(size_t size)
{
    int i;
    for (i = MIN_POW2; i <= MAX_POW2; i++)
        if ((size_t)(2 << i) > size)
            return 2 << i;

    return TOO_LARGE;
}

/**
 * alloc 2's pow size of `size`
 */
static void **alloc_data(size_t size, size_t *allocated_size)
{
    size = make_pow2(size);
    if (allocated_size != NULL)
        *allocated_size = size;

    if (size == TOO_LARGE)
        return NULL;
    else
        return (void **)malloc(sizeof(void*) * size);
}

/**
 * same with memcpy()
 */
static void *list_memcpy(void *dest, void const *src, size_t len)
{
    char *d = dest;
    char const *s = src;

    while (len-- > 0)
        *d++ = *s++;

    return dest;
}

/*
 * same with memset()
 */
static void *list_memset(void *dest, int c, size_t len)
{
    char *d = dest;
    while (len-- > 0)
        *d++ = c;
    return dest;
}

/**
 * create new list
 */
list *List_Create()
{
    list *l = (list *)malloc(sizeof(list));
    l->data = alloc_data(MIN_LIST_SIZE, NULL);
    l->size = 0;
    l->reserved_size = MIN_LIST_SIZE;
    PN_ASSERT(l->data != NULL);
    return l;
}

/**
 * destroy the list
 */
void List_Destroy(list *l, item_deleter deleter)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);
    delete_all_items(l, 0, l->size, deleter);
    free(l->data);
    free(l);
}

/**
 * clear the list
 */
void
List_Clear(list *l, item_deleter deleter)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);

    delete_all_items(l, 0, l->size, deleter);
    l->size = 0;
}

/**
 * check is it empty
 */
bool List_IsEmpty(list *l)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);
    return l->size == 0;
}

/**
 * get size of the list
 */
size_t List_Size(list *l)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);
    return l->size;
}

/**
 * reserve size of the list
 */
void List_Reserve(list *l, size_t size)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);

    // need not new alloc
    if (size <= l->reserved_size)
        return;

    // replace new bigger list
    size_t allocated_size = 0;
    void **new_data = alloc_data(size, &allocated_size);
    list_memcpy(new_data, l->data, sizeof(void *) * l->size);
    free(l->data);
    l->data = new_data;
    l->reserved_size = allocated_size;
}

/**
 * resize the list
 */
void
List_Resize(list *l, size_t size)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);

    if (size == l->size) {
        return;
    } else if (size < l->size) {
        l->size = size;
    } else {// size > l->size
        size_t start = l->size;
        size_t len = size - l->size;
        List_Reserve(l, size);
        list_memset(&l->data[start], 0, len);
    }
}

/**
 * resize to smallest 2's pow size
 */
void List_Compact(list *l)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);

    size_t compact_size = make_pow2(l->size);

    if (compact_size == l->reserved_size)
        return;

    // need replace to smaller list
    size_t allocated_size = 0;
    void **new_data = alloc_data(compact_size, &allocated_size);
    list_memcpy(new_data, l->data, l->size);
    free(l->data);
    l->data = new_data;
    l->reserved_size = allocated_size;
}

/**
 * get index of the list
 */
void *List_Get(list *l, int which)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);
    PN_ASSERT(0 <= which && which < l->size);
    return l->data[(size_t)which];
}

/**
 * update the index. it'll return old value.
 */
void *List_Put(list *l, int which, void *data)
{
    void *old = List_Get(l, which);
    l->data[which] = data;
    return old;
}

/**
 * append the item to the list
 */
void List_AppendItem(list *l, void *data)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);

    expand_if_need(l, 1);
    l->data[l->size++] = data;
}

/**
 * append the other the list
 */
void List_AppendList(list *l, list *other, bool remove_other)
{
    List_InsertList(l, l->size, other, remove_other);
}

/**
 * remove a item from the list
 */
void List_Remove(list *l, int which, size_t size, item_deleter deleter)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(l->data != NULL);
    PN_ASSERT(which <= l->size);
    PN_ASSERT(which + size <= l->size);

    // if need delete, delete items
    delete_all_items(l, which, which + size, deleter);

    // move to
    size_t i;
    size_t last = MIN((size_t)which + size, l->size);

    for (i = (size_t)which; i < last; i++)
        l->data[i] = l->data[i + size];

    // update size
    l->size = l->size - size;
}

/**
 * insert a item to the list
 */
void List_InsertItem(list *l, int which, void *item)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(0 <= which && which < l->size);

    expand_if_need(l, 1);

    int i, size = l->size + 1;
    for (i = size - 1; i > which - 1; i--)
        l->data[i] = l->data[i - 1];

    l->data[which] = item;
}


/**
 * insert a list to the list
 */
void List_InsertList(list *l, int which, list *other, bool remove_other)
{
    PN_ASSERT(l != NULL);
    PN_ASSERT(other != NULL);

    int i;
    size_t new_size = l->size + other->size;

    expand_if_need(l, other->size);

    // copy last first. because it must not overlap
    for (i = new_size - 1; i >= which; i--)
        l->data[i] = l->data[i - which];

    // copy other, from which
    int j = 0;
    for (i = which; i < which + other->size; i++)
        l->data[i] = other->data[j++];

    // update size
    l->size = l->size + other->size;

    // if need remove other, remove it
    if (remove_other)
        List_Destroy(other, NULL);
}

