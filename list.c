#include "globals.h"
#include "list.h"
#include <stdlib.h>

#define MIN_POW2 5
#define MAX_POW2 20
#define MIN_LIST_SIZE (2 << MIN_POW2)
#define MAX_LIST_SIZE (2 << MAX_POW2)
#define TOO_LARGE ((size_t)0xffffffff)

#define MIN(a, b) ((a)<(b)?(a):(b))

/**
 * reserved 가 모자라면, 늘린다.
 */
static void
expand_if_need(list *l, int count)
{
	if (l->size + count > l->reserved_size)
		List_Reserve(l, l->size + count);
}

static void
delete_all_items(list *l, size_t start, size_t end, item_deleter deleter)
{
	if (deleter != NULL)
	{
		size_t i;
		for (i = (start); i < (end); i++)
			deleter(l->data[i]);
	}
}

/**
 * size 보다 큰, 가장 작은 2의 제곱수를 리턴한다.
 */
static size_t
make_pow2(size_t size)
{
	int i;
	for (i = MIN_POW2; i <= MAX_POW2; i++)
		if ((size_t)(2 << i) > size)
			return 2 << i;
	
	return TOO_LARGE;
}

/**
 * 해당 size 길이의 void* 를 2의 제곱수로 할당한다.
 */
static void **
alloc_data(size_t size, size_t *allocated_size)
{
	size = make_pow2(size);
	if (allocated_size != NULL)
		*allocated_size = size;
	
	if (size == TOO_LARGE)
		return NULL;
	else
		return (void **)pn_alloc(sizeof(void*) * size);
}

/**
 * memcpy() 와 똑같다. -_-;;
 */
static void *
list_memcpy(void *dest, void const *src, size_t len)
{
	char *d = dest;
	char const *s = src;
	
	while (len-- > 0)
		*d++ = *s++;
	
	return dest;
}

/*
 * memset() 과 똑같다. -_-;;
 */
static void *
list_memset(void *dest, int c, size_t len)
{
	char *d = dest;
	while (len-- > 0)
		*d++ = c;
	return dest;
}

/**
 * 새 리스트를 생성한다.
 */
list *
List_Create()
{
	list *l = (list *)pn_alloc(sizeof(list));
	l->data = alloc_data(MIN_LIST_SIZE, NULL);
	l->size = 0;
	l->reserved_size = MIN_LIST_SIZE;
	PN_ASSERT(l->data != NULL);
	return l;
}

/**
 * 리스트를 제거한다.
 */
void
List_Destroy(list *l, item_deleter deleter)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	delete_all_items(l, 0, l->size, deleter);
	free(l->data);
	free(l);
}

/**
 * 리스트를 비운다.
 */
void
List_Clear(list *l, item_deleter deleter)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	
	// 아이템을 지워야 하면, 모두 지우고..
	delete_all_items(l, 0, l->size, deleter);
	
	// 사이즈를 0 으로 한다.
	l->size = 0;
}

/**
 * 비어있는지 확인한다.
 */
bool
List_IsEmpty(list *l)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	return l->size == 0;
}

/**
 * 리스트의 크기를 리턴한다.
 */
size_t
List_Size(list *l)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	return l->size;
}

/**
 * 해당 메모리 사이즈를 미리 할당한다.
 * 사이즈 조정된 리스트 자신을 리턴한다.
 */
void
List_Reserve(list *l, size_t size)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	
	// 현재 메모리가 더 크면, 그냥 냅둔다.
	if (size <= l->reserved_size)
		return;
	
	// 새로 할당하고, 아이템을 복사하고, 교체한다.
	size_t allocated_size = 0;
	void **new_data = alloc_data(size, &allocated_size);
	list_memcpy(new_data, l->data, sizeof(void *) * l->size);
	free(l->data);
	l->data = new_data;
	l->reserved_size = allocated_size;
}

/**
 * 배열 사이즈를 해당 사이즈로 맞춘다.
 */
void
List_Resize(list *l, size_t size)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	
	// 사이즈가 같으면 그냥 냅둔다.
	if (size == l->size)
	{
		return;
	}
	
	// 사이즈를 줄인다. 할당된 공간을 줄이진 않고, 크기만 줄인다.
	else if (size < l->size)
	{
		l->size = size;
	}
	
	// 모자라면, 사이즈를 늘리고, 새로 생긴 칸에 0 으로 채운다.
	else
	{
		size_t start = l->size;
		size_t len = size - l->size;
		List_Reserve(l, size);
		list_memset(&l->data[start], 0, len);
	}
}

/**
 * 2의 지수승으로 컴팩트하게 줄인다.
 */
void
List_Compact(list *l)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	
	// 가장 작은 용량을 구해보고,
	size_t compact_size = make_pow2(l->size);
	
	// 길이가 같으면 냅둔다.
	if (compact_size == l->reserved_size)
		return;
	
	// 새로 할당하고, 아이템을 복사하고, 교체한다.
	size_t allocated_size = 0;
	void **new_data = alloc_data(compact_size, &allocated_size);
	list_memcpy(new_data, l->data, l->size);
	free(l->data);
	l->data = new_data;
	l->reserved_size = allocated_size;
}

/**
 * 해당 인덱스 값을 가져옴.
 */
void *
List_Get(list *l, int which)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	PN_ASSERT(0 <= which && which < l->size);
	return l->data[(size_t)which];
}

/**
 * 해당 인덱스 값을 업데이트 한다. 기존 값을 리턴한다.
 */
void *
List_Put(list *l, int which, void *data)
{
	// assert 는 List_Get 에서 한다.
	void *old = List_Get(l, which);
	l->data[which] = data;
	return old;
}

/**
 * 리스트의 맨 마지막에 추가한다.
 */
void
List_AppendItem(list *l, void *data)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	
	expand_if_need(l, 1);
	l->data[l->size++] = data;
}

/**
 * a_list 에다가, other를 추가한다.
 */
void
List_AppendList(list *l, list *other, bool remove_other)
{
	// assert 는 List_InsertList 에서 한다.
	List_InsertList(l, l->size, other, remove_other);
}

/**
 * 특정 위치에서, 사이즈 개수만큼을 제거하고, 뒷 부분을 땡긴다.
 */
void
List_Remove(list *l, int which, size_t size, item_deleter deleter)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(l->data != NULL);
	PN_ASSERT(which <= l->size);			// 시작 위치가 인덱스 범위 내 인지?
	PN_ASSERT(which + size <= l->size);	// 끝 위치가 인덱스 범위 내 인지?
	
	// 아이템을 지워야 하면, 모두 지우고..
	delete_all_items(l, which, which + size, deleter);
	
	// 뒷 부분부터, 데이터를 복사한다.
	size_t i;
	size_t last = MIN((size_t)which + size, l->size);
	
	for (i = (size_t)which; i < last; i++)
		l->data[i] = l->data[i + size];
	
	// 사이즈를 줄인다.
	l->size = l->size - size;
}

/**
 * 특정 위치에 값을 삽입한다.
 */
void
List_InsertItem(list *l, int which, void *item)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(0 <= which && which < l->size);
	
	expand_if_need(l, 1);
	
	// which 뒤로부터, 한칸씩 뒤로 미루고,
	int i, size = l->size + 1;
	for (i = size - 1; i > which - 1; i--)
	{
		l->data[i] = l->data[i - 1];
	}
	
	l->data[which] = item;
}


/**
 * 특정 위치에 다른 리스트를 삽입한다.
 */
void
List_InsertList(list *l, int which, list *other, bool remove_other)
{
	PN_ASSERT(l != NULL);
	PN_ASSERT(other != NULL);
	
	int i;
	size_t new_size = l->size + other->size;
	
	// 삽입할 크기가 되는지 확인하고, 부족하면 늘린다.
	expand_if_need(l, other->size);
	
	// insert 지점 이후의 부분을 먼저 뒤로 복사하고, (뒷부분 먼저 복사한다. 겹치지 않기 위해서)
	for (i = new_size - 1; i >= which; i--)
		l->data[i] = l->data[i - which];
	
	// other 를 which 부터 복사한다.
	int j = 0;
	for (i = which; i < which + other->size; i++)
		l->data[i] = other->data[j++];
	
	// 사이즈를 바꾼다.
	l->size = l->size + other->size;
	
	// 다른 리스트 자료구조를 지워야 하면.. 지운다. 데이터는 l 에서 쓰므로, 지우면 안된다.
	if (remove_other)
		List_Destroy(other, NULL);
}

