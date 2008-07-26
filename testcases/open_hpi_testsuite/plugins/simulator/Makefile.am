#      -*- linux-c -*-
#
# (C) Copyright IBM Corp. 2005, 2006
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
#
# Author(s):
#        Christina Hernandez <hernanc@us.ibm.com>
#        W. David Ashley <dashley@us.ibm.com>
#	 Renier Morales <renier@openhpi.org>
#

SUBDIRS = t

MAINTAINERCLEANFILES 	= Makefile.in

INCLUDES		= @OPENHPI_INCLUDES@

pkglib_LTLIBRARIES	= libsimulator.la

libsimulator_la_LIBADD	= -luuid $(top_builddir)/utils/libopenhpiutils.la
libsimulator_la_SOURCES	= sim_init.c sim_init.h \
                          sim_resources2.c sim_resources.h \
			  sim_sensors.c sim_sensors.h \
			  sim_sensor_func.c sim_sensor_func.h \
			  sim_controls.c sim_controls.h \
			  sim_control_func.c sim_control_func.h \
			  sim_injector.c sim_injector.h \
			  sim_el.c sim_el.h \
			  sim_annunciators.c sim_annunciators.h \
			  sim_annunciator_func.c sim_annunciator_func.h \
              sim_dimi.h sim_dimi.c \
			  sim_dimi_func.h sim_dimi_func.c \
              sim_fumi.h sim_fumi.c \
              sim_fumi_func.h sim_fumi_func.c \
			  sim_power.c sim_power.h \
			  sim_inventory.c sim_inventory.h \
			  sim_watchdog.c sim_watchdog.h \
			  sim_hotswap.c sim_hotswap.h \
			  sim_reset.c sim_reset.h

libsimulator_la_LDFLAGS	= -module -version-info @HPI_LIB_VERSION@
