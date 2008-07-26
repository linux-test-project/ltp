# (C) Copyright IBM Corp 2003, 2004
# All rights reserved.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.

MAINTAINERCLEANFILES = Makefile.in

REMOTE_SOURCES		= rpt_utils.c \
			  epath_utils.c \
			  uid_utils.c \
			  sahpi_enum_utils.c \
			  sahpiatca_enum_utils.c \
			  sahpi_event_encode.c \
			  sahpi_event_utils.c \
			  sahpi_struct_utils.c \
			  sahpi_time_utils.c

MOSTLYCLEANFILES 	= $(REMOTE_SOURCES) @TEST_CLEAN@

INCLUDES		= @OPENHPI_INCLUDES@

EXTRA_DIST              = rpt_resources.h

LIBS += -luuid

$(REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		$(LN_S) $(top_srcdir)/utils/$@; \
	fi

TESTS = rpt_utils_000 \
	rpt_utils_001 \
	rpt_utils_002 \
        rpt_utils_003 \
        rpt_utils_004 \
        rpt_utils_005 \
        rpt_utils_006 \
        rpt_utils_007 \
        rpt_utils_008 \
        rpt_utils_009 \
        rpt_utils_010 \
        rpt_utils_011 \
        rpt_utils_012 \
        rpt_utils_013 \
        rpt_utils_014 \
        rpt_utils_015 \
        rpt_utils_016 \
        rpt_utils_017 \
        rpt_utils_018 \
        rpt_utils_019 \
        rpt_utils_020 \
        rpt_utils_021 \
        rpt_utils_022 \
        rpt_utils_023 \
        rpt_utils_024 \
        rpt_utils_025 \
        rpt_utils_026 \
        rpt_utils_027 \
        rpt_utils_028 \
        rpt_utils_029 \
        rpt_utils_030 \
        rpt_utils_031 \
        rpt_utils_032 \
        rpt_utils_033 \
        rpt_utils_034 \
        rpt_utils_035 \
        rpt_utils_037 \
        rpt_utils_038 \
        rpt_utils_039 \
        rpt_utils_040 \
        rpt_utils_041 \
        rpt_utils_042 \
        rpt_utils_043 \
        rpt_utils_044 \
        rpt_utils_045 \
        rpt_utils_046 \
        rpt_utils_047 \
        rpt_utils_048 \
        rpt_utils_049 \
        rpt_utils_050 \
        rpt_utils_051 \
        rpt_utils_052 \
        rpt_utils_053 \
        rpt_utils_054 \
        rpt_utils_055 \
        rpt_utils_056 \
        rpt_utils_057 \
        rpt_utils_058 \
        rpt_utils_059 \
        rpt_utils_060 \
        rpt_utils_061 \
        rpt_utils_062 \
        rpt_utils_063 \
        rpt_utils_064 \
        rpt_utils_065 \
        rpt_utils_066 \
        rpt_utils_067 \
        rpt_utils_068 \
        rpt_utils_069 \
	rpt_utils_070 \
	rpt_utils_071 \
	rpt_utils_072 \
	rpt_utils_073 \
	rpt_utils_074 \
	rpt_utils_075 \
	rpt_utils_076 \
	rpt_utils_077 \
        rpt_utils_078 \
        rpt_utils_079 \
        rpt_utils_080 \
        rpt_utils_081 \
        rpt_utils_082 \
        rpt_utils_1000

check_PROGRAMS = $(TESTS)

rpt_utils_000_SOURCES = rpt_utils_000.c
nodist_rpt_utils_000_SOURCES = $(REMOTE_SOURCES)
rpt_utils_001_SOURCES = rpt_utils_001.c
nodist_rpt_utils_001_SOURCES = $(REMOTE_SOURCES)
rpt_utils_002_SOURCES = rpt_utils_002.c
nodist_rpt_utils_002_SOURCES = $(REMOTE_SOURCES)
rpt_utils_003_SOURCES = rpt_utils_003.c
nodist_rpt_utils_003_SOURCES = $(REMOTE_SOURCES)
rpt_utils_004_SOURCES = rpt_utils_004.c
nodist_rpt_utils_004_SOURCES = $(REMOTE_SOURCES)
rpt_utils_005_SOURCES = rpt_utils_005.c
nodist_rpt_utils_005_SOURCES = $(REMOTE_SOURCES)
rpt_utils_006_SOURCES = rpt_utils_006.c
nodist_rpt_utils_006_SOURCES = $(REMOTE_SOURCES)
rpt_utils_007_SOURCES = rpt_utils_007.c
nodist_rpt_utils_007_SOURCES = $(REMOTE_SOURCES)
rpt_utils_008_SOURCES = rpt_utils_008.c
nodist_rpt_utils_008_SOURCES = $(REMOTE_SOURCES)
rpt_utils_009_SOURCES = rpt_utils_009.c
nodist_rpt_utils_009_SOURCES = $(REMOTE_SOURCES)
rpt_utils_010_SOURCES = rpt_utils_010.c
nodist_rpt_utils_010_SOURCES = $(REMOTE_SOURCES)
rpt_utils_011_SOURCES = rpt_utils_011.c
nodist_rpt_utils_011_SOURCES = $(REMOTE_SOURCES)
rpt_utils_012_SOURCES = rpt_utils_012.c
nodist_rpt_utils_012_SOURCES = $(REMOTE_SOURCES)
rpt_utils_013_SOURCES = rpt_utils_013.c
nodist_rpt_utils_013_SOURCES = $(REMOTE_SOURCES)
rpt_utils_014_SOURCES = rpt_utils_014.c
nodist_rpt_utils_014_SOURCES = $(REMOTE_SOURCES)
rpt_utils_015_SOURCES = rpt_utils_015.c
nodist_rpt_utils_015_SOURCES = $(REMOTE_SOURCES)
rpt_utils_016_SOURCES = rpt_utils_016.c
nodist_rpt_utils_016_SOURCES = $(REMOTE_SOURCES)
rpt_utils_017_SOURCES = rpt_utils_017.c
nodist_rpt_utils_017_SOURCES = $(REMOTE_SOURCES)
rpt_utils_018_SOURCES = rpt_utils_018.c
nodist_rpt_utils_018_SOURCES = $(REMOTE_SOURCES)
rpt_utils_019_SOURCES = rpt_utils_019.c
nodist_rpt_utils_019_SOURCES = $(REMOTE_SOURCES)
rpt_utils_020_SOURCES = rpt_utils_020.c
nodist_rpt_utils_020_SOURCES = $(REMOTE_SOURCES)
rpt_utils_021_SOURCES = rpt_utils_021.c
nodist_rpt_utils_021_SOURCES = $(REMOTE_SOURCES)
rpt_utils_022_SOURCES = rpt_utils_022.c
nodist_rpt_utils_022_SOURCES = $(REMOTE_SOURCES)
rpt_utils_023_SOURCES = rpt_utils_023.c
nodist_rpt_utils_023_SOURCES = $(REMOTE_SOURCES)
rpt_utils_024_SOURCES = rpt_utils_024.c
nodist_rpt_utils_024_SOURCES = $(REMOTE_SOURCES)
rpt_utils_025_SOURCES = rpt_utils_025.c
nodist_rpt_utils_025_SOURCES = $(REMOTE_SOURCES)
rpt_utils_026_SOURCES = rpt_utils_026.c
nodist_rpt_utils_026_SOURCES = $(REMOTE_SOURCES)
rpt_utils_027_SOURCES = rpt_utils_027.c
nodist_rpt_utils_027_SOURCES = $(REMOTE_SOURCES)
rpt_utils_028_SOURCES = rpt_utils_028.c
nodist_rpt_utils_028_SOURCES = $(REMOTE_SOURCES)
rpt_utils_029_SOURCES = rpt_utils_029.c
nodist_rpt_utils_029_SOURCES = $(REMOTE_SOURCES)
rpt_utils_030_SOURCES = rpt_utils_030.c
nodist_rpt_utils_030_SOURCES = $(REMOTE_SOURCES)
rpt_utils_031_SOURCES = rpt_utils_031.c
nodist_rpt_utils_031_SOURCES = $(REMOTE_SOURCES)
rpt_utils_032_SOURCES = rpt_utils_032.c
nodist_rpt_utils_032_SOURCES = $(REMOTE_SOURCES)
rpt_utils_033_SOURCES = rpt_utils_033.c
nodist_rpt_utils_033_SOURCES = $(REMOTE_SOURCES)
rpt_utils_034_SOURCES = rpt_utils_034.c
nodist_rpt_utils_034_SOURCES = $(REMOTE_SOURCES)
rpt_utils_035_SOURCES = rpt_utils_035.c
nodist_rpt_utils_035_SOURCES = $(REMOTE_SOURCES)
rpt_utils_037_SOURCES = rpt_utils_037.c
nodist_rpt_utils_037_SOURCES = $(REMOTE_SOURCES)
rpt_utils_038_SOURCES = rpt_utils_038.c
nodist_rpt_utils_038_SOURCES = $(REMOTE_SOURCES)
rpt_utils_039_SOURCES = rpt_utils_039.c
nodist_rpt_utils_039_SOURCES = $(REMOTE_SOURCES)
rpt_utils_040_SOURCES = rpt_utils_040.c
nodist_rpt_utils_040_SOURCES = $(REMOTE_SOURCES)
rpt_utils_041_SOURCES = rpt_utils_041.c
nodist_rpt_utils_041_SOURCES = $(REMOTE_SOURCES)
rpt_utils_042_SOURCES = rpt_utils_042.c
nodist_rpt_utils_042_SOURCES = $(REMOTE_SOURCES)
rpt_utils_043_SOURCES = rpt_utils_043.c
nodist_rpt_utils_043_SOURCES = $(REMOTE_SOURCES)
rpt_utils_044_SOURCES = rpt_utils_044.c
nodist_rpt_utils_044_SOURCES = $(REMOTE_SOURCES)
rpt_utils_045_SOURCES = rpt_utils_045.c
nodist_rpt_utils_045_SOURCES = $(REMOTE_SOURCES)
rpt_utils_046_SOURCES = rpt_utils_046.c
nodist_rpt_utils_046_SOURCES = $(REMOTE_SOURCES)
rpt_utils_047_SOURCES = rpt_utils_047.c
nodist_rpt_utils_047_SOURCES = $(REMOTE_SOURCES)
rpt_utils_048_SOURCES = rpt_utils_048.c
nodist_rpt_utils_048_SOURCES = $(REMOTE_SOURCES)
rpt_utils_049_SOURCES = rpt_utils_049.c
nodist_rpt_utils_049_SOURCES = $(REMOTE_SOURCES)
rpt_utils_050_SOURCES = rpt_utils_050.c
nodist_rpt_utils_050_SOURCES = $(REMOTE_SOURCES)
rpt_utils_051_SOURCES = rpt_utils_051.c
nodist_rpt_utils_051_SOURCES = $(REMOTE_SOURCES)
rpt_utils_052_SOURCES = rpt_utils_052.c
nodist_rpt_utils_052_SOURCES = $(REMOTE_SOURCES)
rpt_utils_053_SOURCES = rpt_utils_053.c
nodist_rpt_utils_053_SOURCES = $(REMOTE_SOURCES)
rpt_utils_054_SOURCES = rpt_utils_054.c
nodist_rpt_utils_054_SOURCES = $(REMOTE_SOURCES)
rpt_utils_055_SOURCES = rpt_utils_055.c
nodist_rpt_utils_055_SOURCES = $(REMOTE_SOURCES)
rpt_utils_056_SOURCES = rpt_utils_056.c
nodist_rpt_utils_056_SOURCES = $(REMOTE_SOURCES)
rpt_utils_057_SOURCES = rpt_utils_057.c
nodist_rpt_utils_057_SOURCES = $(REMOTE_SOURCES)
rpt_utils_058_SOURCES = rpt_utils_058.c
nodist_rpt_utils_058_SOURCES = $(REMOTE_SOURCES)
rpt_utils_059_SOURCES = rpt_utils_059.c
nodist_rpt_utils_059_SOURCES = $(REMOTE_SOURCES)
rpt_utils_060_SOURCES = rpt_utils_060.c
nodist_rpt_utils_060_SOURCES = $(REMOTE_SOURCES)
rpt_utils_061_SOURCES = rpt_utils_061.c
nodist_rpt_utils_061_SOURCES = $(REMOTE_SOURCES)
rpt_utils_062_SOURCES = rpt_utils_062.c
nodist_rpt_utils_062_SOURCES = $(REMOTE_SOURCES)
rpt_utils_063_SOURCES = rpt_utils_063.c
nodist_rpt_utils_063_SOURCES = $(REMOTE_SOURCES)
rpt_utils_064_SOURCES = rpt_utils_064.c
nodist_rpt_utils_064_SOURCES = $(REMOTE_SOURCES)
rpt_utils_065_SOURCES = rpt_utils_065.c
nodist_rpt_utils_065_SOURCES = $(REMOTE_SOURCES)
rpt_utils_066_SOURCES = rpt_utils_066.c
nodist_rpt_utils_066_SOURCES = $(REMOTE_SOURCES)
rpt_utils_067_SOURCES = rpt_utils_067.c
nodist_rpt_utils_067_SOURCES = $(REMOTE_SOURCES)
rpt_utils_068_SOURCES = rpt_utils_068.c
nodist_rpt_utils_068_SOURCES = $(REMOTE_SOURCES)
rpt_utils_069_SOURCES = rpt_utils_069.c
nodist_rpt_utils_069_SOURCES = $(REMOTE_SOURCES)
rpt_utils_070_SOURCES = rpt_utils_070.c
nodist_rpt_utils_070_SOURCES = $(REMOTE_SOURCES)
rpt_utils_071_SOURCES = rpt_utils_071.c
nodist_rpt_utils_071_SOURCES = $(REMOTE_SOURCES)
rpt_utils_072_SOURCES = rpt_utils_072.c
nodist_rpt_utils_072_SOURCES = $(REMOTE_SOURCES)
rpt_utils_073_SOURCES = rpt_utils_073.c
nodist_rpt_utils_073_SOURCES = $(REMOTE_SOURCES)
rpt_utils_074_SOURCES = rpt_utils_074.c
nodist_rpt_utils_074_SOURCES = $(REMOTE_SOURCES)
rpt_utils_075_SOURCES = rpt_utils_075.c
nodist_rpt_utils_075_SOURCES = $(REMOTE_SOURCES)
rpt_utils_076_SOURCES = rpt_utils_076.c
nodist_rpt_utils_076_SOURCES = $(REMOTE_SOURCES)
rpt_utils_077_SOURCES = rpt_utils_077.c
nodist_rpt_utils_077_SOURCES = $(REMOTE_SOURCES)
rpt_utils_078_SOURCES = rpt_utils_078.c
nodist_rpt_utils_078_SOURCES = $(REMOTE_SOURCES)
rpt_utils_079_SOURCES = rpt_utils_079.c
nodist_rpt_utils_079_SOURCES = $(REMOTE_SOURCES)
rpt_utils_080_SOURCES = rpt_utils_080.c
nodist_rpt_utils_080_SOURCES = $(REMOTE_SOURCES)
rpt_utils_081_SOURCES = rpt_utils_081.c
nodist_rpt_utils_081_SOURCES = $(REMOTE_SOURCES)
rpt_utils_082_SOURCES = rpt_utils_082.c
nodist_rpt_utils_082_SOURCES = $(REMOTE_SOURCES)
rpt_utils_1000_SOURCES = rpt_utils_1000.c
nodist_rpt_utils_1000_SOURCES = $(REMOTE_SOURCES)
