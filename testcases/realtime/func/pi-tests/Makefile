
srcdir := ../..
include $(srcdir)/config.mk

ALL_TARGETS = testpi-0 testpi-1 testpi-2 testpi-4 testpi-5 testpi-6 testpi-7 \
	      sbrk_mutex

TARGETS = testpi-0

ifeq ($(HAVE_PI_MUTEX), yes)
TARGETS += testpi-1 testpi-2 testpi-4 testpi-5 testpi-7
endif

ifeq ($(HAVE_ROBUST_MUTEX), yes)
TARGETS += testpi-6 sbrk_mutex
endif

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i; done

clean:
	rm -f $(ALL_TARGETS) *~

