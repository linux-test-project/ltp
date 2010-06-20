# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  inaky.perez-gonzalez REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# Kind of a little bit bastardized automakefile ... This is the
# temporary glue to hold it all together; once our needs change or we
# need something more advanced, we'll implement it.
#
# So far, I understand Make is not the best language, but I felt lazy
# today and wanted to use the default rules of automake [did I alredy
# mentioned I am bastardizing it?].
#
# Ok, I don't use Automake any more
#
# Added patch from dank REMOVE-THIS AT kegel DOT com
#

# Added tests timeout from Sebastien Decugis (http://nptl.bullopensource.org) 
# Expiration delay is 240 seconds
TIMEOUT_VAL = 240
# The following value is the shell return value of a timedout application.
# with the bash shell, the ret val of a killed application is 128 + signum
# and under Linux, SIGALRM=14, so we have (Linux+bash) 142.

top_builddir = .

LOGFILE=	$(top_builddir)/logfile

RUN_TESTS=	$(shell $(top_builddir)/locate-test --execs $(top_builddir)/$(POSIX_TARGET))
BUILD_TESTS=	$(shell $(top_builddir)/locate-test --buildable $(top_builddir)/$(POSIX_TARGET))
FUNCTIONAL_MAKE=`$(top_builddir)/locate-test --fmake`
FUNCTIONAL_RUN=	$(shell $(top_builddir)/locate-test --frun)
STRESS_MAKE=	`$(top_builddir)/locate-test --smake`
STRESS_RUN=	$(shell $(top_builddir)/locate-test --srun)
PWD=		`pwd`
TIMEOUT=	$(top_builddir)/t0 $(TIMEOUT_VAL)


all: build-tests run-tests 

build-tests: $(BUILD_TESTS:.c=.test)
run-tests: $(RUN_TESTS:.test=.run-test)

functional-tests: functional-make functional-run
stress-tests: stress-make stress-run

tests-pretty:
	$(MAKE) all | column -t -s:

# add -std=c99, -std=gnu99 if compiler supports it (gcc-2.95.3 does not).
CPPFLAGS+=	`$(CC) -std=c99 -S -o /dev/null -xc /dev/null >/dev/null 2>&1 && echo "-std=c99"`
CPPFLAGS+=	`$(CC) -std=gnu99 -S -o /dev/null -xc /dev/null >/dev/null 2>&1 && echo "-std=gnu99"`
CPPFLAGS+=	-D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600
CPPFLAGS+=	-I$(PWD)/include

LDFLAGS+=
LDLIBS+=	-lpthread -lrt -lm

# FIXME: exhaust cmd line length
clean:
	-@rm -f $(LOGFILE)*
	-@# Timeout helper files
	-@rm -f $(top_builddir)/t0.val $(top_builddir)/t0
	-@# Build runnable tests
	-@find $(top_builddir) -iname \*.test | xargs -n 40 rm -f {}
	-@find $(top_builddir) -iname \*~ -o -iname \*.o | xargs -n 40 rm -f {}
	@for d in $(FUNCTIONAL_MAKE); do \
	    $(MAKE) -C $$d clean >/dev/null; \
	done

# Rule to run a build test
# If the .o doesn't export main, then we don't need to link
.PRECIOUS: %.test
%.test: %.o $(top_builddir)/t0
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

# Rule to run an executable test
# If it is only a build test, then the binary exist, so we don't need to run
.PHONY: %.run-test
%.run-test: %.test $(top_builddir)/t0.val
	@COMPLOG=$(LOGFILE).$$$$; \
	TEST=`echo "$@" | sed -e 's,.run-test$$,,'`; \
	TIMEOUT_RET=`cat $(top_builddir)/t0.val`; \
	[ -f $< ] || exit 0; \
	$(TIMEOUT) $< > $$COMPLOG 2>&1; \
	RESULT=$$?; \
	if [ $$RESULT -eq 1 ]; \
	then \
		MSG="FAILED"; \
	elif [ $$RESULT -eq 2 ]; \
	then \
		MSG="UNRESOLVED"; \
	elif [ $$RESULT -eq 4 ]; \
	then \
		MSG="UNSUPPORTED"; \
	elif [ $$RESULT -eq 5 ]; \
	then \
		MSG="UNTESTED"; \
	elif [ $$RESULT -eq $$TIMEOUT_RET ]; \
	then \
		MSG="HUNG"; \
	elif [ $$RESULT -gt 5 -a $$RESULT -ne $$TIMEOUT_RET ]; \
	then \
		MSG="INTERRUPTED"; \
	elif [ $$RESULT -eq 0 ]; \
	then \
		echo "$$TEST: execution: PASS" >> $(LOGFILE); \
	else \
		( \
			echo "$$TEST: execution: $$MSG: Output: "; \
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$$TEST: execution: $$MSG "; \
	fi; \
	rm -f $$COMPLOG;

$(top_builddir)/t0: $(top_builddir)/t0.c
	@echo Building timeout helper files; \
	$(CC) -O2 -o $@ $<

$(top_builddir)/t0.val: $(top_builddir)/t0
	echo `$(top_builddir)/t0 0; echo $$?` > $(top_builddir)/t0.val

%.run-test: %.sh $(top_builddir)/t0.val
	@COMPLOG=$(LOGFILE).$$$$; \
	TEST=`echo "$@" | sed -e 's,.run-test$$,,'`; \
	chmod +x $<; \
	$(TIMEOUT) $< > $$COMPLOG 2>&1; \
	RESULT=$$?; \
	if [ $$RESULT -eq 0 ]; \
	then \
		echo "$$TEST: execution: PASS" >> $(LOGFILE);\
	else \
		( \
			echo "$$TEST: execution: FAILED: Output: ";\
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$$TEST: execution: FAILED "; \
	fi; \
	rm -f $$COMPLOG;


.PRECIOUS: %.o
%.o: %.c
	@COMPLOG=$(LOGFILE).$$$$; \
	TEST=`echo "$@" | sed -e 's,.o$$,,'`; \
	if $(COMPILE.c) $(OUTPUT_OPTION) $< >$$COMPLOG 2>&1; \
	then \
		echo "$$TEST: build: PASS" >> $(LOGFILE); \
		echo "$$TEST: build: PASS"; \
	else \
		( \
			echo "$$TEST: build: FAILED: Compiler output: "; \
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$$TEST: build: FAILED "; \
	fi; \
	rm -f $$COMPLOG;

# Functional/Stress test build and execution
functional-make:
	@for d in $(FUNCTIONAL_MAKE); do \
		$(MAKE) -C $$d; \
	done

.PHONY: $(FUNCTIONAL_RUN)

functional-run: $(FUNCTIONAL_RUN)

$(FUNCTIONAL_RUN): 
	cd $@ && ./run.sh

stress-make:
	for d in $(STRESS_MAKE); do \
		$(MAKE) -C $$d; \
	done

.PHONY: $(STRESS_RUN)

stress-run: $(STRESS_RUN)

$(STRESS_RUN): 
	cd $@; ./run.sh
