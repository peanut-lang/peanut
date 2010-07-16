#include <stdlib.h>
#include "list.h"

typedef struct _TestItem
{
	int value;
} TestItem;

TestItem *newItem(int v)
{
	TestItem *item = (TestItem *)pn_alloc(sizeof(TestItem));
	item->value = v;
	return item;
}

void print_list(list *L)
{
	size_t size = List_Size(L);
	size_t i;
	
	printf("size = %d\n", (int)size);
	
	for (i = 0; i < size; i++)
	{
		TestItem *item = (TestItem *)List_Get(L, i);
		printf("[%d/%d] => %d\n", (int)i, (int)size, item->value);
		//printf("[%d/%d] => %p\n", (int)i, (int)size, item);
	}
}

void test1() {
	list *L = List_Create();
	size_t size = List_Size(L);
	printf("size = %d\n", (int)size);
	
	List_AppendItem(L, newItem(1));
	List_AppendItem(L, newItem(2));
	List_AppendItem(L, newItem(3));
	List_AppendItem(L, newItem(4));
	printf("< list L >\n");
	print_list(L);
	
	list *L2 = List_Create();
	List_AppendItem(L2, newItem(5));
	List_AppendItem(L2, newItem(6));
	List_AppendItem(L2, newItem(7));
	List_AppendItem(L2, newItem(8));
	printf("< list L2 >\n");
	print_list(L2);
	
	// remove test
	printf("< list L(1,2) >\n");
	List_Remove(L, 1, 2, NULL);
	print_list(L);
	
	// append test
	printf("< append test >\n");
	List_AppendList(L, L2, false);
	print_list(L);
	
	// insert test
	printf("< insert test >\n");
	List_Insert(L, 3, L2, false);
	print_list(L);
	
	List_Destroy(L, NULL);
	List_Destroy(L2, NULL);
}

void test2() {
	int i;
	list *L = List_Create();
	
	for (i = 0; i < 500; i++)
	{
		List_AppendItem(L, newItem(i));
		//TestItem *item = (TestItem *)List_Get(L, i);
		//printf("appended. %d: %d, size=%d, reserved=%d\n", i, item->value, (int)List_Size(L), (int)L->reserved_size);
	}
	
	print_list(L);
	printf("\nreserved = %d\n", (int)L->reserved_size);
	
	List_Destroy(L, NULL);
}

int main() {
	//test1();
	test2();
	
	return 0;
}




