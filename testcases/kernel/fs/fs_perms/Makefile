TARGETS=fs_perms
all: fs_perms.o testx.o
	gcc fs_perms.o -o fs_perms
	gcc testx.o -o testx.file
install:
	@for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done
fs_perms.o: fs_perms.c
	gcc -c fs_perms.c
testx.o: testx.c
	gcc -c testx.c
clean:
	rm -f *.o fs_perms testx.file
