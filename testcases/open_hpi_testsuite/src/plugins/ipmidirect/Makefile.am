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

MAINTAINERCLEANFILES    = Makefile.in rpt_utils.c epath_utils.c uid_utils.c

INCLUDES                = -I$(top_srcdir)/include 

AM_CFLAGS               = @CFLAGS@ -DDEBUG

pkglib_LTLIBRARIES      = libipmidirect.la

libipmidirect_la_SOURCES= \
		entity.cpp \
		force.h \
		force.cpp \
		fru.cpp \
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
		ipmi_discover.cpp \
		ipmi_domain.h \
		ipmi_domain.cpp \
		ipmi_entity.h \
		ipmi_entity.cpp \
		ipmi_event.h \
		ipmi_event.cpp \
		ipmi_fru.h \
		ipmi_fru.cpp \
		ipmi_log.h \
		ipmi_log.cpp \
		ipmi_mc.h \
		ipmi_mc.cpp \
		ipmi_msg.h \
		ipmi_msg.cpp \
		ipmi_sdr.h \
		ipmi_sdr.cpp \
		ipmi_sel.h \
		ipmi_sel.cpp \
		ipmi_sensor.h \
		ipmi_sensor.cpp \
		ipmi_type_code.h \
		ipmi_type_code.cpp \
		ipmi_utils.h \
		ipmi_utils.cpp \
		md2.h \
		md2.cpp \
		md5.h \
		md5.cpp \
		sel.cpp \
		sensor.cpp \
		thread.h \
		thread.cpp

libipmidirect_la_LIBADD	= @SSL_LIB@ -luuid -lm
libipmidirect_la_LDFLAGS= -version-info @HPI_LIB_VERSION@

noinst_PROGRAMS         = plugin_test

plugin_test_SOURCES     = plugin_test.cpp rpt_utils.c epath_utils.c uid_utils.c
plugin_test_LDADD       = libipmidirect.la -luuid -lm @SSL_LIB@
plugin_test_LDFLAGS     = 

# this is a hack to links plugin_test
rpt_utils.c:
	if test ! -f rpt_utils.c -a ! -L rpt_utils.c; then \
		ln -s $(top_srcdir)/src/rpt_utils.c; \
	fi

epath_utils.c:
	if test ! -f epath_utils.c -a ! -L epath_utils.c; then \
		ln -s $(top_srcdir)/src/epath_utils.c; \
	fi

uid_utils.c:
	if test ! -f uid_utils.c -a ! -L uid_utils.c; then \
		ln -s $(top_srcdir)/src/uid_utils.c; \
	fi

clean:
	rm -f *~
	if test -L rpt_utils.c; then \
		rm -f rpt_utils.c; \
	fi
	if test -L epath_utils.c; then \
		rm -f epath_utils.c; \
	fi
	if test -L uid_utils.c; then \
		rm -f uid_utils.c; \
	fi
