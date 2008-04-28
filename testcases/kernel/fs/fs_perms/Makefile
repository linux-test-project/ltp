CFLAGS += -Wall

TARGETS = fs_perms

all: fs_perms testx.file

fs_perms: fs_perms.o

testx.file: testx.o
	$(CC) testx.o -o testx.file $(CFLAGS) $(LDFLAGS)

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f *.o fs_perms testx.file
