INCLUDE = -I../../include

CFLAGS=-Wall -O2 -g

all:  multi_con_pro.test

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ -lpthread
clean: 
	rm *.test

 
