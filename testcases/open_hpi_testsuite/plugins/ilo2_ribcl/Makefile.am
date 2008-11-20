#
# Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
#                     All rights reserved.
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
# Neither the name of the Hewlett-Packard Corporation, nor the names
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
# Author(s)
#     Shuah Khan <shuah.khan@hp.com>
#     Richard White <richard.white@hp.com>
#

MAINTAINERCLEANFILES 	= Makefile.in

INCLUDES		= @OPENHPI_INCLUDES@ @XML2_INCLUDE@ @OH_SSL_INCLUDES@

pkglib_LTLIBRARIES	= libilo2_ribcl.la

EXTRA_DIST = \
             OpenHPI_ProLiant_DevGuide.pdf


libilo2_ribcl_la_SOURCES  =	\
				ilo2_ribcl.c \
				ilo2_ribcl.h \
				ilo2_ribcl_discover.c \
				ilo2_ribcl_discover.h \
				ilo2_ribcl_idr.c \
				ilo2_ribcl_idr.h \
				ilo2_ribcl_reset.c \
				ilo2_ribcl_power.c \
				ilo2_ribcl_control.c \
				ilo2_ribcl_rpt.c \
				ilo2_ribcl_sensor.c \
				ilo2_ribcl_sensor.h \
				ilo2_ribcl_ssl.c \
				ilo2_ribcl_ssl.h \
				ilo2_ribcl_xml.c \
				ilo2_ribcl_xml.h \
				ilo2_ribcl_cmnds.h

libilo2_ribcl_la_LIBADD   = @SSL_LIB@ @XML2_LIB@ -luuid $(top_builddir)/utils/libopenhpiutils.la
libilo2_ribcl_la_LDFLAGS  = -module -version-info @HPI_LIB_VERSION@

