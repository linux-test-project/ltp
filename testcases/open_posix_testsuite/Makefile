#
# Read COPYING for licensing details.
#
# Ngie Cooper, June 2010
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

top_srcdir?=		.

prefix?=		`$(top_srcdir)/scripts/print_prefix.sh`

datadir?=		$(prefix)/share

exec_prefix?=		$(prefix)

all: conformance-all functional-all stress-all tools-all

ifeq ($(shell uname -s), Linux)
include Makefile.linux
endif

clean: $(CRITICAL_MAKEFILE)
	@rm -f $(LOGFILE)*
	@for dir in $(SUBDIRS) tools; do \
	    $(MAKE) -C $$dir clean >/dev/null; \
	done

distclean: distclean-makefiles

# Clean out all of the generated Makefiles.
distclean-makefiles:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

generate-makefiles: distclean-makefiles
	@env top_srcdir=$(top_srcdir) \
	    $(top_srcdir)/scripts/generate-makefiles.sh

install: bin-install conformance-install functional-install stress-install

test: conformance-test functional-test stress-test

# Test build and execution targets.
conformance-all: $(CRITICAL_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C conformance -j1 all

conformance-install:
	@$(MAKE) -C conformance install

conformance-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(TEST_MAKE) -C conformance test

functional-all: $(CRITICAL_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C functional -j1 all

functional-install:
	@$(MAKE) -C functional install

functional-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(TEST_MAKE) -C functional test

stress-all: $(CRITICAL_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C stress -j1 all

stress-install:
	@$(MAKE) -C stress install

stress-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(TEST_MAKE) -C stress test

# Tools build and install targets.
bin-install:
	@$(MAKE) -C bin install

tools-all:
	@$(MAKE) -C tools all

$(CRITICAL_MAKEFILE): \
    $(top_srcdir)/scripts/generate-makefiles.sh	\
    $(top_srcdir)/CFLAGS			\
    $(top_srcdir)/LDFLAGS			\
    $(top_srcdir)/LDLIBS
	@$(MAKE) generate-makefiles
