INCLUDE = -I../../include
LIB=-lrt

CFLAGS=-Wall -O2 -g

all: multi_send_rev_1.test multi_send_rev_2.test

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LIB)   
clean: 
	rm *.test

 
