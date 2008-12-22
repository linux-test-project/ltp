# Check that the definitions below are correct for your system

CFLAGS   +=	-I../../../../../include -Wall
LOADLIBES+=	-L../../../../../lib -lltp
TARGETS  := ht_enabled

all: $(TARGETS)

ht_enabled:
	gcc -o ht_enabled HTenabled.c HTutils.c $(CFLAGS) $(LOADLIBES)

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)


