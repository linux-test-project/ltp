
ifndef POSIX_DIR_LIB
	POSIX_DIR_LIB=/usr/lib
endif

INCLUDE = -I../../include
LIB=-lmqueue

CC=gcc
CFLAGS=-Wall -O2 -g -I$(POSIX_DIR_INC) -L$(POSIX_DIR_LIB) -lpthread

all: multi_send_rev_1.test multi_send_rev_2.test

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LIB)   
clean: 
	rm *.test

 
