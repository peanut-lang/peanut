# Makefile of peanut

LEX= flex
LFLAGS=

YACC= bison
YFLAGS= -v -d

CC= gcc
CFLAGS= -g -Wall -std=gnu99

LIBS= -lm

.SUFFIXES: .c .o

BIN=peanut

OBJS= \
	lex.yy.o \
	peanut.tab.o \
	world.o \
	eval.o \
	hash.o \
	list.o \
	stack.o \
	strtable.o \
	pnobject.o \
	pninteger.o \
	pnreal.o \
	pnstring.o \
	pnbool.o \
	pnfunction.o \
	pnnull.o \
	pnlist.o \
	pnhash.o \
	pnstdio.o

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

all: $(BIN)

lex.yy.o: lex.yy.c peanut.tab.h peanut.tab.o
	$(CC) $(CFLAGS) -o $@ -c lex.yy.c

lex.yy.c: lex.l
	$(LEX) $(LFLAGS) -o $@ $<

peanut.tab.h: peanut.y

peanut.tab.c: peanut.y
	$(YACC) $(YFLAGS) -o $@ $<

peanut.tab.o: peanut.tab.c
	$(CC) $(CFLAGS) -o $@ -c $<

.c.o:
	$(CC) $(CFLAGS) -c $<

test: $(BIN)
	./$(BIN) -t tests/example1.pn tests/example2.pn < tests/input.txt

clean:
	rm -f $(BIN) peanut.tab.c peanut.tab.h peanut.output lex.yy.c test_* $(OBJS)
