#
# Softdog v1.21
# (C) Christophe Dupre 1998

BINNAME	= softdog
OPTIMIZE	= -O2 -fomit-frame-pointer -fno-strength-reduce
CFLAGS	= -Wall $(OPTIMIZE)

all: softdog

softdog: softdog.o
	$(CC) $(CFLAGS) -o $(BINNAME) $<
	# $(STRIP) $(BINNAME)

install: 
	ln -f softdog.sh ../../bin/ || break

uninstall: 
	@rm -f ../../bin/softdog.sh

clean:
	@rm -f *.[oas] *~ core $(BINNAME)

