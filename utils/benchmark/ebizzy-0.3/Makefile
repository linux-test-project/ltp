all: ebizzy

configure:
	./configure
ebizzy: configure
	gcc -Wall -Wshadow -lpthread  -o ebizzy ebizzy.c
install:
	
clean:
	rm -f ebizzy *~
