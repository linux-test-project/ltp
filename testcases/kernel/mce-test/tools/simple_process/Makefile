CFLAGS := -g -Wall

all: simple_process

install:
	cp simple_process ../../bin

simple_process: simple_process.o

simple_process.o: simple_process.c

clean:
	rm -f simple_process *.o
