CFLAGS+= -Wall -O -D_LINUX
CFILES=pthcli.c pthserv.c readline.c writen.c
OFILES=$(CFILES:.c=.o)
LOADLIBES+= -lpthread
TESTS = pthcli pthserv

all: $(OFILES) $(TESTS)

pthcli:
	$(CC) $(CFLAGS) $(LDFLAGS) -opthcli pthcli.o readline.o writen.o $(LOADLIBES)
pthserv:
	$(CC) $(CFLAGS) $(LDFLAGS) -opthserv pthserv.o readline.o writen.o $(LOADLIBES)

install:
	@set -e; for i in $(TESTS) ; do ln -f $$i ../../../bin/$$i ; done ;

clean:
	rm -fr $(OFILES) $(TESTS)
