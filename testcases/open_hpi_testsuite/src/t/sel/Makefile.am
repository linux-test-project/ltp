# (C) Copyright IBM Corp 2003, 2004
# All rights reserved.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.

REMOTE_SOURCES		= sel_utils.c

MOSTLYCLEANFILES 	= $(REMOTE_SOURCES) $(shell ls *.bbg) $(shell ls *.da)
DISTCLEANFILES		= $(top_builddir)/src/t/sel/selTest.data

INCLUDES		= -I$(top_srcdir)/include

AM_CFLAGS		= @CFLAGS@ -ftest-coverage -fprofile-arcs

$(REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		ln -s ../../$@; \
	fi

TESTS = sel_test_001 \
	sel_test_002 \
	sel_test_003 \
	sel_test_004

check_PROGRAMS = sel_test_001 \
		 sel_test_002 \
		 sel_test_003 \
		 sel_test_004

sel_test_001_SOURCES = sel_test.h sel_test_001.c $(REMOTE_SOURCES)
sel_test_002_SOURCES = sel_test.h sel_test_002.c addsel.c $(REMOTE_SOURCES)
sel_test_003_SOURCES = sel_test.h sel_test_003.c addsel.c $(REMOTE_SOURCES)
sel_test_004_SOURCES = sel_test.h sel_test_004.c $(REMOTE_SOURCES)

