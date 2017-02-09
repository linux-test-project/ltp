#
# Read COPYING for licensing details.
#
# Garrett Cooper, July 2010
#

top_srcdir?=		$(abspath ../../..)

srcdir=			$(top_srcdir)/testcases/open_posix_testsuite/bin

prefix?=		`$(top_srcdir)/testcases/open_posix_testsuite/scripts/print-prefix.sh`

bindir?=		$(prefix)/bin

clean:
	@rm -f t0.val

install: clean
	@set -e; for i in `ls *`; do \
	    install -m 0755 $$i $(DESTDIR)/$(bindir)/. ;\
	done
