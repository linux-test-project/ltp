#
# Read COPYING for licensing details.
#
# Garrett Cooper, June 2010
#

# Makefiles that are considered critical to execution; if they don't exist
# all of the Makefiles will be rebuilt by default.
CRITICAL_MAKEFILE=	conformance/interfaces/timer_settime/Makefile

# The default logfile for the tests.
LOGFILE?=		logfile
# Subdirectories to traverse down.
SUBDIRS=		conformance functional stress

MAKE_ENV=		LOGFILE=`if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@

BUILD_MAKE_ENV=		"CFLAGS=$(CFLAGS)" "LDFLAGS=$(LDFLAGS)"
BUILD_MAKE_ENV+=	"LDLIBS=$(LDLIBS)" $(MAKE_ENV)

TEST_MAKE_ENV=		$(MAKE_ENV)

BUILD_MAKE=		env $(BUILD_MAKE_ENV) $(MAKE)

TEST_MAKE=		env $(TEST_MAKE_ENV) $(MAKE) -k

top_srcdir?=		$(abspath ../..)

srcdir=			$(top_srcdir)/testcases/open_posix_testsuite

prefix?=		`$(srcdir)/scripts/print-prefix.sh`

datadir?=		$(prefix)/share

exec_prefix?=		$(prefix)

all: conformance-all functional-all stress-all tools-all

ifeq ($(shell uname -s), Linux)
include $(srcdir)/Makefile.linux
endif

clean: $(CRITICAL_MAKEFILE)
	@rm -f $(LOGFILE)*
	@for dir in $(SUBDIRS) tools; do \
		mkdir -p $$dir; \
		$(MAKE) -C $$dir -f $(srcdir)/$$dir/Makefile clean >/dev/null; \
	done

distclean: distclean-makefiles

# Clean out all of the generated Makefiles.
distclean-makefiles:
	@for dir in $(SUBDIRS); do \
		mkdir -p $$dir; \
		$(MAKE) -C $$dir -f $(srcdir)/$$dir/Makefile $@; \
	done

generate-makefiles: distclean-makefiles
	@env top_srcdir=$(top_srcdir) \
	    $(srcdir)/scripts/generate-makefiles.sh

install: bin-install conformance-install functional-install stress-install

test: conformance-test functional-test stress-test

# Test build and execution targets.
conformance-all: $(CRITICAL_MAKEFILE)
	@mkdir -p conformance
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C conformance -f $(srcdir)/conformance/Makefile -j1 all

conformance-install:
	@$(MAKE) -C conformance -f $(srcdir)/conformance/Makefile install

conformance-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(TEST_MAKE) -C conformance -f $(srcdir)/conformance/Makefile test

functional-all: $(CRITICAL_MAKEFILE)
	@mkdir -p functional
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C functional -f $(srcdir)/functional/Makefile -j1 all

functional-install:
	@$(MAKE) -C functional -f $(srcdir)/functional/Makefile install

functional-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(TEST_MAKE) -C functional -f $(srcdir)/functional/Makefile test

stress-all: $(CRITICAL_MAKEFILE)
	@mkdir -p stress
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C stress -f $(srcdir)/stress/Makefile -j1 all

stress-install:
	@$(MAKE) -C stress -f $(srcdir)/stress/Makefile install

stress-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(TEST_MAKE) -C stress -f $(srcdir)/stress/Makefile test

# Tools build and install targets.
bin-install:
	@mkdir -p bin
	@$(MAKE) -C bin -f $(srcdir)/bin/Makefile install

tools-all:
	@mkdir -p tools
	@$(MAKE) -C tools -f $(srcdir)/tools/Makefile all

$(CRITICAL_MAKEFILE): \
    $(srcdir)/scripts/generate-makefiles.sh  \
    $(srcdir)/CFLAGS                         \
    $(srcdir)/LDFLAGS                        \
    $(srcdir)/LDLIBS
	@$(MAKE) -f $(srcdir)/Makefile generate-makefiles
