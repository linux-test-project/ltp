# (C) Copyright IBM Corp 2005
# All rights reserved.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.

MOSTLYCLEANFILES 	= @TEST_CLEAN@ uid_map
EXTRA_DIST              = openhpi.conf

INCLUDES		= @OPENHPI_INCLUDES@

MAINTAINERCLEANFILES 	= Makefile.in

TDEPLIB			= $(top_builddir)/src/libopenhpi.la \
			  $(top_builddir)/utils/libopenhpiutils.la

TESTS_ENVIRONMENT = OPENHPI_PATH=$(top_builddir)/plugins/simulator
TESTS_ENVIRONMENT += OPENHPI_UID_MAP=$(top_builddir)/plugins/simulator/t/uid_map
TESTS_ENVIRONMENT += OPENHPI_CONF=$(top_srcdir)/plugins/simulator/t/openhpi.conf
TESTS_ENVIRONMENT += LD_LIBRARY_PATH=$(top_srcdir)/src/.libs:$(top_srcdir)/ssl/.libs:$(top_srcdir)/utils/.libs

TESTS = sim_sanity_000 \
	sim_sanity_001 \
	sim_sanity_002 \
	sim_sanity_003 \
	sim_sanity_004 \
	sim_sanity_005 \
	sim_sanity_006 \
	sim_sanity_007 \
	sim_sanity_008 \
	sim_sanity_009 \
	sim_sanity_010 \
	sim_sanity_011 \
	sim_sanity_012 \
	sim_sanity_013 \
	sim_sanity_014 \
	sim_sanity_015 \
	sim_sanity_016 \
	sim_sanity_017 \
	sim_sanity_018 \
	sim_sanity_019 \
	sim_sanity_020 \
	sim_sanity_021 \
	sim_sanity_022 \
	sim_sanity_023 \
	sim_sanity_024 \
	sim_sanity_025 \
	sim_sanity_026 \
	sim_sanity_027 \
	sim_sanity_028 \
	sim_sanity_029 \
	sim_sanity_030 \
	sim_sanity_031 \
	sim_sanity_032 \
	sim_sanity_033 \
	sim_sanity_034 \
	sim_sanity_035 \
	sim_sanity_036 \
	sim_sanity_037 \
	sim_sanity_038 \
	sim_sanity_039 \
	sim_sanity_040 \
	sim_sanity_041 \
	sim_sanity_042 \
	sim_sanity_043 \
	sim_sanity_044 \
	sim_sanity_045

check_PROGRAMS = $(TESTS)

sim_sanity_000_SOURCES = sim_sanity_000.c
sim_sanity_000_LDADD = $(TDEPLIB)
sim_sanity_000_LDFLAGS = -export-dynamic

sim_sanity_001_SOURCES = sim_sanity_001.c
sim_sanity_001_LDADD = $(TDEPLIB)
sim_sanity_001_LDFLAGS = -export-dynamic

sim_sanity_002_SOURCES = sim_sanity_002.c
sim_sanity_002_LDADD = $(TDEPLIB)
sim_sanity_002_LDFLAGS = -export-dynamic

sim_sanity_003_SOURCES = sim_sanity_003.c
sim_sanity_003_LDADD = $(TDEPLIB)
sim_sanity_003_LDFLAGS = -export-dynamic

sim_sanity_004_SOURCES = sim_sanity_004.c
sim_sanity_004_LDADD = $(TDEPLIB)
sim_sanity_004_LDFLAGS = -export-dynamic

sim_sanity_005_SOURCES = sim_sanity_005.c
sim_sanity_005_LDADD = $(TDEPLIB)
sim_sanity_005_LDFLAGS = -export-dynamic

sim_sanity_006_SOURCES = sim_sanity_006.c
sim_sanity_006_LDADD = $(TDEPLIB)
sim_sanity_006_LDFLAGS = -export-dynamic

sim_sanity_007_SOURCES = sim_sanity_007.c
sim_sanity_007_LDADD = $(TDEPLIB)
sim_sanity_007_LDFLAGS = -export-dynamic

sim_sanity_008_SOURCES = sim_sanity_008.c
sim_sanity_008_LDADD = $(TDEPLIB)
sim_sanity_008_LDFLAGS = -export-dynamic

sim_sanity_009_SOURCES = sim_sanity_009.c
sim_sanity_009_LDADD = $(TDEPLIB)
sim_sanity_009_LDFLAGS = -export-dynamic

sim_sanity_010_SOURCES = sim_sanity_010.c
sim_sanity_010_LDADD = $(TDEPLIB)
sim_sanity_010_LDFLAGS = -export-dynamic

sim_sanity_011_SOURCES = sim_sanity_011.c
sim_sanity_011_LDADD = $(TDEPLIB)
sim_sanity_011_LDFLAGS = -export-dynamic

sim_sanity_012_SOURCES = sim_sanity_012.c
sim_sanity_012_LDADD = $(TDEPLIB)
sim_sanity_012_LDFLAGS = -export-dynamic

sim_sanity_013_SOURCES = sim_sanity_013.c
sim_sanity_013_LDADD = $(TDEPLIB)
sim_sanity_013_LDFLAGS = -export-dynamic

sim_sanity_014_SOURCES = sim_sanity_014.c
sim_sanity_014_LDADD = $(TDEPLIB)
sim_sanity_014_LDFLAGS = -export-dynamic

sim_sanity_015_SOURCES = sim_sanity_015.c
sim_sanity_015_LDADD = $(TDEPLIB)
sim_sanity_015_LDFLAGS = -export-dynamic

