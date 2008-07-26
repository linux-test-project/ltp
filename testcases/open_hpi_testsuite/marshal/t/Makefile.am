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

MARSHAL_SRCDIR = $(top_srcdir)/marshal

REMOTE_SOURCES		= connection.c marshal.c
MARSHAL_SOURCES         = marshal_hpi_types.c

MOSTLYCLEANFILES 	= $(REMOTE_SOURCES) $(MARSHAL_SOURCES) @TEST_CLEAN@

MAINTAINERCLEANFILES 	= Makefile.in *~

INCLUDES		= -I $(MARSHAL_SRCDIR) @OPENHPI_INCLUDES@

noinst_PROGRAMS = float_format
float_format_SOURCES = float_format.c

CLEANFILES=float32.bin float64.bin *~


$(REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		ln -s $(MARSHAL_SRCDIR)/$@; \
	fi

$(MARSHAL_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		ln -s $(MARSHAL_SRCDIR)/$@; \
	fi

TESTS = \
       marshal_000 \
       marshal_001 \
       marshal_002 \
       marshal_003 \
       marshal_004 \
       marshal_005 \
       marshal_006 \
       marshal_007 \
       marshal_008 \
       marshal_009 \
       marshal_010 \
       marshal_011 \
       marshal_012 \
       marshal_013 \
       marshal_014 \
       marshal_015 \
       marshal_016 \
       marshal_017 \
       marshal_018 \
       marshal_019 \
       marshal_020 \
       marshal_021 \
       marshal_022 \
       marshal_023 \
       marshal_024 \
       marshal_025 \
       marshal_026 \
       marshal_027 \
       marshal_028 \
       marshal_029 \
       marshal_030 \
       marshal_031 \
       marshal_hpi_types_000 \
       marshal_hpi_types_001 \
       marshal_hpi_types_002 \
       marshal_hpi_types_003 \
       marshal_hpi_types_004 \
       marshal_hpi_types_005 \
       marshal_hpi_types_006 \
       marshal_hpi_types_007 \
       marshal_hpi_types_008 \
       marshal_hpi_types_009 \
       marshal_hpi_types_010 \
       marshal_hpi_types_011 \
       marshal_hpi_types_012 \
       marshal_hpi_types_013 \
       marshal_hpi_types_014 \
       marshal_hpi_types_015 \
       marshal_hpi_types_016 \
       marshal_hpi_types_017 \
       marshal_hpi_types_018 \
       marshal_hpi_types_019 \
       marshal_hpi_types_020 \
       marshal_hpi_types_021 \
       marshal_hpi_types_022 \
       marshal_hpi_types_023 \
       marshal_hpi_types_024 \
       marshal_hpi_types_025 \
       marshal_hpi_types_026 \
       marshal_hpi_types_027 \
       marshal_hpi_types_028 \
       marshal_hpi_types_029 \
       marshal_hpi_types_030 \
       marshal_hpi_types_031 \
       marshal_hpi_types_032 \
       marshal_hpi_types_033 \
       marshal_hpi_types_034 \
       marshal_hpi_types_035 \
       marshal_hpi_types_036 \
       marshal_hpi_types_037 \
       marshal_hpi_types_038 \
       marshal_hpi_types_039 \
       marshal_hpi_types_040 \
       marshal_hpi_types_041 \
       marshal_hpi_types_042 \
       marshal_hpi_types_043 \
       marshal_hpi_types_044 \
       marshal_hpi_types_045 \
       marshal_hpi_types_046 \
       marshal_hpi_types_047 \
       marshal_hpi_types_048
#       connection_seq_000 \
#       connection_000 \
#       connection_001

check_PROGRAMS = $(TESTS)

#connection_000_SOURCES = connection_000.c $(REMOTE_SOURCES)
#connection_seq_000_SOURCES = connection_seq_000.c $(REMOTE_SOURCES)
#connection_001_SOURCES = connection_001.c $(REMOTE_SOURCES)

marshal_000_SOURCES = marshal_000.c
nodist_marshal_000_SOURCES = $(REMOTE_SOURCES)
marshal_001_SOURCES = marshal_001.c
nodist_marshal_001_SOURCES = $(REMOTE_SOURCES)
marshal_002_SOURCES = marshal_002.c
nodist_marshal_002_SOURCES = $(REMOTE_SOURCES)
marshal_003_SOURCES = marshal_003.c
nodist_marshal_003_SOURCES = $(REMOTE_SOURCES)
marshal_004_SOURCES = marshal_004.c
nodist_marshal_004_SOURCES = $(REMOTE_SOURCES)
marshal_005_SOURCES = marshal_005.c
nodist_marshal_005_SOURCES = $(REMOTE_SOURCES)
marshal_006_SOURCES = marshal_006.c
nodist_marshal_006_SOURCES = $(REMOTE_SOURCES)
marshal_007_SOURCES = marshal_007.c
nodist_marshal_007_SOURCES = $(REMOTE_SOURCES)
marshal_008_SOURCES = marshal_008.c
nodist_marshal_008_SOURCES = $(REMOTE_SOURCES)
marshal_009_SOURCES = marshal_009.c
nodist_marshal_009_SOURCES = $(REMOTE_SOURCES)
marshal_010_SOURCES = marshal_010.c
nodist_marshal_010_SOURCES = $(REMOTE_SOURCES)
marshal_011_SOURCES = marshal_011.c
nodist_marshal_011_SOURCES = $(REMOTE_SOURCES)
marshal_012_SOURCES = marshal_012.c
nodist_marshal_012_SOURCES = $(REMOTE_SOURCES)
marshal_013_SOURCES = marshal_013.c
nodist_marshal_013_SOURCES = $(REMOTE_SOURCES)
marshal_014_SOURCES = marshal_014.c
nodist_marshal_014_SOURCES = $(REMOTE_SOURCES)
marshal_015_SOURCES = marshal_015.c
nodist_marshal_015_SOURCES = $(REMOTE_SOURCES)
marshal_016_SOURCES = marshal_016.c
nodist_marshal_016_SOURCES = $(REMOTE_SOURCES)
marshal_017_SOURCES = marshal_017.c
nodist_marshal_017_SOURCES = $(REMOTE_SOURCES)
marshal_018_SOURCES = marshal_018.c
nodist_marshal_018_SOURCES = $(REMOTE_SOURCES)
marshal_019_SOURCES = marshal_019.c
nodist_marshal_019_SOURCES = $(REMOTE_SOURCES)
marshal_020_SOURCES = marshal_020.c
nodist_marshal_020_SOURCES = $(REMOTE_SOURCES)
marshal_021_SOURCES = marshal_021.c
nodist_marshal_021_SOURCES = $(REMOTE_SOURCES)
marshal_022_SOURCES = marshal_022.c
nodist_marshal_022_SOURCES = $(REMOTE_SOURCES)
marshal_023_SOURCES = marshal_023.c
nodist_marshal_023_SOURCES = $(REMOTE_SOURCES)
marshal_024_SOURCES = marshal_024.c
nodist_marshal_024_SOURCES = $(REMOTE_SOURCES)
marshal_025_SOURCES = marshal_025.c
nodist_marshal_025_SOURCES = $(REMOTE_SOURCES)
marshal_026_SOURCES = marshal_026.c
nodist_marshal_026_SOURCES = $(REMOTE_SOURCES)
marshal_027_SOURCES = marshal_027.c
nodist_marshal_027_SOURCES = $(REMOTE_SOURCES)
marshal_028_SOURCES = marshal_028.c
nodist_marshal_028_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_029_SOURCES = marshal_029.c
nodist_marshal_029_SOURCES = $(REMOTE_SOURCES)
marshal_030_SOURCES = marshal_030.c
nodist_marshal_030_SOURCES = $(REMOTE_SOURCES)
marshal_031_SOURCES = marshal_031.c
nodist_marshal_031_SOURCES = $(REMOTE_SOURCES)
marshal_hpi_types_000_SOURCES = marshal_hpi_types_000.c
nodist_marshal_hpi_types_000_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_001_SOURCES = marshal_hpi_types_001.c
nodist_marshal_hpi_types_001_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_002_SOURCES = marshal_hpi_types_002.c
nodist_marshal_hpi_types_002_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_003_SOURCES = marshal_hpi_types_003.c
nodist_marshal_hpi_types_003_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_004_SOURCES = marshal_hpi_types_004.c
nodist_marshal_hpi_types_004_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_005_SOURCES = marshal_hpi_types_005.c
nodist_marshal_hpi_types_005_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_006_SOURCES = marshal_hpi_types_006.c
nodist_marshal_hpi_types_006_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_007_SOURCES = marshal_hpi_types_007.c
nodist_marshal_hpi_types_007_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_008_SOURCES = marshal_hpi_types_008.c
nodist_marshal_hpi_types_008_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_009_SOURCES = marshal_hpi_types_009.c
nodist_marshal_hpi_types_009_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_010_SOURCES = marshal_hpi_types_010.c
nodist_marshal_hpi_types_010_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_011_SOURCES = marshal_hpi_types_011.c
nodist_marshal_hpi_types_011_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_012_SOURCES = marshal_hpi_types_012.c
nodist_marshal_hpi_types_012_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_013_SOURCES = marshal_hpi_types_013.c
nodist_marshal_hpi_types_013_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_014_SOURCES = marshal_hpi_types_014.c
nodist_marshal_hpi_types_014_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_015_SOURCES = marshal_hpi_types_015.c
nodist_marshal_hpi_types_015_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_016_SOURCES = marshal_hpi_types_016.c
nodist_marshal_hpi_types_016_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_017_SOURCES = marshal_hpi_types_017.c
nodist_marshal_hpi_types_017_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_018_SOURCES = marshal_hpi_types_018.c
nodist_marshal_hpi_types_018_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_019_SOURCES = marshal_hpi_types_019.c
nodist_marshal_hpi_types_019_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_020_SOURCES = marshal_hpi_types_020.c
nodist_marshal_hpi_types_020_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_021_SOURCES = marshal_hpi_types_021.c
nodist_marshal_hpi_types_021_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_022_SOURCES = marshal_hpi_types_022.c
nodist_marshal_hpi_types_022_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_023_SOURCES = marshal_hpi_types_023.c
nodist_marshal_hpi_types_023_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_024_SOURCES = marshal_hpi_types_024.c
nodist_marshal_hpi_types_024_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_025_SOURCES = marshal_hpi_types_025.c
nodist_marshal_hpi_types_025_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_026_SOURCES = marshal_hpi_types_026.c
nodist_marshal_hpi_types_026_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_027_SOURCES = marshal_hpi_types_027.c
nodist_marshal_hpi_types_027_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_028_SOURCES = marshal_hpi_types_028.c
nodist_marshal_hpi_types_028_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_029_SOURCES = marshal_hpi_types_029.c
nodist_marshal_hpi_types_029_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_030_SOURCES = marshal_hpi_types_030.c
nodist_marshal_hpi_types_030_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_031_SOURCES = marshal_hpi_types_031.c
nodist_marshal_hpi_types_031_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_032_SOURCES = marshal_hpi_types_032.c
nodist_marshal_hpi_types_032_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_033_SOURCES = marshal_hpi_types_033.c
nodist_marshal_hpi_types_033_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_034_SOURCES = marshal_hpi_types_034.c
nodist_marshal_hpi_types_034_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_035_SOURCES = marshal_hpi_types_035.c
nodist_marshal_hpi_types_035_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_036_SOURCES = marshal_hpi_types_036.c
nodist_marshal_hpi_types_036_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_037_SOURCES = marshal_hpi_types_037.c
nodist_marshal_hpi_types_037_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_038_SOURCES = marshal_hpi_types_038.c
nodist_marshal_hpi_types_038_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_039_SOURCES = marshal_hpi_types_039.c
nodist_marshal_hpi_types_039_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_040_SOURCES = marshal_hpi_types_040.c
nodist_marshal_hpi_types_040_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_041_SOURCES = marshal_hpi_types_041.c
nodist_marshal_hpi_types_041_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_042_SOURCES = marshal_hpi_types_042.c
nodist_marshal_hpi_types_042_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_043_SOURCES = marshal_hpi_types_043.c
nodist_marshal_hpi_types_043_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_044_SOURCES = marshal_hpi_types_044.c
nodist_marshal_hpi_types_044_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_045_SOURCES = marshal_hpi_types_045.c
nodist_marshal_hpi_types_045_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_046_SOURCES = marshal_hpi_types_046.c
nodist_marshal_hpi_types_046_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_047_SOURCES = marshal_hpi_types_047.c
nodist_marshal_hpi_types_047_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_048_SOURCES = marshal_hpi_types_048.c
nodist_marshal_hpi_types_048_SOURCES = $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
