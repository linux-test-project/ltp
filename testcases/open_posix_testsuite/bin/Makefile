#
# Read COPYING for licensing details.
#
# Ngie Cooper, July 2010
#

top_srcdir?=		..

srcdir=			$(top_srcdir)/bin

prefix?=		`$(top_srcdir)/scripts/print-prefix.sh`

bindir?=		$(prefix)/bin

clean:
	@rm -f t0.val

install: clean
	@set -e; for i in `ls *`; do \
	    install -m 0755 $$i $(DESTDIR)/$(bindir)/. ;\
	done
