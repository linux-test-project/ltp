CFLAGS += -Wall
CPPFLAGS += -I../../../../include
LDLIBS += -lltp
LDFLAGS += -L../../../../lib

TARGETS = fs_perms

all: fs_perms testx

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done
	cp fs_perms_simpletest.sh ../../../bin/

clean:
	rm -f *.o fs_perms testx
