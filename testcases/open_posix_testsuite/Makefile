# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  inaky.perez-gonzalez REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# This file has almost been entirely rewritten by Garrett Cooper, but in
# order to note the bits which were acquired from previous versions, the Intel
# copyright notice stands above.
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

# Force to -j1 so the output in the logfile won't be wonky.
MAKE+=			-j1
MAKE+=			LOGFILE=`if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@

MAKE_ENV=		"CFLAGS=$(CFLAGS)" "LDFLAGS=$(LDFLAGS)"
MAKE_ENV+=		"LDLIBS=$(LDLIBS)"

BUILD_MAKE=		env $(MAKE_ENV) $(MAKE)

top_srcdir?=		.

all: conformance-all functional-all stress-all tools

clean:
	@rm -f $(LOGFILE)*
	@for dir in $(SUBDIRS) tools; do \
	    $(BUILD_MAKE) -C $$dir clean >/dev/null; \
	done

# Clean out all of the generated Makefiles; this is done this way to ensure
# there won't be an argument overflow, and to remain POSIXly correct (POSIX
# find(1) doesn't support -mindepth like BSD and GNU find do...).
distclean-makefiles:
	@for dir in $(SUBDIRS); do \
	  (cd $$dir && for subdir in */; do \
	      (cd $$subdir && find */ -name Makefile 2>/dev/null | xargs rm -f); \
	  done); \
	done

generate-makefiles: distclean-makefiles
	@export top_srcdir=$(top_srcdir); \
	$(top_srcdir)/scripts/generate-makefiles.sh

$(CRITICAL_MAKEFILES): scripts/generate-makefiles.sh
	@$(BUILD_MAKE) generate-makefiles

tools:
	@$(BUILD_MAKE) -C tools all

# Test build and execution targets.
conformance-all: $(CRITICAL_CONFORMANCE_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C conformance all

conformance-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(MAKE) -C conformance test

functional-all: $(CRITICAL_FUNCTIONAL_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C functional all

functional-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(MAKE) -C functional test

stress-all: $(CRITICAL_STRESS_MAKEFILE)
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(BUILD_MAKE) -C stress all

stress-test:
	@rm -f `if echo "$(LOGFILE)" | grep -q '^/'; then echo "$(LOGFILE)"; else echo "\`pwd\`/$(LOGFILE)"; fi`.$@
	@$(MAKE) -C stress test

tests-pretty:
	@$(MAKE) all | column -t -s:
