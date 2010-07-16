#ifndef PEANUT_LIST_H
#define PEANUT_LIST_H

// 이름은 리스트지만, 실제로는 동적 배열이다.
// C++의 std::vector 와 같은 역할을 한다.

#include <stdbool.h>
#include <sys/types.h>

typedef struct _list
{
	void **data;
	size_t size;
	size_t reserved_size;
} list;

typedef void (*item_deleter)(void *);


list *List_Create();									// 새 리스트를 생성한다.
void List_Destroy(list *l, item_deleter deleter);		// 리스트를 제거한다.
void List_Clear(list *l, item_deleter deleter);			// 리스트를 비운다.
bool List_IsEmpty(list *l);								// 비어있는지 확인한다.

size_t List_Size(list *l);								// 리스트의 크기를 리턴한다.
void List_Reserve(list *l, size_t size);				// 해당 메모리 사이즈를 미리 할당한다.
void List_Resize(list *l, size_t size);					// 배열 사이즈를 해당 사이즈로 맞춘다.
void List_Compress(list *l);							// 2의 지수승으로 컴팩트하게 줄인다.

void *List_Get(list *l, int which);						// 해당 인덱스 값을 가져옴.
void *List_Put(list *l, int which, void *data);			// 해당 인덱스 값을 업데이트 한다. 기존 값을 리턴한다.
void List_AppendItem(list *l, void *data);				// 리스트의 맨 마지막에 추가한다.
void List_AppendList(list *l, list *other, bool remove_other);				// a_list 에다가, other를 추가한다.
void List_Remove(list *l, int which, size_t size, item_deleter deleter);	// 특정 위치에서, 사이즈 개수만큼을 제거하고, 뒷 부분을 땡긴다.
void List_InsertItem(list *l, int which, void *item);						// 특정 위치에 값을 삽입한다.
void List_InsertList(list *l, int which, list *other, bool remove_other);	// 특정 위치에 다른 리스트를 삽입한다.

#endif//PEANUT_LIST_H
