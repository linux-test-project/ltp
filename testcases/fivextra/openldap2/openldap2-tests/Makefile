#  (C) Copyright IBM Corp. 2003
#
###########################################################################
# name of file	: Makefile						  #
# description	: makefile for openldap2 testcase			  #
# 		Robb Romans <robb@austin.ibm.com>			  #
###########################################################################

TARGETS=`find . -maxdepth 1 -perm +111 -type f`

## Portions Copyright 1998-2002 The OpenLDAP Foundation, All Rights Reserved.
## COPYING RESTRICTIONS APPLY, see COPYRIGHT file

#SUBDIRS    = progs
#TARGETDIR  = ../../../bin/openldap2-tests

all: 		#all-common
clean:		#clean-local


clean-local: clean-common
	@rm -f *leak *gmon *core *~


all-common:
	echo "Making all in `pwd`"
	for i in $(SUBDIRS) ; do \
                echo "  Entering subdirectory $$i"; \
                ( cd $$i; make all );  \
                if test $$? != 0 ; then exit 1; fi ;  \
                echo " "; \
        done

install: all
	echo "Making install in `pwd`"
	for i in $(SUBDIRS); do \
                echo "  Entering subdirectory $$i"; \
                ( cd $$i; make install );  \
                if test $$? != 0 ; then exit 1; fi ;  \
                echo " "; \
	done
	cp -a $(TARGETS) $(TARGETDIR) || break

clean-common:
	@echo "Making clean in `pwd`"
	@for i in $(SUBDIRS); do \
                echo "  Entering subdirectory $$i"; \
                ( cd $$i; make clean ); \
                if test $$? != 0 ; then exit 1; fi ; \
                echo " "; \
	done

