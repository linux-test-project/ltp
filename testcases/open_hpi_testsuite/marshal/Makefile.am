#
# Copyright (c) 2004 by FORCE Computers.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
#
# Authors:
#    Thomas Kanngieser <thomas.kanngieser@fci.com>
#

.NOTPARALLEL:

SUBDIRS			= t
DIST_SUBDIRS 		= t

MAINTAINERCLEANFILES 	= Makefile.in *~

INCLUDES 		= @OPENHPI_INCLUDES@
# just to clear LIBS
LIBS			=

# marshal and connection used by the daemon and client library
lib_LTLIBRARIES = libopenhpimarshal.la libohudpconnx.la libohtcpconnx.la

libopenhpimarshal_la_SOURCES    = \
	marshal_hpi.h \
	marshal_hpi.c \
	marshal_hpi_types.h \
	marshal_hpi_types.c \
	marshal.h \
        marshal.c

libopenhpimarshal_la_LDFLAGS= -version-info @HPI_LIB_VERSION@

libohudpconnx_la_SOURCES    = \
	connection.h \
	connection.c

libohudpconnx_la_LDFLAGS= -version-info @HPI_LIB_VERSION@
libohudpconnx_la_LIBADD  = $(top_builddir)/marshal/libopenhpimarshal.la

libohtcpconnx_la_SOURCES    = \
	strmsock.h \
	strmsock.cpp

libohtcpconnx_la_LDFLAGS= -version-info @HPI_LIB_VERSION@
libohtcpconnx_la_LIBADD  = $(top_builddir)/marshal/libopenhpimarshal.la

clean-local:
	rm -f *~ core core.*
