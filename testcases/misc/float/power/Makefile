
LOADLIBES += -lm
BIN = gensqrt genpow genceil genfabs genfloor genfmod genpower
REF = 1fmod_inp.ref  ceil_inp.ref  fabs_inp.ref  floor_inp.ref  fmod_inp.ref  pow_inp.ref  sqrt_inp.ref 1pow_inp.ref   ceil_out.ref  fabs_out.ref  floor_out.ref  fmod_out.ref  pow_out.ref  sqrt_out.ref

all:	$(BIN)
	./genpower

clean:
	rm -f $(BIN)
	@for i in $(BIN); do rm -f ../../../bin/$$i ; done
	rm -f *.ref
	@for i in $(REF); do rm -f ../../../bin/$$i ; done

install:
	@for i in $(REF) ; do ln -f $$i ../../../bin/$$i ; done ;
