TARGETS := locktests

all:
	gcc -Wall locktests.c -o locktests -lpthread

install: $(TARGETS) 	 
	ln -f locktests ../../../bin/locktests 	 
	  	 
clean: 	 
	$(RM) locktests
