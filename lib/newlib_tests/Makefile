top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

CFLAGS			+= -W -Wall
LDLIBS			+= -lltp

test08: CFLAGS+=-pthread
test09: CFLAGS+=-pthread
test15: CFLAGS+=-pthread
test16: CFLAGS+=-pthread
test16: LDLIBS+=-lrt
tst_expiration_timer: LDLIBS+=-lrt

ifeq ($(ANDROID),1)
FILTER_OUT_MAKE_TARGETS	+= test08
endif


include $(top_srcdir)/include/mk/generic_leaf_target.mk
