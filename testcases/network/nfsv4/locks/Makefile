all: locktests

locktests: locktests.c
	$(CC) -Wall $(CFLAGS) $(LDFLAGS) $^ -o $@ -lpthread

install:

clean:
	rm -f locktests; rm -f test*; rm -f locktests.c~
