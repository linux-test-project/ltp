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
TIMEOUT_RET = $(shell cat $(top_builddir)/t0.val)

top_builddir = .

LOGFILE = $(top_builddir)/logfile

LDFLAGS := $(shell cat LDFLAGS | grep -v \^\#)

RUN_TESTS := $(shell $(top_builddir)/locate-test \
             --execs $(top_builddir)/$(POSIX_TARGET))
BUILD_TESTS := $(shell $(top_builddir)/locate-test \
               --buildable $(top_builddir)/$(POSIX_TARGET))
FUNCTIONAL_MAKE := $(shell $(top_builddir)/locate-test --fmake)
FUNCTIONAL_RUN := $(shell $(top_builddir)/locate-test --frun)
STRESS_MAKE := $(shell $(top_builddir)/locate-test --smake)
STRESS_RUN := $(shell $(top_builddir)/locate-test --srun)
PWD := $(shell pwd)
TIMEOUT = $(top_builddir)/t0 $(TIMEOUT_VAL)


all: build-tests run-tests 

build-tests: $(BUILD_TESTS:.c=.test)
run-tests: $(RUN_TESTS:.test=.run-test)

functional-tests: functional-make functional-run
stress-tests: stress-make stress-run

tests-pretty:
	$(MAKE) all | column -t -s:

CFLAGS = -g -O2 -Wall -Werror -D_POSIX_C_SOURCE=200112L

# add -std=c99, -std=gnu99 if compiler supports it (gcc-2.95.3 does not).
check_gcc = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi)
CFLAGS += $(call check_gcc,-std=c99,)
CFLAGS += $(call check_gcc,-std=gnu99,)

INCLUDE = -Iinclude

# FIXME: exaust cmd line length
clean:
	@rm -f $(LOGFILE)
# Timeout helper files
	@rm -f $(top_builddir)/t0{,.val}
# Built runnable tests
	@find $(top_builddir) -iname \*.test | xargs -n 40 rm -f {}
	@find $(top_builddir) -iname \*~ -o -iname \*.o | xargs -n 40 rm -f {}
	@$(foreach DIR,$(FUNCTIONAL_MAKE),make -C $(DIR) clean >> /dev/null 2>&1;) >> /dev/null 2>&1

# Rule to run a build test
# If the .o doesn't export main, then we don't need to link
.PRECIOUS: %.test
%.test: %.o
	@COMPLOG=$(LOGFILE).$$$$; \
	[ -f $< ] || exit 0; \
	{ nm -g $< | grep -q " T main"; } || \
	{ echo "$(@:.test=): link: SKIP" | tee -a $(LOGFILE) && exit 0; }; \
	if $(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) > $$COMPLOG 2>&1; \
	then \
		echo "$(@:.test=): link: PASS" | tee -a $(LOGFILE); \
	else \
		( \
			echo "$(@:.test=): link: FAILED. Linker output: "; \
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$(@:.test=): link: FAILED "; \
	fi; \
	rm -f $$COMPLOG;

# Rule to run an executable test
# If it is only a build test, then the binary exist, so we don't need to run
.PHONY: %.run-test
%.run-test: %.test $(top_builddir)/t0 $(top_builddir)/t0.val
	@COMPLOG=$(LOGFILE).$$$$; \
	[ -f $< ] || exit 0; \
	$(TIMEOUT) $< > $$COMPLOG 2>&1; \
	RESULT=$$?; \
	if [ $$RESULT -eq 1 ]; \
	then \
		MSG="FAILED"; \
	fi; \
	if [ $$RESULT -eq 2 ]; \
	then \
		MSG="UNRESOLVED"; \
	fi; \
	if [ $$RESULT -eq 4 ]; \
	then \
		MSG="UNSUPPORTED"; \
	fi; \
	if [ $$RESULT -eq 5 ]; \
	then \
		MSG="UNTESTED"; \
	fi; \
	if [ $$RESULT -eq $(TIMEOUT_RET) ]; \
	then \
		MSG="HUNG"; \
	fi; \
	if [  $$RESULT -gt 5  -a  $$RESULT -ne $(TIMEOUT_RET)  ]; \
	then \
		MSG="INTERRUPTED"; \
	fi; \
	if [ $$RESULT -eq 0 ]; \
	then \
		echo "$(@:.run-test=): execution: PASS" | tee -a $(LOGFILE); \
	else \
		( \
			echo "$(@:.run-test=): execution: $$MSG: Output: "; \
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$(@:.run-test=): execution: $$MSG "; \
	fi; \
	rm -f $$COMPLOG;

$(top_builddir)/t0: $(top_builddir)/t0.c
	@echo Building timeout helper files; \
	$(CC) -O2 -o $@ $<
	
$(top_builddir)/t0.val: $(top_builddir)/t0
	echo `$(top_builddir)/t0 0; echo $$?` > $(top_builddir)/t0.val
	
%.run-test: %.sh $(top_builddir)/t0 $(top_builddir)/t0.val
	@COMPLOG=$(LOGFILE).$$$$; \
	chmod +x $<; \
	$(TIMEOUT) $< > $$COMPLOG 2>&1; \
	RESULT=$$?; \
	if [ $$RESULT -eq 0 ]; \
	then \
		echo "$(@:.run-test=): execution: PASS" | tee -a $(LOGFILE);\
	else \
		( \
			echo "$(@:.run-test=): execution: FAILED: Output: ";\
			cat $$COMPLOG; \
		) >> $(LOGFILE); \
		echo "$(@:.run-test=): execution: FAILED "; \
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

