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
 
MAINTAINERCLEANFILES 	= Makefile.in

EXTRA_DIST 		= ipmi.sym ekeyfru.h

SUBDIRS			= t

INCLUDES		= @OPENHPI_INCLUDES@

AM_CFLAGS		= @OPENIPMI_CFLAGS@

pkglib_LTLIBRARIES	= libipmi.la

libipmi_la_SOURCES	= ipmi.c 		\
			  ipmi_close.c		\
			  ipmi_connection.c	\
			  ipmi_event.c 		\
			  ipmi_entity_event.c	\
			  ipmi_mc_event.c	\
			  ipmi_inventory_event.c\
			  ipmi_sensor_event.c	\
			  ipmi_control_event.c	\
			  ipmi_entity.c 	\
			  ipmi_sensor.c 	\
			  ipmi_sel.c 		\
			  ipmi_inventory.c	\
			  ipmi_controls.c	\
			  ipmi_util.c 		\
			  ipmi_drv.c 		\
			  hotswap.c		\
			  sync.c 		\
			  posix_vlog.c		\
			  atca_shelf_rdrs.c	\
			  atca_slot_rdrs.c	\
			  atca_vshm_rdrs.c	\
			  atca_fru_rdrs.c	\
			  atca_shelf_fru.c	\
			  ipmi.h
			  
libipmi_la_LIBADD	= $(top_builddir)/utils/libopenhpiutils.la -luuid -lncurses -lOpenIPMI -lOpenIPMIposix @OPENIPMI_LIBS@
#libipmi_la_LDFLAGS	= -module -version-info @HPI_LIB_VERSION@ -export-symbols $(srcdir)/ipmi.sym
libipmi_la_LDFLAGS	= -module -version-info @HPI_LIB_VERSION@ -export-dymanic $(srcdir)/ipmi.sym
