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

AM_CFLAGS		= @CFLAGS@

pkglib_LTLIBRARIES	= librtas2hpi.la 

librtas2hpi_la_SOURCES	=  rtas.c \
			   rtas_annunciator.c \
			   rtas_discover.c \
			   rtas_control.c \
			   rtas_event.c \
			   rtas_eventlog.c \
			   rtas_hotswap.c \
			   rtas_indicator.c \
			   rtas_inventory.c \
			   rtas_power.c \
			   rtas_reset.c \
			   rtas_resource.c \
			   rtas_sensor.c \
			   rtas_utils.c \
			   rtas_watchdog.c
			   
librtas2hpi_la_LIBADD	= -luuid -lrtas -lrtasevent $(top_builddir)/utils/libopenhpiutils.la
librtas2hpi_la_LDFLAGS	= -module -version-info @HPI_LIB_VERSION@

