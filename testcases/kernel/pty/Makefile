
CFLAGS+= -O -Wall -g -D_GNU_SOURCE -I../../../include
LDFLAGS+= -L../../../lib -lltp

BIN= hangup01 ptem01 pty01

all: $(BIN)

clean:	FRC
	rm -f $(BIN)

install:  FRC
	@set -e; for i in $(BIN); do ln -f $$i ../../bin/$$i ; done

hangup01: hangup01.c
	$(CC) $(CFLAGS) -o hangup01 hangup01.c $(LDFLAGS) 

ptem01: ptem01.c
	$(CC) $(CFLAGS) -o ptem01 ptem01.c $(LDFLAGS)

pty01: pty01.c	
	$(CC) $(CFLAGS) -o pty01 pty01.c $(LDFLAGS)

FRC:
