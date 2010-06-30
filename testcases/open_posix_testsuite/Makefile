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

# add -std=c99, -std=gnu99 if compiler supports it (gcc-2.95.3 does not).
CFLAGS+=		`$(CC) -std=c99 -S -o /dev/null -xc /dev/null >/dev/null 2>&1 && echo "-std=c99"`
CFLAGS+=		`$(CC) -std=gnu99 -S -o /dev/null -xc /dev/null >/dev/null 2>&1 && echo "-std=gnu99"`
CFLAGS+=		-D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600

#LDFLAGS+=
#LDLIBS+=	-lpthread -lrt -lm

top_srcdir?=		.

all: conformance-all functional-all stress-all tools

clean:
	rm -f $(LOGFILE)* t0 t0.val
	for d in $(SUBDIRS); do \
	    $(MAKE) -C $$d clean >/dev/null; \
	done

# Clean out all of the generated Makefiles; this is done this way to ensure
# there won't be an argument overflow, and to remain POSIXly correct (POSIX
# find(1) doesn't support -mindepth like BSD and GNU find do...).
distclean-makefiles:
	for dir in $(SUBDIRS); do \
	  (cd $$dir && for subdir in */; do \
	      (cd $$subdir && find */ -name Makefile 2>/dev/null | xargs rm -f); \
	  done); \
	done

generate-makefiles: distclean-makefiles
	@export top_srcdir=$(top_srcdir); \
	$(top_srcdir)/scripts/generate-makefiles.sh

$(CRITICAL_MAKEFILES): scripts/generate-makefiles.sh
	@$(MAKE) generate-makefiles

tools:
	@$(MAKE) -C tools all

tools/t0: $(srcdir)/tools/t0.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(srcdir)/$(@F) $(LDLIBS)

#build/t0.val: build/t0
#	echo `./t0 0; echo $$?` > t0.val

# Test build and execution targets.
conformance-all: $(CRITICAL_CONFORMANCE_MAKEFILE)
	$(MAKE) -C conformance all

conformance-test:
	$(MAKE) -C conformance all

functional-all: $(CRITICAL_FUNCTIONAL_MAKEFILE)
	$(MAKE) -C functional all

functional-test:
	$(MAKE) -C functional test

stress-all: $(CRITICAL_STRESS_MAKEFILE)
	$(MAKE) -C stress all

stress-test:
	$(MAKE) -C stress test

tests-pretty:
	$(MAKE) all | column -t -s:
