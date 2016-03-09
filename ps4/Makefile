ifdef VERBOSE
	Q = 
	E = @true 
else
	Q = @
	E = @echo 
endif

# Compilers to use
CC=clang
LEX=flex
YACC=bison

# Different flags
YFLAGS+=--defines=inc/y.tab.h -o y.tab.c
CFLAGS+=--std=c11 -g -Iinc -D_POSIX_C_SOURCE=200809L -DYYSTYPE="pNode"
LDFLAGS+=

# Some specific flags, separated for explicitnes
OPT=-O0
WARN=-Wall -Wextra -Weverything -Werror \
	 -Wno-padded -Wno-switch-enum -Wno-unused-macros

# Given target, as well as object-files,
# must filter out O_GEN or suffer double inclusion 
TARGET=bin/vslc.out
O_GEN=obj/parser.o obj/scanner.o
O_FILES=$(filter-out $(O_GEN), $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)))

# Default rule
all: ${TARGET}

# Linking of target
${TARGET}: ${O_GEN} ${O_FILES}
	${E}${CC} : Linking target $@
	${Q}${CC} -o $@ ${O_GEN} ${O_FILES} ${LDFLAGS}

# Flex and yacc specific rules, 
# obj/* is seperate because warnings is in the way |
src/y.tab.h: src/parser.c
src/scanner.c: inc/y.tab.h src/scanner.l
obj/scanner.o: src/scanner.c
	${E}${CC} : Compiling $@ 
	${Q}${CC} -o $@ -MD -MP ${CFLAGS} ${OPT} -c $<
obj/parser.o: src/parser.c 
	${E}${CC} : Compiling $@
	${Q}${CC} -o $@ -MD -MP ${CFLAGS} ${OPT} -c $<

# General compiling rule, 
# -MD -MP is for dependency
obj/%.o: src/%.c
	${E}${CC} : Compiling $@
	${Q}${CC} -o $@ -MD -MP ${CFLAGS} ${WARN} ${OPT} -c $<

# Dependency between files
ALL_DEPS=$(patsubst obj/%.o, obj/%.d, $(O_GEN) $(O_FILES))
-include $(ALL_DEPS)

# Run tests
test:
	@{E}Running testcases

# Clean and purge
clean:
	${E}Removing *.o, *.d, ${TARGET}
	${Q}-rm -f ${O_GEN} ${O_FILES} ${TARGET} ${ALL_DEPS} 

purge: clean
	${E}Removing generated flex/yacc files
	${Q}-rm -f src/parser.c src/scanner.c inc/y.tab.h

# Dont treat rules as files
.PHONY: all test clean purge