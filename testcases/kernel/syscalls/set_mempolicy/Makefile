top_srcdir		?= ../../../..

LTPLIBS = ltpnuma

include $(top_srcdir)/include/mk/testcases.mk

LDLIBS  += $(NUMA_LIBS) -lltpnuma

include $(top_srcdir)/include/mk/generic_leaf_target.mk
