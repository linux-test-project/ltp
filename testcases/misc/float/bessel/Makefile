
CFLAGS = -lm -lpthread
BIN = genj0 genj1 geny0 geny1 genlgamma genbessel
REF = gamma_inp.ref  gamma_out.ref  gamma_sign.ref  j0_inp.ref  j1_inp.ref  y0_inp.ref  y1_inp.ref
REF2 = j0_out.ref2  j1_out.ref2  y0_out.ref2  y1_out.ref2

all:
	gcc $(CFLAGS) genj0.c -o genj0
	gcc $(CFLAGS) genj1.c -o genj1
	gcc $(CFLAGS) geny0.c -o geny0
	gcc $(CFLAGS) geny1.c -o geny1
	gcc $(CFLAGS) genlgamma.c -o genlgamma
	gcc $(CFLAGS) genbessel.c -o genbessel
	./genbessel

clean:
	rm -f $(BIN) 
	@for i in $(BIN); do rm -f ../../../bin/$$i ; done
	rm -f *.ref
	@for i in $(REF); do rm -f ../../../bin/$$i ; done
	rm -f *.ref2 
	@for i in $(REF2); do rm -f ../../../bin/$$i ; done

install: all 
	sleep 10
	@for i in $(REF) $(REF2); do ln -f $$i ../../../bin/$$i ; done ;
