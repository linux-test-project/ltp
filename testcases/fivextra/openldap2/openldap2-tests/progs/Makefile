## Copyright 1998-2002 The OpenLDAP Foundation, All Rights Reserved.
## COPYING RESTRICTIONS APPLY, see COPYRIGHT file
##
## tests Makefile.in for OpenLDAP
#
# modified for standalone use - Robb Romans <robb@austin.ibm.com>

EXE  = slapd-tester slapd-search slapd-read slapd-addel
SRCS = slapd-tester.c slapd-search.c slapd-read.c slapd-addel.c
LIBS = -lldap -llber
TARGETDIR = ../../../../bin/openldap2-tests

all: #$(EXE)

slapd-tester: slapd-tester.o 
	gcc -o $@ slapd-tester.o $(LIBS)

slapd-search: slapd-search.o
	gcc -o $@ slapd-search.o $(LIBS)

slapd-read: slapd-read.o
	gcc -o $@ slapd-read.o $(LIBS)

slapd-addel: slapd-addel.o
	gcc -o $@ slapd-addel.o $(LIBS)

clean:
	@rm -f *.o *.obj *~ core $(EXE)

install: all
#	mkdir -p $(TARGETDIR) || break
#	cp -a $(EXE) $(TARGETDIR) || break

uninstall:
#	for i in $(EXE); do \
		rm -f $(TARGETDIR)/$$i; \
	done

