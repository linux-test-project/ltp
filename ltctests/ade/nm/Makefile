CFLAGS += -O 
LDFLAGS += 

SRCS=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SRCS))

all:  $(OBJECTS)

install:
	cp nm01 ../../bin

clean:
	rm -f nmfile*.o 


