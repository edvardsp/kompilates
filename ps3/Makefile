CC=clang
LEX=flex
YACC=bison
YFLAGS+=--defines=src/y.tab.h -o y.tab.c
CFLAGS+=-std=c11 -O3 -g -Isrc -Iinclude -D_POSIX_C_SOURCE=200809L -DYYSTYPE="pNode"
LDLIBS+=-lc

CFLAGS+=-Wall -Wextra -Weverything

src/vslc: src/vslc.c src/parser.o src/scanner.o src/nodetypes.o src/tree.o src/stack.o
src/y.tab.h: src/parser.c
src/scanner.c: src/y.tab.h src/scanner.l
clean:
	-rm -f src/parser.c src/scanner.c src/*.tab.* src/*.o
purge: clean
	-rm -f src/vslc
