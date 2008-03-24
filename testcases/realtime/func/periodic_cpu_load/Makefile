
srcdir := ../..
include $(srcdir)/config.mk


SRCS    = $(wildcard *.c)

TARGETS = $(SRCS:%.c=%)

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i; done

clean:
	rm -f $(TARGETS) *~

