CFLAGS+= -Wall -O -D_LINUX
CFILES=pthcli.c pthserv.c readline.c writen.c
OFILES=$(CFILES:.c=.o)
LOADLIBES+= -lpthread
TESTS = pthcli pthserv

all: $(TESTS)

pthcli: pthcli.o readline.o writen.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LOADLIBES)

pthserv: pthserv.o readline.o writen.o 
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LOADLIBES)

install:
	@set -e; for i in $(TESTS) ; do ln -f $$i ../../../bin/$$i ; done ;
	ln -f run_sched_cliserv.sh ../../../bin/
	ln -f data ../../../bin/

clean:
	rm -fr $(OFILES) $(TESTS)
