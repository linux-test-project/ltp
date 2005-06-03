ifndef POSIX_DIR_LIB
	POSIX_DIR_LIB=/usr/lib
endif

INCLUDE = -I../../include
LIB=-lposix1b

CC=gcc
CFLAGS=-Wall -O2 -g -I$(POSIX_DIR_INC) -L$(POSIX_DIR_LIB)

all:  multi_con_pro.test

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LIB) -lpthread  
clean: 
	rm *.test

 
