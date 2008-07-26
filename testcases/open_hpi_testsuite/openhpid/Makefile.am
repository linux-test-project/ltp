#
# Copyright (c) 2004-2006 by IBM Corporation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
#
# Authors:
#    W. David Ashley <dashley@us.ibm.com>
#    Renier Morales <renier@openhpi.org>
#

.NOTPARALLEL:

MAINTAINERCLEANFILES 	= Makefile.in *~

REMOTE_SOURCES		= \
	config.c \
       	domain.c \
        event.c \
       	threaded.c \
        alarm.c \
       	hotswap.c \
       	lock.c \
       	plugin.c \
       	plugin_static.c \
       	init.c \
       	safhpi.c \
       	session.c \
       	ohpi.c

INCLUDES 		= \
	@OPENHPI_INCLUDES@  -I$(top_srcdir)/marshal @OH_SSL_INCLUDES@

EXTRA_DIST		= \
	openhpid.sh.in oh_client.h oh_client_conf.h oh_client_session.h

# daemon
sbin_PROGRAMS           = openhpid

openhpid_SOURCES        = openhpid.cpp
nodist_openhpid_SOURCES = $(REMOTE_SOURCES)

openhpid_LDADD		= $(top_builddir)/utils/libopenhpiutils.la \
			  $(top_builddir)/marshal/libohtcpconnx.la
if HAVE_OPENSSL
openhpid_LDADD		+= $(top_builddir)/$(SSLDIR)/libopenhpi_ssl.la
endif
openhpid_LDFLAGS        = -export-dynamic

# this is defined with ./configure --enable-daemon=static
if OPENHPID_STATIC
openhpid_LDFLAGS        += -static
else
openhpid_LDADD          += -lstdc++
endif


lib_LTLIBRARIES	        = libopenhpi.la

libopenhpi_la_SOURCES = oh_client.cpp oh_client_session.cpp oh_client_conf.c

libopenhpi_la_LDFLAGS = -version-info @HPI_LIB_VERSION@
libopenhpi_la_LIBADD  = $(top_builddir)/marshal/libohtcpconnx.la -lstdc++ \
                        $(top_builddir)/utils/libopenhpiutils.la


$(REMOTE_SOURCES):
	if test ! -f $@ -a ! -L $@; then \
		if test -f $(top_srcdir)/src/$@; then \
			ln -s $(top_srcdir)/src/$@; \
		else \
			ln -s $(top_builddir)/src/$@; \
		fi \
	fi

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/init.d
	$(INSTALL) -m 0755 openhpid.sh $(DESTDIR)$(sysconfdir)/init.d/openhpid

uninstall-local:
	rm -f $(DESTDIR)$(sysconfdir)/init.d/openhpid
	
clean-local:
	rm -f *~ core core.* $(REMOTE_SOURCES)
