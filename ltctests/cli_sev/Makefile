CFLAGS+= -W -O -D_LINUX
CFILES=pthcli.c pthserv.c readline.c writen.c
OFILES=$(CFILES:.c=.o)
TESTS = pthcli pthserv

all: $(OFILES) $(TESTS)

pthcli:
	$(CC) $(CFLAGS) -opthcli pthcli.o readline.o writen.o -lpthread
pthserv:
	$(CC) $(CFLAGS) -opthserv pthserv.o readline.o writen.o -lpthread

install: all
	cp -a $(TESTS) ../bin

clean:
	rm -fr $(OFILES) $(TESTS)
