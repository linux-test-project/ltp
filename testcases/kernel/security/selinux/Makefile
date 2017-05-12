top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

INSTALL_TARGETS		:= testall.sh

LDLIBS			+= $(SELINUX_LIBS)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
