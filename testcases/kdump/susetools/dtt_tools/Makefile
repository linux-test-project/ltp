CFLAGS = -Wall

all: ttutils
	make -C ./helper all

ttutils: ttutils.o

ttutils.o: ttutils.c

clean:
	rm -f ttutils ttutils.o
	make -C ./helper clean
