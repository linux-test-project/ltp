#
# Copyright (c) 2003, Intel Corporation
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

MAINTAINERCLEANFILES    = Makefile.in *~ core core.*

INCLUDES                = @OPENHPI_INCLUDES@ 

SUBDIRS                 = t
DIST_SUBDIRS            = t

pkglib_LTLIBRARIES      = libipmidirect.la

libipmidirect_la_SOURCES= \
		array.h \
		hotswap.cpp \
		ipmi.h \
		ipmi.cpp \
		ipmi_addr.h \
		ipmi_addr.cpp \
		ipmi_auth.h \
		ipmi_auth.cpp \
		ipmi_cmd.h \
		ipmi_cmd.cpp \
		ipmi_con.h \
		ipmi_con.cpp \
		ipmi_con_lan.h \
		ipmi_con_lan.cpp \
		ipmi_con_smi.h \
		ipmi_con_smi.cpp \
		ipmi_control.h \
		ipmi_control.cpp \
		ipmi_control_atca_led.h \
		ipmi_control_atca_led.cpp \
		ipmi_control_fan.h \
		ipmi_control_fan.cpp \
		ipmi_control_sun_led.h \
		ipmi_control_sun_led.cpp \
		ipmi_discover.h \
		ipmi_discover.cpp \
		ipmi_domain.h \
		ipmi_domain.cpp \
		ipmi_entity.h \
		ipmi_entity.cpp \
		ipmi_event.h \
		ipmi_event.cpp \
		ipmi_fru_info.h \
		ipmi_fru_info.cpp \
		ipmi_inventory.h \
		ipmi_inventory.cpp \
		ipmi_inventory_parser.h \
		ipmi_inventory_parser.cpp \
		ipmi_log.h \
		ipmi_log.cpp \
		ipmi_mc.h \
		ipmi_mc.cpp \
		ipmi_mc_vendor.h \
		ipmi_mc_vendor.cpp \
		ipmi_mc_vendor_force.h \
		ipmi_mc_vendor_force.cpp \
		ipmi_mc_vendor_intel.h \
		ipmi_mc_vendor_intel.cpp \
		ipmi_mc_vendor_sun.h \
		ipmi_mc_vendor_sun.cpp \
		ipmi_mc_vendor_fix_sdr.h \
		ipmi_mc_vendor_fix_sdr.cpp \
		ipmi_msg.h \
		ipmi_msg.cpp \
		ipmi_rdr.h \
		ipmi_rdr.cpp \
		ipmi_resource.h \
		ipmi_resource.cpp \
		ipmi_sdr.h \
		ipmi_sdr.cpp \
		ipmi_sel.h \
		ipmi_sel.cpp \
		ipmi_sensor.h \
		ipmi_sensor.cpp \
		ipmi_sensor_discrete.h \
		ipmi_sensor_discrete.cpp \
		ipmi_sensor_factors.h \
		ipmi_sensor_factors.cpp \
		ipmi_sensor_hotswap.h \
		ipmi_sensor_hotswap.cpp \
		ipmi_sensor_threshold.h \
		ipmi_sensor_threshold.cpp \
		ipmi_text_buffer.h \
		ipmi_text_buffer.cpp \
		ipmi_utils.h \
		ipmi_utils.cpp \
		ipmi_watchdog.h \
		ipmi_watchdog.cpp \
		thread.h \
		thread.cpp

libipmidirect_la_LIBADD	= @CRYPTO_LIB@ -luuid -lm -lstdc++ $(top_builddir)/utils/libopenhpiutils.la
libipmidirect_la_LDFLAGS= -module -version-info @HPI_LIB_VERSION@

clean-local:
	rm -f *~
