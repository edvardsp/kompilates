ifdef VERBOSE
	Q =
	E = @true
else
	Q = @
	E = @echo
endif

# Compilers to use
CC   := clang
LEX  := flex
YACC := bison

# Different flags
YFLAGS  += --defines=inc/y.tab.h -o y.tab.c
CFLAGS  += --std=c11 -g -Iinc -D_POSIX_C_SOURCE=200809L -DYYSTYPE="pNode"
LDFLAGS +=

# Some specific flags, separated for explicitnes
OPT := -O0

ifeq (${CC}, clang)
WARN := -Wall -Wextra -Werror -Weverything \
	-Wno-padded -Wno-switch-enum -Wno-unused-macros \
	-Wno-unused-function
else
WARN := -Wall -Wextra -Werror \
	-Wno-unused-but-set-parameter
endif

# Given target, as well as object-files,
# must filter out O_GEN to avoid double inclusion
TARGET  := bin/vslc.out
O_GEN   := obj/parser.o obj/scanner.o
O_FILES := $(filter-out $(O_GEN), $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)))

# Default rule
all: ${TARGET}

# Linking of target
${TARGET}: ${O_GEN} ${O_FILES}
	${E} ${CC} : Linking target $@
	${Q} ${CC} -o $@ ${O_GEN} ${O_FILES} ${LDFLAGS}

# Flex and yacc specific rules,
# obj/*.o is seperate because omitting warnings
src/y.tab.h: src/parser.c
src/scanner.c: inc/y.tab.h src/scanner.l
obj/scanner.o: src/scanner.c deps/scanner.d
	${E} '${CC} : Compiling $@'
	${Q} ${CC} -o $@ ${CFLAGS} ${OPT} -c $<
obj/parser.o: src/parser.c deps/parser.d
	${E} '${CC} : Compiling $@'
	${Q} ${CC} -o $@ ${CFLAGS} ${OPT} -c $<

# General compiling
obj/%.o: src/%.c deps/%.d
	${E} '${CC} : Compiling $@'
	${Q} ${CC} -o $@ ${CFLAGS} ${WARN} ${OPT} -c $<

# Dependency generation
deps/%.d: src/%.c
	${Q} ${CC} ${CFLAGS} -MM -MT $(patsubst src/%.c, deps/%.d, $<) $< -MF $@

# Include dependencies
DEPS := $(patsubst obj/%.o, deps/%.d, $(O_FILES) $(O_GEN))
-include ${DEPS}

# Run tests
test: ${TARGET}
	${E} 'Running testcases, compiling VSL programs'
	${Q} -cd vsl_programs/ && ${MAKE} clean && ${MAKE} all

# Clean and purge
clean:
	${E} 'Removing obj, deps, ${TARGET}'
	${Q} -rm -f ${O_GEN} ${O_FILES} ${TARGET} ${DEPS}

purge: clean
	${E} 'Removing generated flex/yacc files'
	${Q} -rm -f src/parser.c src/scanner.c inc/y.tab.h
	${E} 'Removing generated tests'
	${Q} -cd vsl_programs/ && ${MAKE} clean

# Dont treat rules as files
.PHONY: all test clean purge