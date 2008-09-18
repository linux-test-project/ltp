INCLUDE = -I../../include
LIB=-lposix1b

CFLAGS=-Wall -O2 -g

all:  multi_con_pro.test

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LIB) -lpthread  
clean: 
	rm *.test

 
