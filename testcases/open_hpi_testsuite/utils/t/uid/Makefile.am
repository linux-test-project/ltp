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

REMOTE_SOURCES		= uid_utils.c \
			  epath_utils.c \
			  sahpi_enum_utils.c \
			  sahpiatca_enum_utils.c \
			  sahpi_event_encode.c \
			  sahpi_event_utils.c \
			  sahpi_struct_utils.c \
			  sahpi_time_utils.c

MOSTLYCLEANFILES 	= @TEST_CLEAN@ \
	                  $(REMOTE_SOURCES) \
			  uid_map

INCLUDES		= @OPENHPI_INCLUDES@

LIBS += -luuid

$(REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		$(LN_S) $(top_srcdir)/utils/$@; \
	fi

TESTS_ENVIRONMENT = OPENHPI_UID_MAP=$(shell pwd)/uid_map
TESTS = uid_utils_000 \
	uid_utils_001 \
	uid_utils_002 \
	uid_utils_003 \
	uid_utils_004 \
	uid_utils_005 \
        uid_utils_006 \
        uid_utils_007 \
        uid_utils_008 \
        uid_utils_009 \
        uid_utils_010 \
        uid_utils_011 \
        uid_utils_012 \
        uid_utils_013

check_PROGRAMS = $(TESTS)

uid_utils_000_SOURCES = uid_utils_000.c
nodist_uid_utils_000_SOURCES = $(REMOTE_SOURCES)
uid_utils_001_SOURCES = uid_utils_001.c
nodist_uid_utils_001_SOURCES = $(REMOTE_SOURCES)
uid_utils_002_SOURCES = uid_utils_002.c
nodist_uid_utils_002_SOURCES = $(REMOTE_SOURCES)
uid_utils_003_SOURCES = uid_utils_003.c
nodist_uid_utils_003_SOURCES = $(REMOTE_SOURCES)
uid_utils_004_SOURCES = uid_utils_004.c
nodist_uid_utils_004_SOURCES = $(REMOTE_SOURCES)
uid_utils_005_SOURCES = uid_utils_005.c
nodist_uid_utils_005_SOURCES = $(REMOTE_SOURCES)
uid_utils_006_SOURCES = uid_utils_006.c
nodist_uid_utils_006_SOURCES = $(REMOTE_SOURCES)
uid_utils_007_SOURCES = uid_utils_007.c
nodist_uid_utils_007_SOURCES = $(REMOTE_SOURCES)
uid_utils_008_SOURCES = uid_utils_008.c
nodist_uid_utils_008_SOURCES = $(REMOTE_SOURCES)
uid_utils_009_SOURCES = uid_utils_009.c
nodist_uid_utils_009_SOURCES = $(REMOTE_SOURCES)
uid_utils_010_SOURCES = uid_utils_010.c
nodist_uid_utils_010_SOURCES = $(REMOTE_SOURCES)
uid_utils_011_SOURCES = uid_utils_011.c
nodist_uid_utils_011_SOURCES = $(REMOTE_SOURCES)
uid_utils_012_SOURCES = uid_utils_012.c
nodist_uid_utils_012_SOURCES = $(REMOTE_SOURCES)
uid_utils_013_SOURCES = uid_utils_013.c
nodist_uid_utils_013_SOURCES = $(REMOTE_SOURCES)
