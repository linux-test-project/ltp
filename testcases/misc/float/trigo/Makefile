
CFLAGS = -lm -lpthread
BIN = gensin gencos gentan genatan genatan2 genacos genasin gentrigo
DATA = dtan rtan datan ratan dsin rsin datan2 ratan2 dcos rcos dasin rasin dacos racos

all:
	gcc $(CFLAGS) gensin.c -o gensin
	gcc $(CFLAGS) gencos.c -o gencos
	gcc $(CFLAGS) gentan.c -o gentan
	gcc $(CFLAGS) genatan.c -o genatan
	gcc $(CFLAGS) genatan2.c -o genatan2
	gcc $(CFLAGS) genasin.c -o genasin
	gcc $(CFLAGS) genacos.c -o genacos
	gcc $(CFLAGS) gentrigo.c -o gentrigo
	./gentrigo

clean:
	rm -f $(BIN)
	@for i in $(BIN); do rm -f ../../../bin/$$i ; done
	rm -f $(DATA)
	@for i in $(DATA); do rm -f ../../../bin/$$i ; done

install:
	sleep 10
	@for i in $(DATA); do ln -f $$i ../../../bin/$$i ; done ;
