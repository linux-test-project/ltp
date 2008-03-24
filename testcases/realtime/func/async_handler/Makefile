
srcdir := ../..
include $(srcdir)/config.mk


SRCS    = $(wildcard *.c)

ifeq ($(HAVE_PI_MUTEX), yes)
TARGETS = $(SRCS:%.c=%)
else
TARGETS =
endif

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i; done

clean:
	rm -f $(TARGETS) *~

