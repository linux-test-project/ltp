TARGETS=fs_perms
all: fs_perms testx.file
fs_perms: fs_perms.o
	$(CC) fs_perms.o -o fs_perms
testx.file: testx.o
	$(CC) testx.o -o testx.file
install:
	@for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done
fs_perms.o: fs_perms.c
	$(CC) -c fs_perms.c
testx.o: testx.c
	$(CC) -c testx.c
clean:
	rm -f *.o fs_perms testx.file
