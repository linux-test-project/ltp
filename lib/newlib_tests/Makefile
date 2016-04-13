top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

CFLAGS			+= -W -Wall
LDLIBS			+= -lltp

test08: CFLAGS+=-pthread
test09: CFLAGS+=-pthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
