# (C) Copyright IBM Corp 2005
# All rights reserved.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.

MAINTAINERCLEANFILES = Makefile.in

REMOTE_SOURCES       = announcement_utils.c

MOSTLYCLEANFILES     = $(shell ls *data) $(REMOTE_SOURCES) @TEST_CLEAN@

INCLUDES             = @OPENHPI_INCLUDES@

$(REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
                $(LN_S) $(top_srcdir)/utils/$@; \
        fi

TESTS = ann_test_001 \
        ann_test_002 \
        ann_test_003 \
        ann_test_004 \
        ann_test_005 \
        ann_test_006 \
        ann_test_007 \
        ann_test_008


check_PROGRAMS = $(TESTS)

ann_test_001_SOURCES = ann_test_001.c
nodist_ann_test_001_SOURCES = $(REMOTE_SOURCES)
ann_test_002_SOURCES = ann_test_002.c
nodist_ann_test_002_SOURCES = $(REMOTE_SOURCES)
ann_test_003_SOURCES = ann_test_003.c
nodist_ann_test_003_SOURCES = $(REMOTE_SOURCES)
ann_test_004_SOURCES = ann_test_004.c
nodist_ann_test_004_SOURCES = $(REMOTE_SOURCES)
ann_test_005_SOURCES = ann_test_005.c
nodist_ann_test_005_SOURCES = $(REMOTE_SOURCES)
ann_test_006_SOURCES = ann_test_006.c
nodist_ann_test_006_SOURCES = $(REMOTE_SOURCES)
ann_test_007_SOURCES = ann_test_007.c
nodist_ann_test_007_SOURCES = $(REMOTE_SOURCES)
ann_test_008_SOURCES = ann_test_008.c
nodist_ann_test_008_SOURCES = $(REMOTE_SOURCES)

