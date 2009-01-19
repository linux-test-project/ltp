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

MAINTAINERCLEANFILES    = Makefile.in

INCLUDES                = @OPENHPI_INCLUDES@ @XML2_INCLUDE@ @OH_SSL_INCLUDES@

pkglib_LTLIBRARIES      = liboa_soap.la

EXTRA_DIST = \
             OpenHPI_Blade_DevGuide.pdf



liboa_soap_la_SOURCES   = oa_soap.h \
                          oa_soap.c \
                          oa_soap_power.h \
                          oa_soap_power.c \
                          oa_soap_reset.h \
                          oa_soap_reset.c \
                          oa_soap_hotswap.h \
                          oa_soap_hotswap.c \
                          oa_soap_utils.h \
                          oa_soap_utils.c \
                          oa_soap_resources.h \
                          oa_soap_resources.c \
                          oa_soap_discover.h \
                          oa_soap_discover.c \
                          oa_soap_re_discover.h \
                          oa_soap_re_discover.c \
                          oa_soap_event.h \
                          oa_soap_event.c \
                          oa_soap_enclosure_event.h \
                          oa_soap_enclosure_event.c \
                          oa_soap_oa_event.h \
                          oa_soap_oa_event.c \
                          oa_soap_ps_event.h \
                          oa_soap_ps_event.c \
                          oa_soap_fan_event.h \
                          oa_soap_fan_event.c \
                          oa_soap_lcd_event.h \
                          oa_soap_lcd_event.c \
                          oa_soap_server_event.h \
                          oa_soap_server_event.c \
                          oa_soap_interconnect_event.h \
                          oa_soap_interconnect_event.c \
                          oa_soap_control.h \
                          oa_soap_control.c \
                          oa_soap_sensor.h \
                          oa_soap_sensor.c \
                          oa_soap_inventory.h \
                          oa_soap_inventory.c \
                          oa_soap_watchdog.h \
                          oa_soap_watchdog.c \
                          oa_soap_sel.h \
                          oa_soap_sel.c \
                          oa_soap_annunciator.h \
                          oa_soap_annunciator.c \
                          oa_soap_dimi.h \
                          oa_soap_dimi.c \
                          oa_soap_fumi.h \
                          oa_soap_fumi.c \
                          oa_soap_load_id.h \
                          oa_soap_load_id.c \
                          oa_soap_calls.h \
                          oa_soap_calls.c \
                          oa_soap_callsupport.h \
                          oa_soap_callsupport.c

# TODO: What about -luuid in the ilo2_ribcl?
liboa_soap_la_LIBADD    = @SSL_LIB@ @XML2_LIB@ \
                          $(top_builddir)/utils/libopenhpiutils.la
liboa_soap_la_LDFLAGS   = -module -version-info @HPI_LIB_VERSION@
