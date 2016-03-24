TARGETS := $(shell ls *.vsl | sed s/\.vsl/\.table/g)

TARGET ?=

all: ${TARGETS}

%.table: %.vsl
	../bin/vslc.out <$*.vsl > $*.table

clean:
	-rm -f *.table

test:
	diff ${TARGET}.table ${TARGET}.table.correct | colordiff

test_all:
	$(foreach var, $(TARGETS), diff $(var) $(var).correct | colordiff;)