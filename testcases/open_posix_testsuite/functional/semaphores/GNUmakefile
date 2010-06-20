
INCLUDE = -I../../include
LIB=

CFLAGS=-Wall -O2 -g 

all:  make-test
make-test: sem_lock.test sem_conpro.test sem_readerwriter.test sem_philosopher.test sem_sleepingbarber.test

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LIB) -lpthread  
clean: 
	rm *.test

 
