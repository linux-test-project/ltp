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

CRITICAL_CONFORMANCE_MAKEFILE=	conformance/interfaces/timer_settime/Makefile
CRITICAL_FUNCTIONAL_MAKEFILE=	functional/threads/pi_test/Makefile

CRITICAL_MAKEFILES=	$(CRITICAL_CONFORMANCE_MAKEFILE) \
			$(CRITICAL_FUNCTIONAL_MAKEFILE)

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

all: conformance-all functional-all stress-all t0

clean:
	rm -f $(LOGFILE)* t0.val t0
	for d in $(SUBDIRS); do \
	    $(MAKE) -C $$d clean >/dev/null; \
	done

distclean:
	@find conformance/*/*/ functional/*/*/ -name Makefile -print0 | xargs rm -f

generate-makefiles:
	@export top_srcdir=$(top_srcdir); \
	$(top_srcdir)/scripts/generate-makefiles.sh

$(CRITICAL_MAKEFILES): scripts/generate-makefiles.sh
	$(MAKE) generate-makefiles

.PRECIOUS: %.test
%.test: %.o t0
	@COMPLOG=$(LOGFILE).$$$$; \
	TEST=`echo "$@" | sed -e 's,.test$$,,'`; \
	[ -f $< ] || exit 0; \
	{ nm -g $< | grep -q ' T main\| D main'; } || \
	{ echo "$$TEST: link: SKIP" >> $(LOGFILE) && exit 0; }; \
	if $(LINK.o) $< $(LOADLIBES) $(LDLIBS) -o $@ >$$COMPLOG 2>&1; \
	then \
		echo "$$TEST: link: PASS" >> $(LOGFILE); \
		echo "$$TEST: link: PASS"; \
	else \
		( \
			echo "$$TEST: link: FAILED. Linker output: "; \
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$$TEST: link: FAILED "; \
	fi; \
	rm -f $$COMPLOG;

t0: $(top_srcdir)/t0.c
	@echo Building timeout helper files
	$(CC) -o $@ $<

t0.val: t0
	echo `./t0 0; echo $$?` > t0.val

# Test build and execution targets.
conformance-all: conformance/interfaces/timer_settime/Makefile
	$(MAKE) -C conformance all

conformance-test:
	$(MAKE) -C conformance all

functional-all: functional/threads/pi_test/Makefile
	$(MAKE) -C functional all

functional-test:
	$(MAKE) -C functional test

stress-all:
	$(MAKE) -C stress all

stress-test:
	$(MAKE) -C stress test

tests-pretty:
	$(MAKE) all | column -t -s:
