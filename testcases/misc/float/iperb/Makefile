
CFLAGS = -lm -lpthread
BIN = gensinh gencosh gentanh geniperb
DATA = dsinh rsinh dcosh rcosh dtanh rtanh

all:
	$(CC) $(CFLAGS) gensinh.c -o gensinh
	$(CC) $(CFLAGS) gencosh.c -o gencosh
	$(CC) $(CFLAGS) gentanh.c -o gentanh
	$(CC) $(CFLAGS) geniperb.c -o geniperb
	./geniperb

clean:
	rm -f $(BIN)
	@for i in $(BIN); do rm -f ../../../bin/$$i ; done
	rm -f $(DATA)
	@for i in $(DATA); do rm -f ../../../bin/$$i ; done

install:
	sleep 10
	@for i in $(DATA); do ln -f $$i ../../../bin/$$i ; done ;
