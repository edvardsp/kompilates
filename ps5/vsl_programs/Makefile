ifdef VERBOSE
	Q =
	E = @true
else
	Q = @
	E = @echo
endif

TARGETS := $(shell ls *.vsl | sed s/\.vsl/\.out/g)

CC  := cc
ASM := as
IR  := ../bin/vslc.out

all: ${TARGETS}

%.out: %.vsl
	${E} 'Compiling target $@'
	${Q} ${IR} < $< > $(patsubst %.vsl, %.s, $<)
	${Q} ${ASM} -o $(patsubst %.vsl, %.o, $<) $(patsubst %.vsl, %.s, $<)
	${Q} ${CC} -o $@ $(patsubst %.vsl, %.o, $<)
	${Q} -rm -f $(patsubst %.vsl, %.s, $<) $(patsubst %.vsl, %.o, $<)

clean:
	${E} 'Removing compiled targets'
	${Q} -rm -f ${TARGETS}

.PHONY: all clean