# Check that the definitions below are correct for your system

CFLAGS    += -I../../../../../include -Wall
LOADLIBES += -L../../../../../lib -lltp
TARGETS   := ht_affinity

all: $(TARGETS)

#ht_affinity: HTaffinity.o HTutils.o
#	gcc -o ht_affinity HTaffinity.o HTutils.o $(CFLAGS) $(LOADLIBES)
ht_affinity:
	gcc -o ht_affinity HTaffinity.c HTutils.c $(CFLAGS) $(LOADLIBES)

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)
