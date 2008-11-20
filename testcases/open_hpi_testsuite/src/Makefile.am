#
# Copyright (c) 2003, Intel Corporation
# (C) Copyright IBM Corp 2003-2006
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
#


MAINTAINERCLEANFILES    = Makefile.in
DISTCLEANFILES		= plugin_static.c
MOSTLYCLEANFILES        = @TEST_CLEAN@

EXTRA_DIST              = plugin_static.c.in

INCLUDES                = @OPENHPI_INCLUDES@ @OH_SSL_INCLUDES@

SUBDIRS                 = t

noinst_LTLIBRARIES     = libopenhpi.la

libopenhpi_la_SOURCES   = \
                           config.c \
                           domain.c \
                           event.c \
			   threaded.c \
                           alarm.c \
                           hotswap.c \
                           lock.c \
                           plugin.c \
                           init.c \
                           safhpi.c \
                           session.c \
                           ohpi.c
nodist_libopenhpi_la_SOURCES = plugin_static.c

libopenhpi_la_LIBADD    = @STATIC_PLUGIN_LIBS@ @STATIC_PLUGIN_EXTRA_LIBS@ $(top_builddir)/utils/libopenhpiutils.la
if HAVE_OPENSSL
libopenhpi_la_LIBADD += $(top_builddir)/$(SSLDIR)/libopenhpi_ssl.la
endif
libopenhpi_la_LDFLAGS   = -L$(top_builddir)/utils -version-info @HPI_LIB_VERSION@ -export-dynamic -static
libopenhpi_la_DEPENDENCIES = @STATIC_PLUGIN_LIBS@

