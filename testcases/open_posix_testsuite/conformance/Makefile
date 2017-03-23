#
# Read COPYING for licensing details.
#
# Ngie Cooper, June 2010
#

top_srcdir?=$(abspath ../../..)
srcdir=$(top_srcdir)/testcases/open_posix_testsuite/conformance

all clean distclean-makefiles install test:
	@for dir in `cd $(srcdir) && ls -d */Makefile 2>/dev/null | sed -e 's,/Makefile$$,,g'`; do \
		mkdir -p $$dir; \
		$(MAKE) -C $$dir -f $(srcdir)/$$dir/Makefile $@; \
	done
