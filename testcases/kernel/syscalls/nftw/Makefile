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

LINT =		lint
CFLAGS+=        -Wall -g -ansi -O -DLINUX -D_XOPEN_SOURCE_EXTENDED -D_XOPEN_SOURCE -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -I../../../../include 
SRCS1 =		nftw.c tools.c test_func.c test.c lib.c
OBJS1 =		nftw.o tools.o test_func.o test.o lib.o 
SRCS2 =		nftw64.c tools64.c test_func64.c test64.c lib64.c
OBJS2 =		nftw64.o tools64.o test_func64.o test64.o lib64.o 
BINS= 		nftw01  nftw6401

all:		nftw01 nftw6401

nftw01:	$(&)	nftw.h $(OBJS1) ../../../../include/test.h ../../../../include/usctest.h ../../../../lib/libltp.a
	$(CC) $(CFLAGS) -o nftw01 $(OBJS1) $(LDFLAGS) ../../../../lib/libltp.a

nftw6401:$(&)	nftw64.h $(OBJS2) ../../../../include/test.h ../../../../include/usctest.h ../../../../lib/libltp.a
	$(CC) $(CFLAGS) -o nftw6401 $(OBJS2) $(LDFLAGS) ../../../../lib/libltp.a

remove_bin:
	rm -f $(BINS) $(OBJS1) $(OBJS2)

remove_obj:
	rm -f $(OBJS1)
	rm -f $(OBJS2)

clean:	remove_bin remove_obj 

lint:
	$(LINT) -mnuts $(SRCS1) 
	$(LINT) -mnuts $(SRCS2) 

install: 
	ln -f $(BINS) ../../../bin
