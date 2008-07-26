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

REMOTE_SOURCES =  epath_utils.c \
		  sahpi_enum_utils.c \
		  sahpiatca_enum_utils.c \
		  sahpi_event_encode.c \
		  sahpi_event_utils.c \
	          sahpi_struct_utils.c \
	          sahpi_time_utils.c \
		  uid_utils.c

MOSTLYCLEANFILES = $(REMOTE_SOURCES) @TEST_CLEAN@
INCLUDES	= @OPENHPI_INCLUDES@
AM_CFLAGS	= -std=c99
# the -std=c99 comment above is required to have unicode in the tests

LIBS += -luuid

$(REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		if test -f $(top_srcdir)/utils/$@; then \
			$(LN_S) $(top_srcdir)/utils/$@; \
		else \
			$(LN_S) $(top_builddir)/utils/$@; \
		fi; \
	fi

# FIXME:: Remove epath_enum_utils_test and sahpi_event_encode_test files
#         when we can auto-generate file for SMP systems.
#         Create rule using SaHpi2code.pl using -tdir option
TESTS = sahpi_enum_utils_test \
	sahpiatca_enum_utils_test \
	sahpi_event_encode_test \
	sahpi_event_utils_test \
	sahpi_time_utils_test \
	sahpi_struct_idrinfo_test \
	sahpi_struct_idrarea_test \
	sahpi_struct_idrfield_test \
	sahpi_struct_rptentry_test \
	sahpi_struct_rdr_test \
	sahpi_struct_utils_test \
	sahpi_text_utils_test

check_PROGRAMS = $(TESTS)

sahpi_enum_utils_test_SOURCES = sahpi_enum_utils_test.c 
nodist_sahpi_enum_utils_test_SOURCES = $(REMOTE_SOURCES)
sahpiatca_enum_utils_test_SOURCES = sahpiatca_enum_utils_test.c 
nodist_sahpiatca_enum_utils_test_SOURCES = $(REMOTE_SOURCES)
sahpi_event_encode_test_SOURCES = sahpi_event_encode_test.c 
nodist_sahpi_event_encode_test_SOURCES = $(REMOTE_SOURCES)
sahpi_event_utils_test_SOURCES = sahpi_event_utils_test.c 
nodist_sahpi_event_utils_test_SOURCES = $(REMOTE_SOURCES)
sahpi_struct_utils_test_SOURCES = sahpi_struct_utils_test.c
nodist_sahpi_struct_utils_test_SOURCES = $(REMOTE_SOURCES)
sahpi_text_utils_test_SOURCES = sahpi_text_utils_test.c
nodist_sahpi_text_utils_test_SOURCES = $(REMOTE_SOURCES)
sahpi_time_utils_test_SOURCES = sahpi_time_utils_test.c
nodist_sahpi_time_utils_test_SOURCES = $(REMOTE_SOURCES)
sahpi_struct_idrinfo_test_SOURCES = sahpi_struct_idrinfo_test.c
nodist_sahpi_struct_idrinfo_test_SOURCES = $(REMOTE_SOURCES)
sahpi_struct_idrarea_test_SOURCES = sahpi_struct_idrarea_test.c
nodist_sahpi_struct_idrarea_test_SOURCES = $(REMOTE_SOURCES)
sahpi_struct_idrfield_test_SOURCES = sahpi_struct_idrfield_test.c
nodist_sahpi_struct_idrfield_test_SOURCES = $(REMOTE_SOURCES)
sahpi_struct_rptentry_test_SOURCES = sahpi_struct_rptentry_test.c
nodist_sahpi_struct_rptentry_test_SOURCES = $(REMOTE_SOURCES)
sahpi_struct_rdr_test_SOURCES = sahpi_struct_rdr_test.c
nodist_sahpi_struct_rdr_test_SOURCES = $(REMOTE_SOURCES)

