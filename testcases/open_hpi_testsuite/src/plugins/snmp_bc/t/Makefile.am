#
# (C) Copyright IBM Corp 2003, 2004
# All rights reserved.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
 
REMOTE_SIM_SOURCES 	= 	bc_resources.c \
				snmp_bc.c \
				snmp_bc_control.c \
	     			snmp_bc_discover.c \
				snmp_bc_hotswap.c \
	     			snmp_bc_sel.c \
				snmp_bc_sensor.c \
				snmp_bc_utils.c \
				snmp_bc_watchdog.c

MOSTLYCLEANFILES 	= Makefile.in $(REMOTE_SIM_SOURCES) $(shell ls *.bb*) $(shell ls *.da) $(shell ls *.gcov) 

SNMPUTILSDIR            = $(top_srcdir)/src/$(SNMPDIR)

INCLUDES                = -I$(top_srcdir)/include -I$(SNMPUTILSDIR) \
                          -I$(top_srcdir)/src/plugins/snmp_bc

AM_CFLAGS		= -g @CFLAGS@ @SNMPFLAGS@ -DDEBUG -ftest-coverage -fprofile-arcs

EXTRA_DIST		= sim_resources.h

noinst_LTLIBRARIES	= libsnmp_bc.la

$(REMOTE_SIM_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		ln -s ../$@; \
	fi

libsnmp_bc_la_SOURCES	= 	sim_resources.c \
				snmp_util.c \
				snmp_bc_session.c \
				$(REMOTE_SIM_SOURCES)

libsnmp_bc_la_LIBADD	= -luuid 

libsnmp_bc_la_LDFLAGS	= -module -avoid-version

libsnmp_bc.la: $(libsnmp_bc_la_OBJECTS) $(libsnmp_bc_la_DEPENDENCIES)
	$(LINK) -rpath $(pkglibdir) $(libsnmp_bc_la_LDFLAGS) $(libsnmp_bc_la_OBJECTS) $(libsnmp_bc_la_LIBADD) $(LIBS)


# Need to put expected test failures in BOTH TESTS and XFAIL_TESTS
TESTS = thotswap_state \
	thotswap_state_err \
	thotswap_reset \
	thotswap_reset_err \
	thotswap_reset_set \
	thotswap_reset_set_err \
	thotswap_power \
	thotswap_power_err \
	thotswap_power_set \
	thotswap_power_set_err \
	thotswap_unsupported \
	tsensor \
	tutils_conv \
	tutils_oid

# XFAIL_TESTS = thotswap_reset

check_PROGRAMS = $(TESTS)

tutils_oid_SOURCES = tutils_oid.c
tutils_oid_LDADD   = snmp_bc_utils.o

tutils_conv_SOURCES = tutils_conv.c
tutils_conv_LDADD   = snmp_bc_utils.o

tsensor_SOURCES = tsensor.c
tsensor_LDADD   = snmp_bc_sensor.o snmp_bc_utils.o

thotswap_state_SOURCES = thotswap_state.c
thotswap_state_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_state_err_SOURCES = thotswap_state_err.c
thotswap_state_err_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_reset_SOURCES = thotswap_reset.c
thotswap_reset_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_reset_err_SOURCES = thotswap_reset_err.c
thotswap_reset_err_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_reset_set_SOURCES = thotswap_reset_set.c
thotswap_reset_set_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_reset_set_err_SOURCES = thotswap_reset_set_err.c
thotswap_reset_set_err_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_power_SOURCES = thotswap_power.c
thotswap_power_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_power_err_SOURCES = thotswap_power_err.c
thotswap_power_err_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_power_set_SOURCES = thotswap_power_set.c
thotswap_power_set_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_power_set_err_SOURCES = thotswap_power_set_err.c
thotswap_power_set_err_LDADD   = snmp_bc_hotswap.o snmp_bc_utils.o

thotswap_unsupported_SOURCES = thotswap_unsupported.c
thotswap_unsupported_LDADD = snmp_bc_hotswap.o snmp_bc_utils.o
