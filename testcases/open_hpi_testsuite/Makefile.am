#
# Copyright (c) 2003, Intel Corporation
# (C) Copyright IBM Corp 2003-2006
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

#AUTOMAKE_OPTIONS        = 1.8

TARFILE                 = $(PACKAGE_NAME)-$(VERSION).tar.gz

DESTDIR                 =

RPM                     = @RPM@
RPMFLAGS                = -ba

EXTRA_DIST              = openhpi.spec.in \
	openhpi.conf.example \
	openhpiclient.conf.example \
	README.daemon \
        $(shell find $(srcdir)/include -name '*.h')

AUTOMAKE_OPTIONS        = foreign

MAINTAINERCLEANFILES    = Makefile.in aclocal.m4 configure config.guess config.sub \
                        depcomp install-sh ltmain.sh missing mkinstalldirs config.h.in \
                        stamp-h.in cscope.files cscope.out $(distdir).tar.gz compile
MOSTLYCLEANFILES        = tags


SUBDIRS                 = utils scripts @SSLDIR@ src @SNMPDIR@ @MARSHALDIR@ plugins docs @ENABLED_DIRS@ hpi_shell
DIST_SUBDIRS            = utils scripts src snmp ssl marshal plugins docs openhpid clients cpp hpi_shell


DISTCHECK_CONFIGURE_FLAGS = --with-initdir=prefix

ALLSOURCES = \
        $(shell find $(top_srcdir)/include -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/src -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/utils -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/openhpid -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print -o '*.cpp' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/plugins -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print -o '*.cpp' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/snmp -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/ssl -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/marshal -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/clients -name .svn -o -name CVS -prune -o \
        -name '*.[chS]' -print | grep -v '/t/') \
        $(shell find $(top_srcdir)/cpp -name .svn -o -name CVS -prune -o \
        -name '*.[ch]pp' -print | grep -v '/t/')

includedir=$(base_includedir)/openhpi
# These are the only headers that users should have any access to
include_HEADERS = $(top_srcdir)/include/SaHpi.h \
                  $(top_srcdir)/include/SaHpiAtca.h \
		  $(top_srcdir)/include/SaHpiBladeCenter.h \
                  $(top_srcdir)/include/oHpi.h \
                  $(top_srcdir)/utils/oh_utils.h \
                  $(top_srcdir)/utils/announcement_utils.h \
                  $(top_srcdir)/utils/rpt_utils.h \
                  $(top_srcdir)/utils/sahpi_enum_utils.h \
		  $(top_srcdir)/utils/sahpiatca_enum_utils.h \
                  $(top_srcdir)/utils/sahpi_event_encode.h \
                  $(top_srcdir)/utils/sahpi_event_utils.h \
                  $(top_srcdir)/utils/sahpi_struct_utils.h \
                  $(top_srcdir)/utils/sahpi_time_utils.h \
                  $(top_srcdir)/utils/uid_utils.h \
                  $(top_srcdir)/utils/epath_utils.h \
                  $(top_srcdir)/utils/el_utils.h

docsdir=$(datadir)/doc/$(PACKAGE_NAME)-$(VERSION)
docs_DATA = README README.daemon COPYING

$(top_srcdir)/utils/sahpi_enum_utils.h:
	make -C $(top_builddir)/utils sahpi_enum_utils.h

$(top_srcdir)/utils/sahpiatca_enum_utils.h:
	make -C $(top_builddir)/utils sahpiatca_enum_utils.h

install-data-local: openhpi.pc openhpi.conf.example openhpiclient.conf.example
	$(mkinstalldirs) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) openhpi.pc $(DESTDIR)$(libdir)/pkgconfig
	$(mkinstalldirs) $(DESTDIR)$(VARPATH)
	chmod 777 $(DESTDIR)$(VARPATH)
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/openhpi
	if test ! -e  $(DESTDIR)$(sysconfdir)/openhpi/openhpi.conf; then \
		$(INSTALL_DATA) $(top_srcdir)/openhpi.conf.example $(DESTDIR)$(sysconfdir)/openhpi/openhpi.conf; \
	fi
	if test ! -e  $(DESTDIR)$(sysconfdir)/openhpi/openhpiclient.conf; then \
		$(INSTALL_DATA) $(top_srcdir)/openhpiclient.conf.example $(DESTDIR)$(sysconfdir)/openhpi/openhpiclient.conf; \
	fi

uninstall-local:
	rm -f $(DESTDIR)$(libdir)/pkgconfig/openhpi.pc
	if cmp $(top_srcdir)/openhpi.conf.example $(DESTDIR)$(sysconfdir)/openhpi/openhpi.conf; then \
		rm -f $(DESTDIR)$(sysconfdir)/openhpi/openhpi.conf; \
	fi
	if cmp $(top_srcdir)/openhpiclient.conf.example $(DESTDIR)$(sysconfdir)/openhpi/openhpiclient.conf; then \
		rm -f $(DESTDIR)$(sysconfdir)/openhpi/openhpiclient.conf; \
	fi

rpm: dist
	rm -rf $(top_srcdir)/rpm
	mkdir -p $(top_srcdir)/rpm/RPMS
	mkdir $(top_srcdir)/rpm/SRPMS
	mkdir $(top_srcdir)/rpm/BUILD
	mkdir $(top_srcdir)/rpm/INSTALL
	$(RPM) $(RPMFLAGS) $(PACKAGE_NAME).spec </dev/null;

documentation:
	$(MAKE) -C docs

clean-local: am_config_clean-local

am_config_clean-local:
	rm -fr $(PACKAGE)-*.tar.gz
	rm -rf rpm

report:
	$(MAKE) -C scripts/test report

tags:   FORCE
	@echo making tags
	ctags $(ALLSOURCES)
TAGS:   FORCE
	@echo making TAGS
	etags $(ALLSOURCES)
cscope: FORCE
	@echo making cscope
	echo "$(ALLSOURCES)" | sed -e 's/[ ]\+/\n/g' > cscope.files
	cscope -b

.PHONY: FORCE
