#
# Copyright (c) 2004 by FORCE Computers
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
#
# Authors:
#     Thomas Kanngieser <thomas.kanngieser@fci.com>
#

CON_REMOTE_SOURCES = \
	ipmi_con.cpp \
	ipmi_con_lan.cpp \
	ipmi_con_smi.cpp \
	ipmi_auth.cpp \
	ipmi_cmd.cpp \
	ipmi_log.cpp \
	ipmi_msg.cpp \
	ipmi_addr.cpp \
	ipmi_utils.cpp \
	thread.cpp

THREAD_REMOTE_SOURCES = thread.cpp

SENSOR_FACTORS_REMOTE_SOURCES = ipmi_sensor_factors.cpp

MOSTLYCLEANFILES 	= \
	$(CON_REMOTE_SOURCES) \
	$(THREAD_REMOTE_SOURCES) \
	$(SENSOR_FACTORS_REMOTE_SOURCES) \
	@TEST_CLEAN@ \
	*.log

MAINTAINERCLEANFILES 	= Makefile.in *~

CLEANFILES		= @CLEANFILES@ $(MOSTLYCLEANFILES)

INCLUDES		= @OPENHPI_INCLUDES@ -I$(top_srcdir)/plugins/ipmidirect


$(CON_REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		ln -s $(top_srcdir)/plugins/ipmidirect/$@; \
	fi

$(SENSOR_FACTORS_REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		ln -s $(top_srcdir)/plugins/ipmidirect/$@; \
	fi

check_PROGRAMS = \
	con_000 \
	con_001 \
	thread_000 \
	sensor_factors_000

TESTS = \
	thread_000 \
	sensor_factors_000

con_000_SOURCES = con_000.cpp
nodist_con_000_SOURCES = $(CON_REMOTE_SOURCES)
con_000_LDADD   = @CRYPTO_LIB@

con_001_SOURCES = con_001.c
nodist_con_001_SOURCES = $(CON_REMOTE_SOURCES)
con_001_LDADD   = @CRYPTO_LIB@

thread_000_SOURCES = thread_000.cpp test.h
nodist_thread_000_SOURCES = $(THREAD_REMOTE_SOURCES)

sensor_factors_000_SOURCES = sensor_factors_000.cpp test.h
nodist_sensor_factors_000_SOURCES = $(SENSOR_FACTORS_REMOTE_SOURCES)
