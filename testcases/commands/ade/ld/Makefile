CFLAGS += -O 
LDFLAGS += 

SRCS=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SRCS))

all:  $(OBJECTS)

install: all
	ln -f ld01 ../../../bin

clean:
	rm -f main.o d1.o f1.o rd1.o rf1.o ldmain.o lola a.out


