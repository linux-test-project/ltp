
LOADLIBES += -lm
BIN = genj0 genj1 geny0 geny1 genlgamma genbessel
REF = gamma_inp.ref  gamma_out.ref  gamma_sign.ref  j0_inp.ref  j1_inp.ref  y0_inp.ref  y1_inp.ref
REF2 = j0_out.ref2  j1_out.ref2  y0_out.ref2  y1_out.ref2

all:	$(BIN)
	./genbessel

clean:
	rm -f $(BIN) 
	@for i in $(BIN); do rm -f ../../../../bin/$$i ; done
	rm -f *.ref
	@for i in $(REF); do rm -f ../../../../bin/$$i ; done
	rm -f *.ref2 
	@for i in $(REF2); do rm -f ../../../../bin/$$i ; done

install:
	@for i in $(REF) $(REF2); do ln -f $$i ../../../../bin/$$i ; done ;
