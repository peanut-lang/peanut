# Makefile of peanut
#

LEX= flex
LFLAGS=

YACC= bison
YFLAGS= -v -d

CC= gcc
#CC= /Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/arm-apple-darwin10-gcc-4.2.1
#CC= /Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/gcc-4.2
CFLAGS= -g -Wall -DSTANDALONE
#CFLAGS= -miphoneos-version-min=4.0 -std=gnu99 -arch armv6  -pipe -no-cpp-precomp --sysroot='/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.0.sdk' -isystem /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.0.sdk/usr/include -DSTDC_HEADERS

LIBS= -lfl

peanut: lex.yy.o peanut.tab.o world.o hash.o list.o stack.o pn_integer.o pn_real.o pn_string.o pn_bool.o utils.o eval.o pn_object.o pn_function.o pn_null.o pn_list.o pn_hash.o pn_stdio.c pn_stdio.h
	$(CC) $(CFLAGS) -lm -o $@ $^ $(LIBS)

libpeanut.a: lex.yy.o peanut.tab.o world.o hash.o list.o stack.o pn_integer.o pn_real.o pn_string.o pn_bool.o utils.o eval.o pn_object.o pn_function.o pn_null.o pn_list.o pn_hash.o pn_stdio.c pn_stdio.h
	$(AR) rcs $@ $^

lex.yy.o: lex.yy.c peanut.tab.h peanut.tab.o
	$(CC) $(CFLAGS) -o $@ -c lex.yy.c 

peanut.tab.h: peanut.y

peanut.tab.c: peanut.y
	$(YACC) $(YFLAGS) -o $@ $<

peanut.tab.o: peanut.tab.c
	$(CC) $(CFLAGS) -o $@ -c $<

lex.yy.c: lex.l
	$(LEX) $(LFLAGS) -o $@ $<

world.o: world.c
	$(CC) $(CFLAGS) -o $@ -c $<

world.c: world.h globals.h

stack.o: stack.c
	$(CC) $(CFLAGS) -o $@ -c $<

stack.c: stack.h globals.h

list.o: list.c
	$(CC) $(CFLAGS) -o $@ -c $<

list.c: list.h globals.h

hash.o: hash.c
	$(CC) $(CFLAGS) -o $@ -c $<

hash.c: hash.h globals.h


pn_integer.o: pn_integer.c
	$(CC) $(CFLAGS) -o $@ -c $<

pn_integer.c: pn_integer.h globals.h

pn_real.o: pn_real.c pn_real.h globals.h
	$(CC) $(CFLAGS) -o $@ -c $<

pn_string.o: pn_string.c pn_string.h globals.h
	$(CC) $(CFLAGS) -o $@ -c $<

utils.o: utils.c utils.h globals.h
	$(CC) $(CFLAGS) -o $@ -c $<

eval.o: eval.c eval.h globals.h
	$(CC) $(CFLAGS) -o $@ -c $<

pn_object.o: pn_object.c pn_object.h
	$(CC) $(CFLAGS) -o $@ -c $<

pn_function.o: pn_function.c pn_function.h
	$(CC) $(CFLAGS) -o $@ -c $<

pn_null.o: pn_null.c pn_null.h globals.h pn_object.h
	$(CC) $(CFLAGS) -o $@ -c $<

pn_list.o: pn_list.c
	$(CC) $(CFLAGS) -o $@ -c $<

pn_hash.o: pn_hash.c
	$(CC) $(CFLAGS) -o $@ -c $<

pn_bool.o: pn_bool.c
	$(CC) $(CFLAGS) -o $@ -c $<

list_test: list.o list_test.c
	$(CC) $(CFLAGS) -lm -o list_test $^

hash_test: hash.o hash_test.c
	$(CC) $(CFLAGS) -lm -o hash_test $^
	./hash_test

world_test: world.o stack.o hash.o list.o pn_integer.o pn_real.o pn_string.o pn_hash.o pn_list.o pn_bool.o pn_object.o pn_function.o utils.o pn_null.o eval.o pn_stdio.o world_test.c
	$(CC) $(CFLAGS) -lm -o world_test $^
	./world_test

clean:
	rm -f peanut peanut.exe peanut.tab.c peanut.tab.h peanut.output lex.yy.c *.o
	rm -f hash_test world_test list_test

all: peanut

multi_test: peanut
	./peanut --multi-test

eval_test: peanut
	./peanut --eval-test -t example.pn example2.pn example2.pn example2.pn example2.pn < input.txt

switch_test: peanut
	./peanut -t -i example.pn example2.pn example2.pn example2.pn example2.pn < input.txt

test: peanut
	./peanut -t example.pn example2.pn example2.pn example2.pn example2.pn < input.txt
#./peanut -t example.pn < input.txt

