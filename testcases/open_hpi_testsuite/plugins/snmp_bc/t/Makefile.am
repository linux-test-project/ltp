# (C) Copyright IBM Corp 2003, 2006
# All rights reserved.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
# file and program are licensed under a BSD style license. See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.

# Generated files - need to keep in sync with parent directory's Makefile.am
GENERATED_EVENT_XML_MAP = event.xml
GENERATED_EVENT_CODE = el2event.c
GENERATED_CODE = $(GENERATED_EVENT_XML_MAP) $(GENERATED_EVENT_CODE)

REMOTE_SIM_SOURCES = \
		snmp_bc.c \
		snmp_bc_annunciator.c \
		snmp_bc_control.c \
	     	snmp_bc_discover.c \
		snmp_bc_discover_bc.c \
		snmp_bc_discover_rsa.c \
		snmp_bc_event.c \
		snmp_bc_inventory.c \
		snmp_bc_hotswap.c \
		snmp_bc_power.c \
		snmp_bc_reset.c \
		snmp_bc_resources.c \
		snmp_bc_resources_rsa.c \
		snmp_bc_sel.c \
		snmp_bc_sensor.c \
		snmp_bc_session.c \
	 	snmp_bc_time.c \
	 	snmp_bc_utils.c \
		snmp_bc_watchdog.c \
		snmp_bc_xml2event.c

MOSTLYCLEANFILES = @TEST_CLEAN@ $(REMOTE_SIM_SOURCES) uid_map
MOSTLYCLEANFILES += $(GENERATED_CODE)

MAINTAINERCLEANFILES = Makefile.in

SNMPUTILSDIR = $(top_srcdir)/$(SNMPDIR)

INCLUDES = \
		@OPENHPI_INCLUDES@ \
		-I$(top_srcdir)/snmp \
		-I$(top_srcdir)/include \
		-I$(SNMPUTILSDIR) \
                -I$(top_srcdir)/plugins/snmp_bc

AM_CFLAGS = @SNMPFLAGS@
# AM_CFLAGS = @CFLAGS@ -I$(top_srcdir)/include @SNMPFLAGS@

EXTRA_DIST = sim_resources.h openhpi.conf tsetup.h sim_test_file

noinst_LTLIBRARIES = libsnmp_bc.la

