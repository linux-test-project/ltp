TARGETS := locktests

all: $(TARGETS)

locktests: locktests.c netsync.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lpthread

install: $(TARGETS)
	ln -f locktests ../../../bin/locktests

clean:
	$(RM) locktests
