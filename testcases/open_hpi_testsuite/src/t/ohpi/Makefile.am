# (C) Copyright IBM Corp 2004
# All rights reserved.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.

MAINTAINERCLEANFILES = Makefile.in

MOSTLYCLEANFILES 	= @TEST_CLEAN@ uid_map
EXTRA_DIST              = openhpi.conf

INCLUDES		= @OPENHPI_INCLUDES@

TDEPLIB                 = $(top_builddir)/src/libopenhpi.la \
			  $(top_builddir)/utils/libopenhpiutils.la

TESTS_ENVIRONMENT = OPENHPI_PATH=$(top_builddir)/plugins/simulator:$(top_builddir)/plugins/watchdog
TESTS_ENVIRONMENT += OPENHPI_UID_MAP=$(top_builddir)/src/t/ohpi/uid_map
TESTS_ENVIRONMENT += OPENHPI_CONF=$(top_srcdir)/src/t/ohpi/openhpi.conf
TESTS_ENVIRONMENT += LD_LIBRARY_PATH=$(top_srcdir)/src/.libs:$(top_srcdir)/ssl/.libs:$(top_srcdir)/utils/.libs

TESTS = \
        ohpi_007 \
        ohpi_008 \
        ohpi_009 \
        ohpi_010 \
        ohpi_024 \
        ohpi_025 \
        ohpi_026 \
        ohpi_027 \
        ohpi_028 \
        ohpi_029 \
        ohpi_030 \
        ohpi_031 \
        ohpi_032 \
        ohpi_033 \
        ohpi_034 \
        ohpi_035 \
        ohpi_036 \
        ohpi_037 \
        ohpi_038 \
        ohpi_039 \
	ohpi_version \
	hpiinjector

check_PROGRAMS = $(TESTS)

ohpi_007_SOURCES = ohpi_007.c
ohpi_007_LDADD   = $(TDEPLIB)

ohpi_008_SOURCES = ohpi_008.c
ohpi_008_LDADD   = $(TDEPLIB)

ohpi_009_SOURCES = ohpi_009.c
ohpi_009_LDADD   = $(TDEPLIB)

ohpi_010_SOURCES = ohpi_010.c
ohpi_010_LDADD   = $(TDEPLIB)

ohpi_024_SOURCES = ohpi_024.c
ohpi_024_LDADD   = $(TDEPLIB)

ohpi_025_SOURCES = ohpi_025.c
ohpi_025_LDADD   = $(TDEPLIB)

ohpi_026_SOURCES = ohpi_026.c
ohpi_026_LDADD   = $(TDEPLIB)

ohpi_027_SOURCES = ohpi_027.c
ohpi_027_LDADD   = $(TDEPLIB)

ohpi_028_SOURCES = ohpi_028.c
ohpi_028_LDADD   = $(TDEPLIB)

ohpi_029_SOURCES = ohpi_029.c
ohpi_029_LDADD   = $(TDEPLIB)

ohpi_030_SOURCES = ohpi_030.c
ohpi_030_LDADD   = $(TDEPLIB)

ohpi_031_SOURCES = ohpi_031.c
ohpi_031_LDADD   = $(TDEPLIB)

ohpi_032_SOURCES = ohpi_032.c
ohpi_032_LDADD   = $(TDEPLIB)

ohpi_033_SOURCES = ohpi_033.c
ohpi_033_LDADD   = $(TDEPLIB)

ohpi_034_SOURCES = ohpi_034.c
ohpi_034_LDADD   = $(TDEPLIB)

ohpi_035_SOURCES = ohpi_035.c
ohpi_035_LDADD   = $(TDEPLIB)

ohpi_036_SOURCES = ohpi_036.c
ohpi_036_LDADD   = $(TDEPLIB)

ohpi_037_SOURCES = ohpi_037.c
ohpi_037_LDADD   = $(TDEPLIB)

ohpi_038_SOURCES = ohpi_038.c
ohpi_038_LDADD   = $(TDEPLIB)

ohpi_039_SOURCES = ohpi_039.c
ohpi_039_LDADD   = $(TDEPLIB)

ohpi_version_SOURCES = ohpi_version.c
ohpi_version_LDADD   = $(TDEPLIB)

hpiinjector_SOURCES = hpiinjector.c
hpiinjector_LDADD   = $(TDEPLIB)

