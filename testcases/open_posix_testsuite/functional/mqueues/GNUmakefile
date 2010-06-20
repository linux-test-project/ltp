INCLUDE = -I../../include
LIB=-lrt -lpthread

CFLAGS=-Wall -O2 -g

all: send_rev_1.test send_rev_2.test 

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LIB)   
clean: 
	rm *.test

 
