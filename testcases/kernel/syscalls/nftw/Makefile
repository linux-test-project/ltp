# IBM Corporation 
# 10/31/2002   Port to LTP     robbiew@us.ibm.com 
# 06/30/2001   Port to Linux   nsharoff@us.ibm.com

#
# $Copyright: $
# Copyright (c) 1984-1999
# Sequent Computer Systems, Inc.   All rights reserved.
#  
# This software is furnished under a license and may be used
# only in accordance with the terms of that license and with the
# inclusion of the above copyright notice.   This software may not
# be provided or otherwise made available to, or used by, any
# other person.  No title to or ownership of the software is
# hereby transferred.

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -ansi -DLINUX -D_XOPEN_SOURCE_EXTENDED -D_XOPEN_SOURCE

MAKE_TARGETS		:= nftw01 nftw6401

nftw01: CPPFLAGS += -D_LARGEFILE_SOURCE

nftw01: nftw.o tools.o test_func.o test.o lib.o 
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o "$@"

nftw64.o tools64.o test_func64.o test64.o lib64.o: %.o: %.c

%64.o: CPPFLAGS += -D_LARGEFILE64_SOURCE

nftw6401: nftw64.o tools64.o test_func64.o test64.o lib64.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
