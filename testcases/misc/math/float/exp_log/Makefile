CFLAGS += -Wall
LOADLIBES = -lm

BIN = genexp genlog genlog10 genfrexp genldexp genhypot genmodf genexp_log
REF = exp_inp.ref frexp_inp.ref hypot_out.ref ldexp_out.ref log_inp.ref modf_out.ref exp_out.ref frexp_out.ref ildexp_inp.ref log10_inp.ref modf1_out.ref frexp1_out.ref hypot_inp.ref ldexp_inp.ref log10_out.ref modf_inp.ref
REF2 = log_out.ref2

all:	$(BIN)


clean:
	rm -f $(BIN)
	@set -e; for i in $(BIN); do rm -f ../../../../bin/$$i ; done
	rm -f *.ref
	@set -e; for i in $(REF); do rm -f ../../../../bin/$$i ; done
	rm -f *.ref2
	@set -e; for i in $(REF2); do rm -f ../../../../bin/$$i ; done

install:
	@set -e; for i in $(BIN); do ln -f $$i ../../../../bin/$$i ; done
