
CFLAGS = -lm -lpthread
BIN = gensin gencos gentan genatan genatan2 genacos genasin gentrigo
DATA = dtan rtan datan ratan dsin rsin datan2 ratan2 dcos rcos dasin rasin dacos racos

all:
	$(CC) $(CFLAGS) gensin.c -o gensin
	$(CC) $(CFLAGS) gencos.c -o gencos
	$(CC) $(CFLAGS) gentan.c -o gentan
	$(CC) $(CFLAGS) genatan.c -o genatan
	$(CC) $(CFLAGS) genatan2.c -o genatan2
	$(CC) $(CFLAGS) genasin.c -o genasin
	$(CC) $(CFLAGS) genacos.c -o genacos
	$(CC) $(CFLAGS) gentrigo.c -o gentrigo
	./gentrigo

clean:
	rm -f $(BIN)
	@for i in $(BIN); do rm -f ../../../bin/$$i ; done
	rm -f $(DATA)
	@for i in $(DATA); do rm -f ../../../bin/$$i ; done

install:
	sleep 10
	@for i in $(DATA); do ln -f $$i ../../../bin/$$i ; done ;
