CFLAGS += -w -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS) file1.o

install:
	ln -f ar01 ../../../bin
	ln -f file*.in ../../../bin
	ln -f file1.o ../../../bin

$(TARGETS):
	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGETS) file1.o


