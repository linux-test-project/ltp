
CFLAGS = -lm -lpthread
BIN = genexp genlog genlog10 genfrexp genldexp genhypot genmodf genexp_log
REF = exp_inp.ref frexp_inp.ref hypot_out.ref ldexp_out.ref log_inp.ref modf_out.ref exp_out.ref frexp_out.ref ildexp_inp.ref log10_inp.ref modf1_out.ref frexp1_out.ref hypot_inp.ref ldexp_inp.ref log10_out.ref modf_inp.ref
REF2 = log_out.ref2

all:
	$(CC) $(CFLAGS) genexp.c -o genexp
	$(CC) $(CFLAGS) genlog.c -o genlog
	$(CC) $(CFLAGS) genlog10.c -o genlog10
	$(CC) $(CFLAGS) genfrexp.c -o genfrexp
	$(CC) $(CFLAGS) genldexp.c -o genldexp
	$(CC) $(CFLAGS) genmodf.c -o genmodf
	$(CC) $(CFLAGS) genhypot.c -o genhypot
	$(CC) $(CFLAGS) genexp_log.c -o genexp_log
	./genexp_log

clean:
	rm -f $(BIN)
	@for i in $(BIN); do rm -f ../../../bin/$$i ; done
	rm -f *.ref
	@for i in $(REF); do rm -f ../../../bin/$$i ; done
	rm -f *.ref2
	@for i in $(REF2); do rm -f ../../../bin/$$i ; done

install:
	sleep 60
	@for i in $(REF) $(REF2); do ln -f $$i ../../../bin/$$i ; done ;
