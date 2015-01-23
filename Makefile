# Makefile of peanut

OS=$(shell uname -s)
ifeq ($(OS), Linux)
DEBUGSYMBOL= -rdynamic
endif
ifeq ($(OS), Darwin)
DEBUGSYMBOL=
endif

.PHONY: directories

BUILD=out

LEX= flex
LFLAGS=

YACC= bison
YFLAGS= -v -d

CC= gcc
CFLAGS= -g $(DEBUGSYMBOL) -Wall -std=gnu99

LIBS= -lm

.SUFFIXES: .c .o

BIN=peanut

dir_guard=@mkdir -p $(BUILD)

OBJS= \
	$(BUILD)/lex.yy.o \
	$(BUILD)/peanut.tab.o \
	$(BUILD)/world.o \
	$(BUILD)/eval.o \
	$(BUILD)/hash.o \
	$(BUILD)/list.o \
	$(BUILD)/stack.o \
	$(BUILD)/strtable.o \
	$(BUILD)/pnobject.o \
	$(BUILD)/pninteger.o \
	$(BUILD)/pnreal.o \
	$(BUILD)/pnstring.o \
	$(BUILD)/pnbool.o \
	$(BUILD)/pnfunction.o \
	$(BUILD)/pnnull.o \
	$(BUILD)/pnlist.o \
	$(BUILD)/pnhash.o \
	$(BUILD)/pnstdio.o


all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/lex.yy.o: $(BUILD)/lex.yy.c $(BUILD)/peanut.tab.h $(BUILD)/peanut.tab.o
	$(dir_guard)
	$(CC) $(CFLAGS) -Isrc -o $@ -c $(BUILD)/lex.yy.c

$(BUILD)/lex.yy.c: src/lex.l
	$(dir_guard)
	$(LEX) $(LFLAGS) -o $@ $<

$(BUILD)/peanut.tab.h: src/peanut.y

$(BUILD)/peanut.tab.c: src/peanut.y
	$(dir_guard)
	$(YACC) $(YFLAGS) -o $@ $<

$(BUILD)/peanut.tab.o: $(BUILD)/peanut.tab.c
	$(dir_guard)
	$(CC) $(CFLAGS) -Isrc -o $@ -c $<

$(BUILD)/%.o: src/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) -Isrc -c $< -o $@

test: $(BIN)
	./$(BIN) -t tests/example1.pn tests/example2.pn < tests/input.txt

clean:
	rm -f $(BIN)
	rm -rf $(BUILD)
