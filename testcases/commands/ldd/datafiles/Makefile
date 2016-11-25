#
#    commands/ade/ldd/datafiles testcases Makefile.
#
#    Copyright (C) 2014, Linux Test Project.
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

CPPFLAGS		+= -fpic

INSTALL_TARGETS		:= ldd*.so lddfile.out

LDD_C_FILES		:= $(wildcard $(abs_srcdir)/lddfile*.c)
LDD_SO_FILES		:= $(patsubst $(abs_srcdir)/%.c,%.obj.so,$(LDD_C_FILES))
MAKE_TARGETS		:= lddfile.out
CLEAN_TARGETS		+= *.obj $(LDD_SO_FILES)

%.obj.so: %.o
	$(CC) $(CFLAGS) -shared -o $@ $^

lddfile.out: main.o $(LDD_SO_FILES)
	$(CC) $(CFLAGS) -O -o $@ $?

INSTALL_DIR		:= testcases/data/ldd01

include $(top_srcdir)/include/mk/generic_leaf_target.mk
