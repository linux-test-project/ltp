#      -*- linux-c -*-
#
# (C) Copyright IBM Corp. 2005
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
#
# Author(s):
#        Renier Morales <renier@openhpi.org>
#

MAINTAINERCLEANFILES 	= Makefile.in

INCLUDES		= @OPENHPI_INCLUDES@ 

pkglib_LTLIBRARIES	= librtas2hpi.la 

librtas2hpi_la_SOURCES	=  rtas.h rtas.c \
			   rtas_annunciator.h rtas_annunciator.c \
			   rtas_discover.h rtas_discover.c \
			   rtas_control.h rtas_control.c \
			   rtas_event.h rtas_event.c \
			   rtas_eventlog.h rtas_eventlog.c \
			   rtas_hotswap.h rtas_hotswap.c \
			   rtas_indicator.h rtas_indicator.c \
			   rtas_inventory.h rtas_inventory.c \
			   rtas_power.h rtas_power.c \
			   rtas_reset.h rtas_reset.c \
			   rtas_resource.h rtas_resource.c \
			   rtas_sensor.h rtas_sensor.c \
			   rtas_utils.h rtas_utils.c \
			   rtas_watchdog.h rtas_watchdog.c
			   
librtas2hpi_la_LIBADD	= -luuid -lrtas -lrtasevent $(top_builddir)/utils/libopenhpiutils.la
librtas2hpi_la_LDFLAGS	= -module -version-info @HPI_LIB_VERSION@

