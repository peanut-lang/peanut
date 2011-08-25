# Makefile of peanut

LEX= flex
LFLAGS=

YACC= bison
YFLAGS= -v -d

CC= gcc
CFLAGS= -g -Wall

LIBS= -lfl

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
	pn_integer.o \
	pn_real.o \
	pn_string.o \
	pn_bool.o \
	pn_object.o \
	pn_function.o \
	pn_null.o \
	pn_list.o \
	pn_hash.o \
	pn_stdio.o

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -lm -o $@ $^ $(LIBS)

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
	./$(BIN) -t example1.pn example2.pn < input.txt

clean:
	rm -f $(BIN) peanut.tab.c peanut.tab.h peanut.output lex.yy.c $(OBJS)
