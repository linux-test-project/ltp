
LOADLIBES += -lm
BIN = gensinh gencosh gentanh geniperb
DATA = dsinh rsinh dcosh rcosh dtanh rtanh

all:	$(BIN)
	./geniperb

clean:
	rm -f $(BIN)
	@for i in $(BIN); do rm -f ../../../../bin/$$i ; done
	rm -f $(DATA)
	@for i in $(DATA); do rm -f ../../../../bin/$$i ; done

install:
	@for i in $(DATA); do ln -f $$i ../../../../bin/$$i ; done ;
