## Process this file with automake to create Makefile.in. -*-Makefile-*-

## Makefile for Autoconf.
## Copyright 1999, 2000, 2001 Free Software Foundation, Inc.

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

AUTOMAKE_OPTIONS = check-news 1.4 readme-alpha

SUBDIRS = . config m4 man doc tests

SUFFIXES = .m4 .m4f
## There is currently no means with Automake not to run aclocal.
ACLOCAL_AMFLAGS = --version >/dev/null && touch aclocal.m4
WGET = wget

bin_SCRIPTS = autoconf autoheader autoreconf ifnames @PERLSCRIPTS@
EXTRA_SCRIPTS = autoscan autoupdate

# FIXME:
# s/distpackageDATA/dist_pkgdata_DATA/
# s/nodistpackageDATA/nodist_pkgdata_DATA/
# and adapt dependencies once we use a more recent Automake

m4sources =  m4sugar.m4 m4sh.m4  \
             $(srcdir)/acversion.m4 \
             autoconf.m4 \
             acgeneral.m4 acoldnames.m4 acspecific.m4 aclang.m4 \
             acfunctions.m4 acheaders.m4 actypes.m4

distpkgdataDATA = acfunctions acheaders acidentifiers acmakevars acprograms \
                  aclibraries $(m4sources)

nodistpkgdataDATA = autoconf.m4f

pkgdata_DATA = $(distpkgdataDATA) $(nodistpkgdataDATA)

EXTRA_DIST = ChangeLog.0 ChangeLog.1 ChangeLog.2 \
             BUGS INSTALL.txt \
             acversion.in \
             autoconf.in autoheader.in autoreconf.in autoupdate.in ifnames.in \
             autoscan.in \
             $(distpkgdataDATA) \
             GNUmakefile Makefile.maint

# Files that should be removed, but which Automake does not know:
# the frozen files and the scripts.
CLEANFILES = autoconf.m4f \
             $(bin_SCRIPTS)


## ------------------ ##
## Maintainer rules.  ##
## ------------------ ##

## acversion.m4.  ##

# - acversion.m4 needs to be updated only once, since it depends on
#   configure.ac, not on the results of a 'configure' run.
# - It is guaranteed (with GNU Make) that when the version in configure.ac
#   is changed, acversion.m4 is built only after the new version number is
#   propagated to the Makefile.  (Libtool uses the same guarantee.)

$(srcdir)/acversion.m4: $(srcdir)/acversion.in $(srcdir)/configure.ac
	sed 's,@VERSION\@,$(VERSION),g' $(srcdir)/acversion.in >acversion.tm4
	mv acversion.tm4 $(srcdir)/acversion.m4

## INSTALL.  ##

# INSTALL is a special case.  Automake seems to have a single name space
# for both targets and variables.  If we just use INSTALL, then the var
# $(INSTALL) is not defined, and the install target fails.
MAKEINFO = @MAKEINFO@ --no-headers --no-validate --no-split
INSTALL.txt: $(top_srcdir)/doc/install.texi
	$(MAKEINFO) $(top_srcdir)/doc/install.texi --output=$(srcdir)/INSTALL.txt
	cp $(srcdir)/INSTALL.txt $(srcdir)/INSTALL

MAINTAINERCLEANFILES = acversion.m4 INSTALL.txt


## maintainer-check ##
maintainer-check: maintainer-check-tests
maintainer-check-tests:
	cd tests && make maintainer-check

## ----------------------------------- ##
## Special installation instructions.  ##
## ----------------------------------- ##

