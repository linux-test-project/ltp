LIBS= -ldl
CC=gcc

all: shared.so libwritetest
	@bsign=`which bsign`; \
	if test -n $$bsign; then \
		echo "running bsign"; \
		$$bsign -s -P "--homedir=../twiddlebit" libwritetest shared.so; \
		cp shared.so shared.so.signed; \
	else \
		echo "No bsign found"; \
	fi;

libwritetest: libwritetest.c
	$(CC) -o $@ libwritetest.c $(LIBS)

shared.so: shared.o
	ld -shared shared.o -o shared.so

clean:
	rm -f *.o core *.so weoutput libwritetest shared.so.signed execlog writelog
