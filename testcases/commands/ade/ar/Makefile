CFLAGS += -w -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS) object

install:
	ln -f ar01 ../../../bin
	ln -f file*.in ../../../bin
	ln -f file1.o ../../../bin


object:
	$(CC) -c file1.c

clean:
	rm -f $(TARGETS) file1.o


