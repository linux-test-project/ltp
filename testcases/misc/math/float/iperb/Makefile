CFLAGS += -Wall
LOADLIBES = -lm

BIN = gensinh gencosh gentanh geniperb
DATA = dsinh rsinh dcosh rcosh dtanh rtanh

all:	$(BIN)


clean:
	rm -f $(BIN)
	@set -e; for i in $(BIN); do rm -f ../../../../bin/$$i ; done
	rm -f $(DATA)
	@set -e; for i in $(DATA); do rm -f ../../../../bin/$$i ; done

install:
	@set -e; for i in $(BIN); do ln -f $$i ../../../../bin/$$i ; done
