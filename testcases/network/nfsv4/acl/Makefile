#
#       @(#)Makefile	ACL Basic tests
#
# to make tests, use 'make'

ifeq ($(shell echo | $(CC) $(CFLAGS) -E -include sys/acl.h - &>/dev/null && echo yes),yes)
ACLTESTS := acl1 
else
ACLTESTS :=
endif

#include ../tests.init
LIBS := -lacl

all: $(ACLTESTS)
	@chmod a+x runtest

acl1: acl1.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o acl1 acl1.c $(LIBS)

install: $(ACLTESTS)
	@set -e; for i in $(ACLTESTS); do ln -f $$i ../../../bin ; done

clean:
	rm -f $(ACLTESTS)
