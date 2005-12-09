#
#       @(#)Makefile	ACL Basic tests
#
# to make tests, use 'make'

#include ../tests.init
CFLAGS=-g
LIBS = -lacl
ACLTESTS=acl1 

all:
	@make $(ACLTESTS)
	if test ! -x runtest; then chmod a+x runtest; fi

# Basic acl tests
acl1:	acl1.c
	$(CC) $(CFLAGS) -o acl1 acl1.c $(LIBS)

stress:	stress.c
	$(CC) $(CFLAGS) -o stress stress.c $(LIBS)

clean: acl1
	rm -f acl1

