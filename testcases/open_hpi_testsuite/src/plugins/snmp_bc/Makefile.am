# (C) Copyright IBM Corp 2003, 2004
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

SUBDIRS 		= t

INCLUDES		= -I$(top_srcdir)/include -I$(SNMPUTILSDIR)

AM_CFLAGS		= @CFLAGS@ @SNMPFLAGS@ -DDEBUG

pkglib_LTLIBRARIES	= libsnmp_bc.la

EXTRA_DIST		=	bc_resources.h \
				snmp_bc.h \
				snmp_bc_control.h \
	     			snmp_bc_discover.h \
				snmp_bc_hotswap.h \
	     			snmp_bc_sel.h \
				snmp_bc_time.h \
				snmp_bc_sensor.h \
				snmp_bc_session.h \
				snmp_bc_utils.h \
				snmp_bc_watchdog.h

libsnmp_bc_la_SOURCES	=	bc_resources.c \
				snmp_bc.c \
				snmp_bc_control.c \
	     			snmp_bc_discover.c \
				snmp_bc_hotswap.c \
	     			snmp_bc_sel.c \
				snmp_bc_sensor.c \
				snmp_bc_session.c \
				snmp_bc_utils.c \
				snmp_bc_watchdog.c

libsnmp_bc_la_LIBADD	= -luuid @SNMPLIBS@ $(SNMPUTILBDIR)/libsnmp_util.la
libsnmp_bc_la_LDFLAGS	= -version-info @HPI_LIB_VERSION@
