CFLAGS += -Wall
LOADLIBES = -lm

BIN = gensqrt genpow genceil genfabs genfloor genfmod genpower
REF = 1fmod_inp.ref  ceil_inp.ref  fabs_inp.ref  floor_inp.ref  fmod_inp.ref  pow_inp.ref  sqrt_inp.ref 1pow_inp.ref   ceil_out.ref  fabs_out.ref  floor_out.ref  fmod_out.ref  pow_out.ref  sqrt_out.ref

all:	$(BIN)


clean:
	rm -f $(BIN)
	@set -e; for i in $(BIN); do rm -f ../../../../bin/$$i ; done
	rm -f *.ref
	@set -e; for i in $(REF); do rm -f ../../../../bin/$$i ; done

install:
	@set -e; for i in $(BIN); do ln -f $$i ../../../../bin/$$i ; done
