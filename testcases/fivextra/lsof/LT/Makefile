CPPFLAGS += -DLT_DIAL_linux
LDFLAGS += LTlib.o

TARGETS = LTbasic LTnlink LTsock LTunix LTdnlc LTlock

all:	LTlib.o $(TARGETS)
	echo $(TARGETS) > ../testlist

install:	all
	for f in $(TARGETS); do ln -f $$f ../../../bin/LT/$$f || break; done

uninstall:
	for f in $(TARGETS); do rm -f ../../../bin/LT/$$f || break; done

%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -rf *~ *.o $(TARGETS)
