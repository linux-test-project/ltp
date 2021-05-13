# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2012, Linux Test Project.

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

CPPFLAGS		+= -DNUMA_VERSION1_COMPATIBILITY \
			   -I$(abs_srcdir)/../include \
			   -I$(abs_builddir)/../include
INTERNAL_LIB		:= libkerntest.a

include $(top_srcdir)/include/mk/lib.mk
