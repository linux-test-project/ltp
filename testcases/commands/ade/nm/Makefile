CFLAGS += -O -Wall
LOADLIBES += 

SRCS=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SRCS))

all:  $(OBJECTS)

install:
	ln -f nm01 ../../../bin
	ln -f nmfile*.c ../../../bin
	ln -f *.o ../../../bin

clean:
	rm -f nmfile*.o 


