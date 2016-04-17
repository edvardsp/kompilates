ifdef VERBOSE
	Q =
	E = @true
else
	Q = @
	E = @echo
endif

TARGETS := $(shell ls *.vsl | sed s/\.vsl/\.out/g)

CC  := gcc
IR  := ../bin/vslc.out

all: ${TARGETS}

%.out: %.vsl
	${E} 'Compiling target $@'
	${Q} ${IR} < $< > $(patsubst %.vsl, %.s, $<)
	${Q} ${CC} -o $@ $(patsubst %.vsl, %.s, $<)

clean:
	${E} 'Removing compiled targets'
	${Q} -rm -f ${TARGETS}

.PHONY: all clean