# Copyright (c) 2003 by Intel Corp.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
#
# Authors:
#     Kevin Gao <kevin.gao@intel.com>

top_builddir = .

LOGFILE = $(top_builddir)/log/logfile

LDFLAGS := $(shell cat LDFLAGS | grep -v \^\#)

RUN_TESTS := $(shell $(top_builddir)/locate-test \
             --execs $(top_builddir)/$(POSIX_TARGET))
BUILD_TESTS := $(shell $(top_builddir)/locate-test \
               --buildable $(top_builddir)/$(POSIX_TARGET))
PWD := $(shell pwd)

VERBOSE = 0

all: build-tests run-tests 

build-tests: $(BUILD_TESTS:.c=.test)
run-tests: $(RUN_TESTS:.test=.run-test)

functional-tests: functional-make functional-run
stress-tests: stress-make stress-run

tests-pretty:
	$(MAKE) all | column -t -s:

CFLAGS = -g -O2 -Wall -Werror

INCLUDE = -Iinclude -I/usr/local/include/openhpi

documentation:
	make -C doc

# FIXME: exaust cmd line length
clean:
	@rm -f $(LOGFILE)
# Built runnable tests
	@find $(top_builddir) -iname \*.test | xargs -n 40 rm -f {}
	@find $(top_builddir) -iname \*~ -o -iname \*.o | xargs -n 40 rm -f {}
	make -C doc clean
	@$(foreach DIR,$(FUNCTIONAL_MAKE),make -C $(DIR) clean >> /dev/null 2>&1;) >> /dev/null 2>&1

# Rule to run a build test
# If the .o doesn't export main, then we don't need to link
.PRECIOUS: %.test
%.test: %.o
	@COMPLOG=$(LOGFILE).$$$$; \
	[ -f $< ] || exit 0; \
	nm -g --defined-only $< | grep -q " T main" || exit 0; \
	if $(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) > $$COMPLOG 2>&1; \
	then \
		echo "$(@:.test=): link: PASS" | tee -a $(LOGFILE); \
	else \
		( \
			echo "$(@:.test=): link: FAILED. Linker output: "; \
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$(@:.test=): link: FAILED "; \
		if [ $(VERBOSE) -eq 1 ]; \
		then \
			cat $$COMPLOG; \
		fi; \
	fi; \
	rm -f $$COMPLOG;

# Rule to run an executable test
# If it is only a build test, then the binary exist, so we don't need to run
.PHONY: %.run-test
%.run-test: %.test
	@COMPLOG=$(LOGFILE).$$$$; \
	[ -f $< ] || exit 0; \
	if $< > $$COMPLOG 2>&1; \
	then \
		echo "$(@:.run-test=): execution: PASS" | tee -a $(LOGFILE); \
	else \
		( \
			echo "$(@:.run-test=): execution: FAILED: Output: "; \
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$(@:.run-test=): execution: FAILED "; \
		if [ $(VERBOSE) -eq 1 ]; \
		then \
			cat $$COMPLOG; \
		fi; \
	fi; \
	rm -f $$COMPLOG;

%.run-test: %.sh
	@COMPLOG=$(LOGFILE).$$$$; \
	if $< > $$COMPLOG 2>&1; \
	then \
		echo "$(@:.run-test=): execution: PASS" | tee -a $(LOGFILE);\
	else \
		( \
			echo "$(@:.run-test=): execution: FAILED: Output: ";\
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$(@:.run-test=): execution: FAILED "; \
		if [ $(VERBOSE) -eq 1 ]; \
		then \
			cat $$COMPLOG; \
		fi; \
	fi; \
	rm -f $$COMPLOG;


.PRECIOUS: %.o
%.o: %.c
	@COMPLOG=$(LOGFILE).$$$$; \
	if $(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ $(LDFLAGS) > $$COMPLOG 2>&1; \
	then \
		echo "$(@:.o=): build: PASS" | tee -a $(LOGFILE); \
	else \
		( \
			echo "$(@:.o=): build: FAILED: Compiler output: "; \
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$(@:.o=): build: FAILED "; \
		if [ $(VERBOSE) -eq 1 ]; \
		then \
			cat $$COMPLOG; \
		fi; \
	fi; \
	rm -f $$COMPLOG;

# Functional/Stress test build and execution
functional-make:
	$(foreach DIR,$(FUNCTIONAL_MAKE),make -C $(DIR);)

.PHONY: $(FUNCTIONAL_RUN)

functional-run: $(FUNCTIONAL_RUN)

$(FUNCTIONAL_RUN): 
	cd $@; ./run.sh
	cd $(PWD)

stress-make:
	$(foreach DIR,$(STRESS_MAKE),make -C $(DIR);)

.PHONY: $(STRESS_RUN)

stress-run: $(STRESS_RUN)

$(STRESS_RUN): 
	cd $@; ./run.sh
	cd $(PWD)