$(REMOTE_SIM_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		$(LN_S) $(top_srcdir)/plugins/snmp_bc/$@; \
	fi

libsnmp_bc_la_SOURCES = \
		sim_init.c \
		sim_resources.c \
		sim_file.c \
		snmp_utils.c \
		tsetup.c
nodist_libsnmp_bc_la_SOURCES = $(GENERATED_EVENT_CODE) $(REMOTE_SIM_SOURCES)

# libopenhpi_la_LIBADD    = @STATIC_PLUGIN_LIBS@ @STATIC_PLUGIN_EXTRA_LIBS@ $(top_builddir)/utils/libopenhpiutils.la
# libopenhpi_la_LDFLAGS   = -L$(top_builddir)/utils -version-info @HPI_LIB_VERSION@ -export-symbols $(top_srcdir)/src/hpi.sym

libsnmp_bc_la_LIBADD = -luuid @SNMPLIBS@ $(top_builddir)/utils/libopenhpiutils.la
libsnmp_bc_la_LDFLAGS = -L$(top_builddir)/utils -module -version-info @HPI_LIB_VERSION@
# libsnmp_bc_la_LDFLAGS = -version 0:0:0

libsnmp_bc.la: $(libsnmp_bc_la_OBJECTS) $(libsnmp_bc_la_DEPENDENCIES)
	$(LINK) -rpath $(pkglibdir) $(libsnmp_bc_la_LDFLAGS) $(libsnmp_bc_la_OBJECTS) $(libsnmp_bc_la_LIBADD) $(LIBS)

# Keep in sync with parent directory's Makefile.am
# FIXME:: Be nice if auto-generation rules where just in snmp_bc/Makefile.am
# and not repeated here; but t directory is done first.
EVENT_MAP_FILE = $(top_srcdir)/plugins/snmp_bc/snmp_bc_event.map
EVENT_MAP_SCRIPT = $(top_srcdir)/plugins/snmp_bc/eventmap2code.pl
EVENT_XML_MAP_SCRIPT = $(top_srcdir)/scripts/text2cstr.pl

$(GENERATED_EVENT_CODE): $(EVENT_MAP_FILE) $(EVENT_MAP_SCRIPT) $(EVENT_XML_MAP_SCRIPT)
	$(EVENT_MAP_SCRIPT) -xml -idir $(top_srcdir)/plugins/snmp_bc -mapfile snmp_bc_event.map
	$(EVENT_XML_MAP_SCRIPT) -s eventxml $(GENERATED_EVENT_XML_MAP) > $(GENERATED_EVENT_CODE)

# Setup environment variables for TESTS programs
TESTS_ENVIRONMENT  = OPENHPI_CONF=$(srcdir)/openhpi.conf
TESTS_ENVIRONMENT += OPENHPI_SIMTEST_FILE=$(srcdir)/sim_test_file
#TESTS_ENVIRONMENT += OPENHPI_ERROR=YES
#TESTS_ENVIRONMENT += OPENHPI_DEBUG=YES
TESTS_ENVIRONMENT += LD_LIBRARY_PATH=$(top_srcdir)/src/.libs:$(top_srcdir)/ssl/.libs:$(top_srcdir)/utils/.libs:$(top_srcdir)/plugins/snmp/.libs:$(top_srcdir)/plugins/snmp_bc/t/.libs
TESTS_ENVIRONMENT += OPENHPI_UID_MAP=$(shell pwd)/uid_map
TESTS_ENVIRONMENT += OPENHPI_PATH=$(shell pwd)

TESTS = \
	tsim_file \
	tevent \
	tcontrol_parms \
	tset_resource_tag \
	tset_resource_sev \
	tsnmp_bc_getset \
	tsensorget001 \
	tsensorget002 \
	tsensorget003 \
	tsensorget004 \
	tsensorget005 \
	tsensorget010 \
	tsensorget013 \
	tsensorget016 \
	tsensorget031 \
	tsensorget033 \
	tsensorget034 \
	tsensorget035 \
	tsensorget036 \
	tsensorget040 \
	tsensorget041 \
	tcontrol001   \
	tcontrol002   \
	tcontrol003   \
	tcontrolfailover \
	tdiscover001  \
	tel001        \
	tel002        \
	tel003        \
	tel004        \
	tel005        \
	tel006        \
	tel007        \
	thotswap001   \
	thotswap-mhs001 \
	thotswap-simple001 \
	tinv001       \
	tinv002       \
	tinv003       \
	tinv004       \
	tinv005       \
	tinv006       \
	tinv007       \
	tpower001     \
	treset001     \
	tsptime001    \
	tsptime002    \
	tsensorset001 \
	tsensorset002 \
	tsensorset003 \
	tsensorset004 \
	tsensorset005 \
	tsensorset006 \
	tsensorset007 \
	tsensorset008 \
	tsensorset009 \
	tsensorset010 \
	tsensorset011 \
	tsensorset012 \
	tsensorset013 \
	tsensorset015 \
	tsensorset017 \
	tsensorset018 \
	tsensorset019 \
	tsensorset020 \
	tsensorset021 \
	tsensorset022 \
	tsensorset023 \
	tsensorset024
#	tdiscover002
#	tsnmp_bc_get_guid

# Need to put expected test failures in BOTH TESTS and XFAIL_TESTS
# XFAIL_TESTS = thotswap_reset

check_PROGRAMS = $(TESTS)

# Unit test using normal IF calls and simulation library
tsim_file_SOURCES = tsim_file.c
tsim_file_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

# Unit test using normal IF calls and simulation library
tevent_SOURCES = tevent.c
tevent_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

# Unit test using normal IF calls and simulation library
tcontrol_parms_SOURCES = tcontrol_parms.c
tcontrol_parms_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

# Unit test using normal IF calls and simulation library
tset_resource_tag_SOURCES = tset_resource_tag.c
tset_resource_tag_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

# Unit test using normal IF calls and simulation library
tset_resource_sev_SOURCES = tset_resource_sev.c
tset_resource_sev_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

# Unit test using normal IF calls and simulation library
tsnmp_bc_getset_SOURCES = tsnmp_bc_getset.c
tsnmp_bc_getset_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget001_SOURCES = tsensorget001.c
tsensorget001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget002_SOURCES = tsensorget002.c
tsensorget002_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget003_SOURCES = tsensorget003.c
tsensorget003_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget004_SOURCES = tsensorget004.c
tsensorget004_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget005_SOURCES = tsensorget005.c
tsensorget005_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget010_SOURCES = tsensorget010.c
tsensorget010_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget013_SOURCES = tsensorget013.c
tsensorget013_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget016_SOURCES = tsensorget016.c
tsensorget016_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget031_SOURCES = tsensorget031.c
tsensorget031_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget033_SOURCES = tsensorget033.c
tsensorget033_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget034_SOURCES = tsensorget034.c
tsensorget034_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget035_SOURCES = tsensorget035.c
tsensorget035_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget036_SOURCES = tsensorget036.c
tsensorget036_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget040_SOURCES = tsensorget040.c
tsensorget040_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorget041_SOURCES = tsensorget041.c
tsensorget041_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset001_SOURCES = tsensorset001.c
tsensorset001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset002_SOURCES = tsensorset002.c
tsensorset002_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset003_SOURCES = tsensorset003.c
tsensorset003_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset004_SOURCES = tsensorset004.c
tsensorset004_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset005_SOURCES = tsensorset005.c
tsensorset005_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset006_SOURCES = tsensorset006.c
tsensorset006_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset007_SOURCES = tsensorset007.c
tsensorset007_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset008_SOURCES = tsensorset008.c
tsensorset008_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset009_SOURCES = tsensorset009.c
tsensorset009_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset010_SOURCES = tsensorset010.c
tsensorset010_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset011_SOURCES = tsensorset011.c
tsensorset011_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset012_SOURCES = tsensorset012.c
tsensorset012_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset013_SOURCES = tsensorset013.c
tsensorset013_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset015_SOURCES = tsensorset015.c
tsensorset015_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset017_SOURCES = tsensorset017.c
tsensorset017_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset018_SOURCES = tsensorset018.c
tsensorset018_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset019_SOURCES = tsensorset019.c
tsensorset019_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset020_SOURCES = tsensorset020.c
tsensorset020_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset021_SOURCES = tsensorset021.c
tsensorset021_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset022_SOURCES = tsensorset022.c
tsensorset022_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset023_SOURCES = tsensorset023.c
tsensorset023_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsensorset024_SOURCES = tsensorset024.c
tsensorset024_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tel001_SOURCES = tel001.c
tel001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tel002_SOURCES = tel002.c
tel002_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tel003_SOURCES = tel003.c
tel003_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tel004_SOURCES = tel004.c
tel004_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tel005_SOURCES = tel005.c
tel005_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tel006_SOURCES = tel006.c
tel006_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tel007_SOURCES = tel007.c
tel007_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tinv001_SOURCES = tinv001.c
tinv001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tinv002_SOURCES = tinv002.c
tinv002_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tinv003_SOURCES = tinv003.c
tinv003_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tinv004_SOURCES = tinv004.c
tinv004_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tinv005_SOURCES = tinv005.c
tinv005_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tinv006_SOURCES = tinv006.c
tinv006_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tinv007_SOURCES = tinv007.c
tinv007_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
thotswap001_SOURCES = thotswap001.c
thotswap001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
thotswap_mhs001_SOURCES = thotswap-mhs001.c
thotswap_mhs001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
thotswap_simple001_SOURCES = thotswap-simple001.c
thotswap_simple001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tpower001_SOURCES = tpower001.c
tpower001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
treset001_SOURCES = treset001.c
treset001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsptime001_SOURCES = tsptime001.c
tsptime001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tsptime002_SOURCES = tsptime002.c
tsptime002_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tdiscover001_SOURCES = tdiscover001.c
tdiscover001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
#tdiscover002_SOURCES = tdiscover002.c
#tdiscover002_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
#		 $(top_builddir)/src/libopenhpi.la \
#		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tcontrol001_SOURCES = tcontrol001.c
tcontrol001_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tcontrol002_SOURCES = tcontrol002.c
tcontrol002_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

#
tcontrol003_SOURCES = tcontrol003.c
tcontrol003_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

tcontrolfailover_SOURCES = tcontrolfailover.c
tcontrolfailover_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
		 $(top_builddir)/src/libopenhpi.la \
		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

# Unit test using normal IF calls and simulation library
#tsnmp_bc_get_guid_SOURCES = tsnmp_bc_get_guid.c
#tsnmp_bc_get_guid_LDADD   = $(top_builddir)/utils/libopenhpiutils.la \
#		 $(top_builddir)/src/libopenhpi.la \
#		 $(top_builddir)/plugins/snmp_bc/t/libsnmp_bc.la

