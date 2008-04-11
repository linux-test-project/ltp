

# Check kernel/glibc support for PI and Robust mutexes and cache
# the result.
ifeq ($(shell [ -e $(srcdir)/.config ] && echo yes), yes)
	HAVE_PI_MUTEX := $(shell grep HAVE_PI_MUTEX $(srcdir)/.config | \
				 awk '{print $$2}')
	HAVE_ROBUST_MUTEX := $(shell grep HAVE_ROBUST_MUTEX $(srcdir)/.config | \
				     awk '{print $$2}')
else
	HAVE_PI_MUTEX := $(shell sh $(srcdir)/scripts/check_pi.sh)
	HAVE_ROBUST_MUTEX:= $(shell sh $(srcdir)/scripts/check_robust.sh)
	dummy := $(shell echo "HAVE_PI_MUTEX $(HAVE_PI_MUTEX)" > $(srcdir)/.config; \
		echo "HAVE_ROBUST_MUTEX $(HAVE_ROBUST_MUTEX)" >> $(srcdir)/.config)
endif

# Default stuff common to all testcases
#
CPPFLAGS += -I$(srcdir)/include -D_GNU_SOURCE
CFLAGS   += -Wall
LDLIBS   += $(srcdir)/lib/libjvmsim.a \
	   $(srcdir)/lib/librttest.a \
	   $(srcdir)/lib/libstats.a \
	   -lpthread -lrt -lm

