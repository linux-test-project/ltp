#  (C) Copyright IBM Corp. 2003
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
###########################################################################
# name of file	: Makefile						  #
# description	: makefile for openldap2 testcase			  #
# 		Robb Romans <robb@austin.ibm.com>			  #
###########################################################################

TARGETS=`find . -maxdepth 1 -perm +111 -type f`

## Portions Copyright 1998-2002 The OpenLDAP Foundation, All Rights Reserved.
## COPYING RESTRICTIONS APPLY, see COPYRIGHT file

srcdir     = openldap2-tests
SUBDIRS    = $(srcdir) $(srcdir)/progs
TARGETDIR  = ../../bin/$(srcdir)
BUILD_LDBM = yes

all: 		#all-common
install:	#install-common
uninstall:	#clean
clean:		#clean-local
test:		#tests
tests:		#ldbm

#from toplevel makefile
test:
	cd $(srcdir); make test

ldbm: test-ldbm
test-ldbm:
	@ln -s $(srcdir)/data .
	@ln -s $(srcdir)/schema .
	@if test "$(BUILD_LDBM)" = "yes" ; then \
                echo "Initiating LDAP tests for LDBM..." ; \
                @mkdir test-db test-repl || true; \
                $(srcdir)/scripts/all $(srcdir) ldbm ; \
	else \
                echo "run configure with --enable-ldbm" ; \
	fi

passwd: test-passwd
test-passwd:
	@ln -s $(srcdir)/data .
	@ln -s $(srcdir)/schema .
	@echo "Initiating LDAP tests..."
	@mkdir test-db test-repl || true
	$(srcdir)/scripts/passwd-search $(srcdir) passwd

test-nis-schema: test-nis-schema-ldbm
test-nis-schema-ldbm:
	@ln -s $(srcdir)/data .
	@ln -s $(srcdir)/schema .
	@echo "Initiating LDAP server with NIS schema & ldbm backend..."; \
	@mkdir test-db test-repl ; \
	$(srcdir)/scripts/startup_nis_ldap_server.sh $(srcdir) ldbm

clean-local:
	@rm -rf test-db test-repl *leak *gmon *core

veryclean-local:
	@rm -f data schema
	@rm -r test-db test-repl

all-common:
	@echo "Making all in `pwd`"
	@for i in $(SUBDIRS) $(ALLDIRS); do \
                echo "  Entering subdirectory $$i"; \
                ( cd $$i; make all );  \
                if test $$? != 0 ; then exit 1; fi ;  \
                echo " "; \
        done

install-common: make-links all
	@echo "Making install in `pwd`"
	@for i in $(SUBDIRS); do \
                echo "  Entering subdirectory $$i"; \
                ( cd $$i; make install );  \
                if test $$? != 0 ; then exit 1; fi ;  \
                echo " "; \
	done

make-links:
	( cd $(TARGETDIR); ln -s $(srcdir)/data . )
	( cd $(TARGETDIR); ln -s $(srcdir)/schema . )
	( cd $(TARGETDIR); mkdir test-db test-repl )

clean-common:
	@echo "Making clean in `pwd`"
	@for i in $(SUBDIRS); do \
                echo "  Entering subdirectory $$i"; \
                ( cd $$i; make clean ); \
                if test $$? != 0 ; then exit 1; fi ; \
                echo " "; \
        done

