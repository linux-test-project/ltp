
CFLAGS = -lm -lpthread
BIN = gensqrt genpow genceil genfabs genfloor genfmod genpower
REF = 1fmod_inp.ref  ceil_inp.ref  fabs_inp.ref  floor_inp.ref  fmod_inp.ref  pow_inp.ref  sqrt_inp.ref 1pow_inp.ref   ceil_out.ref  fabs_out.ref  floor_out.ref  fmod_out.ref  pow_out.ref  sqrt_out.ref

all:
	gcc $(CFLAGS) gensqrt.c -o gensqrt
	gcc $(CFLAGS) genpow.c -o genpow
	gcc $(CFLAGS) genceil.c -o genceil
	gcc $(CFLAGS) genfabs.c -o genfabs
	gcc $(CFLAGS) genfloor.c -o genfloor
	gcc $(CFLAGS) genfmod.c -o genfmod
	gcc $(CFLAGS) genpower.c -o genpower
	./genpower

clean:
	rm -f $(BIN)
	@for i in $(BIN); do rm -f ../../../bin/$$i ; done
	rm -f *.ref
	@for i in $(REF); do rm -f ../../../bin/$$i ; done

install: all
	sleep 1
	@for i in $(REF) ; do ln -f $$i ../../../bin/$$i ; done ;
