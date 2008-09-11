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
# Technique used here is suggested by Garrett Cooper <yanegomi@gmail.com>

# This file does the same things on foo64 system call 
# as compat_16.mk does on foo16. See both compat_16.mk
# and Makefile for fadvise test case.

TARGETS_64 = $(patsubst %.c,%_64,$(SRCS))

ifneq ($(TST_NEWER_64_SYSCALL),no)
TARGETS  +=  $(TARGETS_64)
endif

DEF_64 = TST_USE_NEWER64_SYSCALL
NEWER_64_H = newer_64.h
HAS_NEWER_64 := $(shell if [ -f $(NEWER_64_H) ]; then	\
	  			echo yes;		\
			else				\
				echo no;		\
			fi)


ifeq ($(HAS_NEWER_64),yes)
%.c: $(NEWER_64_H)
endif

%_64.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<
%_64: CFLAGS += -D$(DEF_64)=1
