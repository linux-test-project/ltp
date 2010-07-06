#
# Read COPYING for licensing details.
#
# Garrett Cooper, June 2010
#

# Makefiles that are considered critical to execution; if they don't exist
# all of the Makefiles will be rebuilt by default.
CRITICAL_CONFORMANCE_MAKEFILE=	conformance/interfaces/timer_settime/Makefile
CRITICAL_FUNCTIONAL_MAKEFILE=	functional/threads/pi_test/Makefile
CRITICAL_SECTION_MAKEFILE=	stress/threads/sem_open/Makefile

CRITICAL_MAKEFILES=	$(CRITICAL_CONFORMANCE_MAKEFILE) \
			$(CRITICAL_FUNCTIONAL_MAKEFILE) \
			$(CRITICAL_SECTION_MAKEFILE)

# The default expiration delay is 240 seconds
TIMEOUT_VAL?=		240
# The default logfile for the tests.
LOGFILE?=		logfile
# Subdirectories to traverse down.
SUBDIRS=		conformance functional stress

MAKE_ENV=		"CFLAGS=$(CFLAGS)" "LDFLAGS=$(LDFLAGS)"
MAKE_ENV+=		"LDLIBS=$(LDLIBS)"
MAKE_ENV+=		LOGFILE=`if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@

BUILD_MAKE=		env $(MAKE_ENV) $(MAKE)

top_srcdir?=		.

all: conformance-all functional-all stress-all tools-all

clean: $(CRITICAL_MAKEFILES) 
	@rm -f $(LOGFILE)*
	@for dir in $(SUBDIRS) tools; do \
	    $(MAKE) -C $$dir clean >/dev/null; \
	done

# Clean out all of the generated Makefiles.
distclean-makefiles:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

generate-makefiles: distclean-makefiles
	@env top_srcdir=$(top_srcdir) \
	    $(top_srcdir)/scripts/generate-makefiles.sh

install: conformance-install functional-install stress-install
	@install -m 0755 scripts/run-test.sh \
	    $(DESTDIR)/`$(SHELL) $(top_srcdir)/scripts/print-prefix.sh`/.

test: conformance-test functional-test stress-test
	@ln tools/t0 bin/t0

# Test build and execution targets.
conformance-all: $(CRITICAL_CONFORMANCE_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C conformance -j1 all

conformance-install:
	@$(MAKE) -C conformance install

conformance-test:
	@$(MAKE) -C conformance test

functional-all: $(CRITICAL_FUNCTIONAL_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C functional -j1 all

functional-install:
	@$(MAKE) -C functional install

functional-test:
	@$(MAKE) -C functional test

stress-all: $(CRITICAL_STRESS_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C stress -j1 all

stress-install:
	@$(MAKE) -C stress install

stress-test:
	@$(MAKE) -C stress test

tools-all:
	@$(MAKE) -C tools all

tools-install:
	@$(MAKE) -C tools install

$(CRITICAL_MAKEFILES): \
    $(top_srcdir)/scripts/generate-makefiles.sh	\
    $(top_srcdir)/CFLAGS			\
    $(top_srcdir)/LDFLAGS			\
    $(top_srcdir)/LDLIBS
	@$(MAKE) generate-makefiles

#tests-pretty:
#	$(MAKE) all | column -t -s:
