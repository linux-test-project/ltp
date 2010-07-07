#
# Read COPYING for licensing details.
#
# Garrett Cooper, July 2010
#

top_srcdir?=		..

srcdir=			$(top_srcdir)/bin

clean:
	@rm -f t0.val

install: clean
	@set -e; \
	INSTALL_DIR=`$(SHELL) $(top_srcdir)/scripts/print-prefix.sh`/bin/; \
	for i in `ls *`; do \
	    install -m 0755 $$i $(DESTDIR)/$$INSTALL_DIR/. ;\
	done



