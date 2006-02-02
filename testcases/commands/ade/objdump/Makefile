CFLAGS += -w -o
LDFLAGS +=

SRCS=$(wildcard *.c)
TESTS= test test_arch.o test_D

all: $(TESTS)

install:
	ln -f objdump01 ../../../bin
	ln -f test_arch.o ../../../bin
	ln -f test ../../../bin
	ln -f test_D ../../../bin


clean:
	rm -f $(TESTS)

$(TESTS): 
	$(CC) -O $(CFLAGS) $(cflags) test $(LDFLAGS) ./test.c
	$(CC) -c $(CFLAGS) $(cflags) test_arch.o ./test.c
	$(CC) $(CFLAGS) $(cflags) test_D $(LDFLAGS) ./test.c
