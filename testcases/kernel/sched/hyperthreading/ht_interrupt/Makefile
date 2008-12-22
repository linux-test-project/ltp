# Check that the definitions below are correct for your system

CFLAGS+=	-I../../../../../include -Wall
LOADLIBES+=	-L../../../../../lib -lltp
TARGETS = ht_interrupt

all: $(TARGETS)

ht_interrupt:
	gcc -o ht_interrupt HTinterrupt.c HTutils.c $(CFLAGS) $(LOADLIBES) 

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)


