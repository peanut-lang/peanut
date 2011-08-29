#ifndef PEANUT_LIST_H
#define PEANUT_LIST_H

#include <stdbool.h>
#include <sys/types.h>

typedef struct _list
{
    void **data;
    size_t size;
    size_t reserved_size;
} list;

typedef void (*item_deleter)(void *);


list *List_Create();
void List_Destroy(list *l, item_deleter deleter);
void List_Clear(list *l, item_deleter deleter);
bool List_IsEmpty(list *l);

size_t List_Size(list *l);
void List_Reserve(list *l, size_t size);
void List_Resize(list *l, size_t size);
void List_Compress(list *l);

void *List_Get(list *l, int which);
void *List_Put(list *l, int which, void *data);
void List_AppendItem(list *l, void *data);
void List_AppendList(list *l, list *other, bool remove_other);
void List_Remove(list *l, int which, size_t size, item_deleter deleter);
void List_InsertItem(list *l, int which, void *item);
void List_InsertList(list *l, int which, list *other, bool remove_other);

#endif//PEANUT_LIST_H
