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

SUBDIRS 		=

INCLUDES		= -I$(top_srcdir)/include 

AM_CFLAGS		= -g @CFLAGS@ @SNMPFLAGS@ -DDEBUG

noinst_LTLIBRARIES		= libsnmp_util.la

EXTRA_DIST = snmp_util.h 

libsnmp_util_la_SOURCES = snmp_util.c 

libsnmp_util_la_LIBADD	= -luuid @SNMPLIBS@
libsnmp_util_la_LDFLAGS	= -version-info @HPI_LIB_VERSION@
