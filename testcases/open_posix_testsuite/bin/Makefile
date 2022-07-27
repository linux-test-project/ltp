#
# Read COPYING for licensing details.
#
# Ngie Cooper, July 2010
#

top_srcdir ?= ..

include $(top_srcdir)/include/mk/config.mk

INSTALL_BIN_TARGETS = run-all-posix-option-group-tests.sh run-posix-option-group-test.sh
INSTALL_TESTCASE_BIN_TARGETS = run-tests.sh t0

.PHONY: clean
clean:
	@rm -f t0.val

.PHONY: install
install: clean $(DESTDIR)/$(bindir) $(DESTDIR)/$(testdir_bin)
	set -e; for file in $(INSTALL_BIN_TARGETS); do           \
		install -m 00755 $$file $(DESTDIR)/$(bindir)/$$file; \
	done

	sed -i 's~TESTPATH=""~TESTPATH="$(testdir_rel)"~' $(DESTDIR)/$(bindir)/run-posix-option-group-test.sh

	set -e; for file in $(INSTALL_TESTCASE_BIN_TARGETS); do	      \
		install -m 00755 $$file $(DESTDIR)/$(testdir_bin)/$$file; \
	done


$(DESTDIR)/$(bindir):
	mkdir -p $@

$(DESTDIR)/$(testdir_bin):
	mkdir -p $@
