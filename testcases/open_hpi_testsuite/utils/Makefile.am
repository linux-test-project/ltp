#
# Copyright (c) 2003 Intel Corporation
# (C) Copyright IBM Corp 2003, 2004
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or
# without modification, are permitted provided that the following
# conditions are met:
#
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the distribution.
#
# Neither the name of Intel Corporation nor the names
# of its contributors may be used to endorse or promote products
# derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

MAINTAINERCLEANFILES = Makefile.in

GENERATED_HEADER_FILES = \
			sahpi_enum_utils.h \
			sahpiatca_enum_utils.h \
			sahpi_event_encode.h

GENERATED_SRC_FILES = \
			sahpi_enum_utils.c \
			sahpiatca_enum_utils.c \
			sahpi_event_encode.c

GENERATED_FILES = $(GENERATED_HEADER_FILES) $(GENERATED_SRC_FILES)

EXTRA_DIST = \
		$(GENERATED_HEADER_FILES) \
		announcement_utils.h \
		el_utils.h \
		epath_utils.h \
		oh_utils.h \
		rpt_utils.h \
		sahpi_event_utils.h \
		sahpi_struct_utils.h \
		sahpi_time_utils.h \
		uid_utils.h

MOSTLYCLEANFILES = @TEST_CLEAN@
# FIXME:: Add when we can auto-generate files for SMP systems
# MOSTLYCLEANFILES += $(GENERATED_FILES)

INCLUDES = @OPENHPI_INCLUDES@

SUBDIRS = t
DIST_SUBDIRS = t

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = openhpiutils.pc

lib_LTLIBRARIES = libopenhpiutils.la

# Generated files (e.g.sahpi_enum_utils.c) must be listed first
libopenhpiutils_la_SOURCES = \
			$(GENERATED_SRC_FILES) \
			announcement_utils.c \
			el_utils.c \
			epath_utils.c \
			rpt_utils.c \
			sahpi_event_utils.c \
			sahpi_struct_utils.c \
			sahpi_time_utils.c \
			uid_utils.c

libopenhpiutils_la_LDFLAGS = -version-info @HPI_LIB_VERSION@
libopenhpiutils_la_LIBADD = -luuid 

# FIXME:: Add when we can auto-generate files for SMP systems
#$(GENERATED_FILES): $(top_srcdir)/include/SaHpi.h $(top_srcdir)/scripts/SaHpi2code.pl
#	$(top_srcdir)/scripts/SaHpi2code.pl -ifile $(top_srcdir)/include/SaHpi.h
