#
#  Copyright (c) International Business Machines  Corp., 2001
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
###########################################################################
LKSCTP_DIR=/usr/src/lksctp-2.4.18/lksctp
INCLUDE_USR=${LKSCTP_DIR}/test/libc/include
LINUX=${LKSCTP_DIR}/linux_sctp
SCTP_LIB_DIR=${LKSCTP_DIR}/test
CC=gcc

CFLAGS+=-g ${INCLUDES} ${WARNINGS} -DTEST_FRAME=1 \
        -DCONFIG_IP_SCTP=1 \
        -DUSE_CRC32C=1 \
        -DCONFIG_IP_SCTP_ADDIP=0 -DSCTP_DEBUG=1

V6FLAGS=-DCONFIG_IPV6=1 -DTEST_V6=1 -I${INCLUDE_V6}


CFLAGS+=	-I../../../../include -I${INCLUDE_USR} -I${LINUX}/include
LOADLIBES+=	-L../../../../lib -lltp -L${SCTP_LIB_DIR} -lsctp \
		-L.. -lsctp_test

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)
	rm -f ../../../bin/$(TARGETS)

