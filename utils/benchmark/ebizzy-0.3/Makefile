all: ebizzy

configure:
	./configure
ebizzy: configure
	gcc -Wall -Wshadow -lpthread  -o ebizzy ebizzy.c
clean:
	rm -f ebizzy *~
