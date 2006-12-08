# Check that the definitions below are correct for your system

CFLAGS    += -I../../../../../include -Wall
LDLFLAGS  += -L../../../../../lib
LDLIBS    += -lltp
TARGETS   := ht_affinity

all: $(TARGETS)

ht_affinity: HTaffinity.o HTutils.o

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS) *.o
