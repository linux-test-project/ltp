#
#  Copyright (c) Red Hat Inc., 2008
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

# Author: Masatake YAMATO <yamato@redhat.com>
# Technique used here is suggested by Ngie Cooper <yaneurabeya@gmail.com>

# This file does the same things on foo64 system call
# as compat_16.mk does on foo16. See both compat_16.mk
# and Makefile for fadvise test case.

CPPFLAGS		+= -I$(abs_srcdir) -I$(abs_srcdir)/../utils

SRCS			?= $(wildcard $(abs_srcdir)/*.c)

MAKE_TARGETS		:= $(notdir $(patsubst %.c,%,$(SRCS)))

ifneq ($(TST_NEWER_64_SYSCALL),no)
MAKE_TARGETS		+= $(addsuffix _64,$(MAKE_TARGETS))
endif

# XXX (garrcoop): This code should be put in question as it cannot be applied
# (no .h file, no TST_USE_NEWER64_SYSCALL def).
DEF_64			:= TST_USE_NEWER64_SYSCALL

NEWER_64_H		:= $(abs_srcdir)/../utils/newer_64.h

ifneq ($(wildcard $(NEWER_64_H)),)
HAS_NEWER_64		:= 1

%.c: $(NEWER_64_H)

else
HAS_NEWER_64		:= 0
endif

%_64: CFLAGS += -D$(DEF_64)=1
# XXX (garrcoop): End section of code in question..

%_64.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<
