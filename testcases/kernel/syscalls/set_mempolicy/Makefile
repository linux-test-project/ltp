top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

LDFLAGS += -L$(top_builddir)/libs/libltpnuma
LDLIBS  += $(NUMA_LIBS) -lltpnuma

include $(top_srcdir)/include/mk/generic_leaf_target.mk
