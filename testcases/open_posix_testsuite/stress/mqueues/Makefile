INCLUDE = -I../../include
LIB=-lmqueue

CFLAGS=-Wall -O2 -g -lpthread

all: multi_send_rev_1.test multi_send_rev_2.test

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LIB)   
clean: 
	rm *.test

 
