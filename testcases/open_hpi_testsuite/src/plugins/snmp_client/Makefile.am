#
# (C) Copyright IBM Corp 2003
# All rights reserved.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
 
MAINTAINERCLEANFILES 	= Makefile.in

SNMPUTILSDIR    = $(top_srcdir)/src/$(SNMPDIR)
SNMPUTILBDIR    = $(top_builddir)/src/$(SNMPDIR)

EXTRA_DIST 		= \
			  snmp_client.h \
			  snmp_client_res.h \
			  snmp_client_utils.h

INCLUDES		= -I$(top_srcdir)/include -I$(SNMPUTILSDIR)

AM_CFLAGS		= -g @CFLAGS@ @SNMPFLAGS@ -DDEBUG

pkglib_LTLIBRARIES		= libsnmp_client.la

libsnmp_client_la_SOURCES	= \
				  snmp_client.c \
				  snmp_client_discover.c \
				  snmp_client_get_rdrs.c \
				  snmp_client_gen_evts.c \
				  snmp_client_utils.c 
				  
libsnmp_client_la_LIBADD	= -luuid @SNMPLIBS@ $(SNMPUTILSDIR)/libsnmp_util.la
libsnmp_client_la_LDFLAGS	= -version-info @HPI_LIB_VERSION@
