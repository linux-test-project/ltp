#
#    Copyright (C) 2016, Linux Test Project.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/env_pre.mk
INSTALL_DIR		:= testcases/data/nm01
INSTALL_TARGETS		:= f1 f2 f3 lib.a
MAKE_TARGETS		+= f1 f2 f3 lib.a

lib.a: f1.o f2.o f3.o
	$(AR) rcs $@ $?

include $(top_srcdir)/include/mk/generic_leaf_target.mk