sim_sanity_016_SOURCES = sim_sanity_016.c
sim_sanity_016_LDADD = $(TDEPLIB)
sim_sanity_016_LDFLAGS = -export-dynamic

sim_sanity_017_SOURCES = sim_sanity_017.c
sim_sanity_017_LDADD = $(TDEPLIB)
sim_sanity_017_LDFLAGS = -export-dynamic

sim_sanity_018_SOURCES = sim_sanity_018.c
sim_sanity_018_LDADD = $(TDEPLIB)
sim_sanity_018_LDFLAGS = -export-dynamic

sim_sanity_019_SOURCES = sim_sanity_019.c
sim_sanity_019_LDADD = $(TDEPLIB)
sim_sanity_019_LDFLAGS = -export-dynamic

sim_sanity_020_SOURCES = sim_sanity_020.c
sim_sanity_020_LDADD = $(TDEPLIB)
sim_sanity_020_LDFLAGS = -export-dynamic

sim_sanity_021_SOURCES = sim_sanity_021.c
sim_sanity_021_LDADD = $(TDEPLIB)
sim_sanity_021_LDFLAGS = -export-dynamic

sim_sanity_022_SOURCES = sim_sanity_022.c
sim_sanity_022_LDADD = $(TDEPLIB)
sim_sanity_022_LDFLAGS = -export-dynamic

sim_sanity_023_SOURCES = sim_sanity_023.c
sim_sanity_023_LDADD = $(TDEPLIB)
sim_sanity_023_LDFLAGS = -export-dynamic

sim_sanity_024_SOURCES = sim_sanity_024.c
sim_sanity_024_LDADD = $(TDEPLIB)
sim_sanity_024_LDFLAGS = -export-dynamic

sim_sanity_025_SOURCES = sim_sanity_025.c
sim_sanity_025_LDADD = $(TDEPLIB)
sim_sanity_025_LDFLAGS = -export-dynamic

sim_sanity_026_SOURCES = sim_sanity_026.c
sim_sanity_026_LDADD = $(TDEPLIB)
sim_sanity_026_LDFLAGS = -export-dynamic

sim_sanity_027_SOURCES = sim_sanity_027.c
sim_sanity_027_LDADD = $(TDEPLIB)
sim_sanity_027_LDFLAGS = -export-dynamic

sim_sanity_028_SOURCES = sim_sanity_028.c
sim_sanity_028_LDADD = $(TDEPLIB)
sim_sanity_028_LDFLAGS = -export-dynamic

sim_sanity_029_SOURCES = sim_sanity_029.c
sim_sanity_029_LDADD = $(TDEPLIB)
sim_sanity_029_LDFLAGS = -export-dynamic

sim_sanity_030_SOURCES = sim_sanity_030.c
sim_sanity_030_LDADD = $(TDEPLIB)
sim_sanity_030_LDFLAGS = -export-dynamic

sim_sanity_031_SOURCES = sim_sanity_031.c
sim_sanity_031_LDADD = $(TDEPLIB)
sim_sanity_031_LDFLAGS = -export-dynamic

sim_sanity_032_SOURCES = sim_sanity_032.c
sim_sanity_032_LDADD = $(TDEPLIB)
sim_sanity_032_LDFLAGS = -export-dynamic

sim_sanity_033_SOURCES = sim_sanity_033.c
sim_sanity_033_LDADD = $(TDEPLIB)
sim_sanity_033_LDFLAGS = -export-dynamic

sim_sanity_034_SOURCES = sim_sanity_034.c
sim_sanity_034_LDADD = $(TDEPLIB)
sim_sanity_034_LDFLAGS = -export-dynamic

sim_sanity_035_SOURCES = sim_sanity_035.c
sim_sanity_035_LDADD = $(TDEPLIB)
sim_sanity_035_LDFLAGS = -export-dynamic

sim_sanity_036_SOURCES = sim_sanity_036.c
sim_sanity_036_LDADD = $(TDEPLIB)
sim_sanity_036_LDFLAGS = -export-dynamic

sim_sanity_037_SOURCES = sim_sanity_037.c
sim_sanity_037_LDADD = $(TDEPLIB)
sim_sanity_037_LDFLAGS = -export-dynamic

sim_sanity_038_SOURCES = sim_sanity_038.c
sim_sanity_038_LDADD = $(TDEPLIB)
sim_sanity_038_LDFLAGS = -export-dynamic

sim_sanity_039_SOURCES = sim_sanity_039.c
sim_sanity_039_LDADD = $(TDEPLIB)
sim_sanity_039_LDFLAGS = -export-dynamic

sim_sanity_040_SOURCES = sim_sanity_040.c
sim_sanity_040_LDADD = $(TDEPLIB)
sim_sanity_040_LDFLAGS = -export-dynamic

sim_sanity_041_SOURCES = sim_sanity_041.c
sim_sanity_041_LDADD = $(TDEPLIB)
sim_sanity_041_LDFLAGS = -export-dynamic

sim_sanity_042_SOURCES = sim_sanity_042.c
sim_sanity_042_LDADD = $(TDEPLIB)
sim_sanity_042_LDFLAGS = -export-dynamic

sim_sanity_043_SOURCES = sim_sanity_043.c
sim_sanity_043_LDADD = $(TDEPLIB)
sim_sanity_043_LDFLAGS = -export-dynamic

sim_sanity_044_SOURCES = sim_sanity_044.c
sim_sanity_044_LDADD = $(TDEPLIB)
sim_sanity_044_LDFLAGS = -export-dynamic

sim_sanity_045_SOURCES = sim_sanity_045.c
sim_sanity_045_LDADD = $(TDEPLIB)
sim_sanity_045_LDFLAGS = -export-dynamic

