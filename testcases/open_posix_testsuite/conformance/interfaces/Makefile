# SPDX-License-Identifier: GPL-2.0-or-later
# Ngie Cooper, June 2010

.PHONY: all clean install test
all clean install test:
	@for dir in `ls -d */Makefile 2>/dev/null | sed -e 's,/Makefile$$,,g'`; do \
		$(MAKE) -C $$dir $@;						  \
	done

.PHONY: distclean-makefiles
distclean-makefiles:
	@find */ -name 'Makefile*' | xargs rm -f
