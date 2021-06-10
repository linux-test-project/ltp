top_srcdir		?= ../..

include $(top_srcdir)/include/mk/testcases.mk

CFLAGS			+= -W
LDLIBS			+= -lltp

tst_cleanup_once: CFLAGS += -pthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
