LIBS= -ldl
CC=gcc
SRCS= example.c
OBJECTS=$(SRCS:.c=.o)
CFLAGS=-O9 -rdynamic

all: example shared.so wetest
	@bsign=`which bsign`; \
	if test -n $$bsign; then \
		echo "running bsign"; \
		$$bsign -s -P "--homedir=../twiddlebit" wetest shared.so example; \
	else \
		echo "No bsign found"; \
	fi;

example: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)

shared.so: shared.o
	ld -shared shared.o -o shared.so

clean:
	rm -f *.o core y.tab.h lang.c example *.so weoutput wetest wetest2

spotless: clean
	rm -f example *.so
