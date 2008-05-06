CFLAGS += -Wall
CPPFLAGS += -I../../../../include
LDLIBS += -lltp
LDFLAGS += -L../../../../lib

TARGETS = fs_perms

all: fs_perms testx

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f *.o fs_perms testx
