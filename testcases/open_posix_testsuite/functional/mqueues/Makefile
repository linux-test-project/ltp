ifndef POSIX_DIR_LIB
	POSIX_DIR_LIB=/usr/lib
endif

INCLUDE = -I../../include
LIB=-lmqueue -lpthread

CC=gcc
CFLAGS=-Wall -O2 -g -L$(POSIX_DIR_LIB)

all: send_rev_1.test send_rev_2.test 

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LIB)   
clean: 
	rm *.test

 