install-data-hook: INSTALL.txt
	@$(NORMAL_INSTALL)
	@list='INSTALL'; for p in $$list; do \
	  if test -f "$$p.txt"; then d= ; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p.txt $(DESTDIR)$(pkgdatadir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p.txt $(DESTDIR)$(pkgdatadir)/$$f; \
	done


## ------------- ##
## The scripts.  ##
## ------------- ##

edit = sed \
	-e 's,@SHELL\@,$(SHELL),g' \
	-e 's,@PERL\@,$(PERL),g' \
	-e 's,@datadir\@,$(pkgdatadir),g' \
	-e 's,@bindir\@,$(bindir),g' \
	-e 's,@autoconf-name\@,'`echo autoconf | sed '$(transform)'`',g' \
	-e 's,@autoheader-name\@,'`echo autoheader | sed '$(transform)'`',g' \
	-e 's,@M4\@,$(M4),g' \
	-e 's,@AWK\@,$(AWK),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@PACKAGE_NAME\@,$(PACKAGE_NAME),g'

## All the scripts below depend on configure.ac so that they are rebuilt
## when the Autoconf version changes. Unfortunately, suffix rules cannot
## have additional dependencies, so we have to use explicit rules for
## every script.

autoconf: $(srcdir)/autoconf.in $(srcdir)/configure.ac
	rm -f autoconf autoconf.tmp
	$(edit) $(srcdir)/autoconf.in >autoconf.tmp
	chmod +x autoconf.tmp
	mv autoconf.tmp autoconf

autoheader: $(srcdir)/autoheader.in $(srcdir)/configure.ac
	rm -f autoheader autoheader.tmp
	$(edit) $(srcdir)/autoheader.in >autoheader.tmp
	chmod +x autoheader.tmp
	mv autoheader.tmp autoheader

autoreconf: $(srcdir)/autoreconf.in $(srcdir)/configure.ac
	rm -f autoreconf autoreconf.tmp
	$(edit) $(srcdir)/autoreconf.in >autoreconf.tmp
	chmod +x autoreconf.tmp
	mv autoreconf.tmp autoreconf

autoupdate: $(srcdir)/autoupdate.in $(srcdir)/configure.ac
	rm -f autoupdate autoupdate.tmp
	$(edit) $(srcdir)/autoupdate.in >autoupdate.tmp
	chmod +x autoupdate.tmp
	mv autoupdate.tmp autoupdate

ifnames: $(srcdir)/ifnames.in $(srcdir)/configure.ac
	rm -f ifnames ifnames.tmp
	$(edit) $(srcdir)/ifnames.in >ifnames.tmp
	chmod +x ifnames.tmp
	mv ifnames.tmp ifnames

autoscan: $(srcdir)/autoscan.in $(srcdir)/configure.ac
	rm -f autoscan autoscan.tmp
	$(edit) $(srcdir)/autoscan.in >autoscan.tmp
	chmod +x autoscan.tmp
	mv autoscan.tmp autoscan


## ------------------ ##
## The frozen files.  ##
## ------------------ ##


# When processing the file with diversion disabled, there must be no
# output but comments and empty lines.
# If freezing produces output, something went wrong: a bad `divert',
# or an improper paren etc.
# It may happen that the output does not end with a end of line, hence
# force an end of line when reporting errors.
.m4.m4f:
	$(M4) --include $(srcdir) --fatal-warning --define divert \
	      $(srcdir)/$*.m4 2>error.log | \
	  sed 's/#.*//;/^$$/d' >process.log
	if grep . error.log >/dev/null 2>&1; then \
	  echo "ERROR: Processing $(srcdir)/$*.m4 produced errors:" >&2; \
	  sed "s,^,$(srcdir)/$*.m4: ," <error.log >&2; \
	  echo >&2; \
	  exit 1; \
	else \
	  rm -f error.log; \
	fi
	if grep . process.log >/dev/null 2>&1; then \
	  echo "ERROR: Processing $(srcdir)/$*.m4 produced output:" >&2; \
	  sed "s,^,$(srcdir)/$*.m4: ," <process.log >&2; \
	  echo >&2; \
	  exit 1; \
	else \
	  rm -f process.log; \
	fi
	$(M4) --include $(srcdir) --fatal-warning --freeze-state=$*.m4f \
	      $(srcdir)/$*.m4 >freeze.log
	if grep . freeze.log >/dev/null 2>&1; then \
	  echo "ERROR: Freezing $(srcdir)/$*.m4 produced output:" >&2; \
	  sed "s,^,$(srcdir)/$*.m4: ," <freeze.log >&2; \
	  echo >&2; \
	  exit 1; \
	else \
	  rm -f freeze.log; \
	fi

autoconf.m4f: $(m4sources)


## ---------------------------- ##
## Customizing Makefile.maint.  ##
## ---------------------------- ##

prev_version_file = $(srcdir)/config/prev-version.txt
release_archive_dir = releases

# Uploading betas.
hosts = alpha
alpha_host = alpha.gnu.org
alpha_url_dir = gnu/autoconf

# Files to update automatically.
wget_files = $(srcdir)/config/config.guess $(srcdir)/config/config.sub \
	     $(srcdir)/config/texinfo.tex \
	     $(srcdir)/doc/standards.texi $(srcdir)/doc/make-stds.texi
