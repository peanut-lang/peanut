#include "hash.h"
#include <stdlib.h>
#include <string.h>

typedef struct _test_struct {
	int val;
	char *name;
} test_struct;

int
main(int argc, char **argv) {
	test_struct *test = pn_alloc(sizeof(test_struct));
	test->val = 3;
	test->name = "huhuhuhuhu";

	printf("# Hash Create\n");
	hash *h = Hash_Create();
	printf("# Hash Put\n");
	Hash_Put(h, "test1", test);
	Hash_Put(h, "test2", "jweb2");
	Hash_Put(h, "test3", "jweb3");
	Hash_Put(h, "test4", "jweb4");
	printf("# Hash Get\n");
	char *value;
	test_struct *test2 = Hash_Get(h, "test1");
	if(test2 == NULL) {
		printf("헐 종텐이다.\n");
		return -1;
	}
	printf("-_-: %p\n", test);
	printf("-_-: %p\n", test2);

	printf("Get Result : %s\n", test2->name);
	value = Hash_Get(h, "test3");
	printf("Get Result : %s\n", value);
	value = Hash_Get(h, "test2");
	printf("Get Result : %s\n", value);
	value = Hash_Get(h, "test4");
	printf("Get Result : %s\n", value);
	value = Hash_Get(h, "test5");
	printf("Get Result : %s\n", value);
	printf("# Hash Destroy\n");
	Hash_Destroy(h, false);
	printf("# Test End\n");
	return 0;
}

