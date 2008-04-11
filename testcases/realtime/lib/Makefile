
srcdir := ..
include $(srcdir)/config.mk


AR ?= ar
RANLIB ?= ranlib


SRCS	= libjvmsim.c librttest.c libstats.c
OBJS	= $(SRCS:%.c=%.o)
TARGETS	= $(SRCS:%.c=%.a)

all: $(TARGETS)

%.a: %.o
	@rm -f $@
	$(AR) cru $@ $<
	$(RANLIB) $@

install:

clean:
	rm -f $(TARGETS) $(OBJS) *~
