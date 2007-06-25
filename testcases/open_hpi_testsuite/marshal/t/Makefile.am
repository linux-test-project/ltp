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

AM_CFLAGS		= @CFLAGS@ @COV_CFLAGS@

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

marshal_000_SOURCES = marshal_000.c $(REMOTE_SOURCES)
marshal_001_SOURCES = marshal_001.c $(REMOTE_SOURCES)
marshal_002_SOURCES = marshal_002.c $(REMOTE_SOURCES)
marshal_003_SOURCES = marshal_003.c $(REMOTE_SOURCES)
marshal_004_SOURCES = marshal_004.c $(REMOTE_SOURCES)
marshal_005_SOURCES = marshal_005.c $(REMOTE_SOURCES)
marshal_006_SOURCES = marshal_006.c $(REMOTE_SOURCES)
marshal_007_SOURCES = marshal_007.c $(REMOTE_SOURCES)
marshal_008_SOURCES = marshal_008.c $(REMOTE_SOURCES)
marshal_009_SOURCES = marshal_009.c $(REMOTE_SOURCES)
marshal_010_SOURCES = marshal_010.c $(REMOTE_SOURCES)
marshal_011_SOURCES = marshal_011.c $(REMOTE_SOURCES)
marshal_012_SOURCES = marshal_012.c $(REMOTE_SOURCES)
marshal_013_SOURCES = marshal_013.c $(REMOTE_SOURCES)
marshal_014_SOURCES = marshal_014.c $(REMOTE_SOURCES)
marshal_015_SOURCES = marshal_015.c $(REMOTE_SOURCES)
marshal_016_SOURCES = marshal_016.c $(REMOTE_SOURCES)
marshal_017_SOURCES = marshal_017.c $(REMOTE_SOURCES)
marshal_018_SOURCES = marshal_018.c $(REMOTE_SOURCES)
marshal_019_SOURCES = marshal_019.c $(REMOTE_SOURCES)
marshal_020_SOURCES = marshal_020.c $(REMOTE_SOURCES)
marshal_021_SOURCES = marshal_021.c $(REMOTE_SOURCES)
marshal_022_SOURCES = marshal_022.c $(REMOTE_SOURCES)
marshal_023_SOURCES = marshal_023.c $(REMOTE_SOURCES)
marshal_024_SOURCES = marshal_024.c $(REMOTE_SOURCES)
marshal_025_SOURCES = marshal_025.c $(REMOTE_SOURCES)
marshal_026_SOURCES = marshal_026.c $(REMOTE_SOURCES)
marshal_027_SOURCES = marshal_027.c $(REMOTE_SOURCES)
marshal_028_SOURCES = marshal_028.c $(REMOTE_SOURCES)
marshal_029_SOURCES = marshal_029.c $(REMOTE_SOURCES)
marshal_hpi_types_000_SOURCES = marshal_hpi_types_000.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_001_SOURCES = marshal_hpi_types_001.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_002_SOURCES = marshal_hpi_types_002.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_003_SOURCES = marshal_hpi_types_003.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_004_SOURCES = marshal_hpi_types_004.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_005_SOURCES = marshal_hpi_types_005.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_006_SOURCES = marshal_hpi_types_006.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_007_SOURCES = marshal_hpi_types_007.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_008_SOURCES = marshal_hpi_types_008.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_009_SOURCES = marshal_hpi_types_009.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_010_SOURCES = marshal_hpi_types_010.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_011_SOURCES = marshal_hpi_types_011.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_012_SOURCES = marshal_hpi_types_012.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_013_SOURCES = marshal_hpi_types_013.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_014_SOURCES = marshal_hpi_types_014.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_015_SOURCES = marshal_hpi_types_015.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_016_SOURCES = marshal_hpi_types_016.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_017_SOURCES = marshal_hpi_types_017.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_018_SOURCES = marshal_hpi_types_018.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_019_SOURCES = marshal_hpi_types_019.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_020_SOURCES = marshal_hpi_types_020.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_021_SOURCES = marshal_hpi_types_021.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_022_SOURCES = marshal_hpi_types_022.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_023_SOURCES = marshal_hpi_types_023.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_024_SOURCES = marshal_hpi_types_024.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_025_SOURCES = marshal_hpi_types_025.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_026_SOURCES = marshal_hpi_types_026.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_027_SOURCES = marshal_hpi_types_027.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_028_SOURCES = marshal_hpi_types_028.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_029_SOURCES = marshal_hpi_types_029.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_030_SOURCES = marshal_hpi_types_030.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_031_SOURCES = marshal_hpi_types_031.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_032_SOURCES = marshal_hpi_types_032.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_033_SOURCES = marshal_hpi_types_033.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_034_SOURCES = marshal_hpi_types_034.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_035_SOURCES = marshal_hpi_types_035.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_036_SOURCES = marshal_hpi_types_036.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_037_SOURCES = marshal_hpi_types_037.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_038_SOURCES = marshal_hpi_types_038.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_039_SOURCES = marshal_hpi_types_039.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_040_SOURCES = marshal_hpi_types_040.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_041_SOURCES = marshal_hpi_types_041.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_042_SOURCES = marshal_hpi_types_042.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_043_SOURCES = marshal_hpi_types_043.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_044_SOURCES = marshal_hpi_types_044.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_045_SOURCES = marshal_hpi_types_045.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_046_SOURCES = marshal_hpi_types_046.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_047_SOURCES = marshal_hpi_types_047.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
marshal_hpi_types_048_SOURCES = marshal_hpi_types_048.c $(MARSHAL_SOURCES) $(REMOTE_SOURCES)
