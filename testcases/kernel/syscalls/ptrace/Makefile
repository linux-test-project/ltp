# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2002-2023
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

# - simple_tracer is a utility that Mike Frysinger added that shouldn't be
#   compiled by default:
#
# gcc -g -O2 -g -O2 -fno-strict-aliasing -pipe -Wall
# -I//scratch/ltp-install6/include -I../../../../include
#  -L//scratch/ltp-install6/lib   simple_tracer.c  -laio -lltp -o
#  simple_tracer
#  In file included from simple_tracer.c:25:
#  syscalls.h:6:23: error: _syscalls.h: No such file or directory
#  make[4]: *** [simple_tracer] Error 1
#  make[4]: Leaving directory
#  `/scratch/ltp-dev2/ltp/testcases/kernel/syscalls/ptrace'
#  make[3]: *** [all] Error 2
#  make[3]: Leaving directory `/scratch/ltp-dev2/ltp/testcases/kernel/syscalls'
#  make[2]: *** [all] Error 2
#  make[2]: Leaving directory `/scratch/ltp-dev2/ltp/testcases/kernel'
#  make[1]: *** [all] Error 2
#  make[1]: Leaving directory `/scratch/ltp-dev2/ltp/testcases'
#  make: *** [testcases-all] Error 2
#

FILTER_OUT_MAKE_TARGETS	:= simple_tracer

include $(top_srcdir)/include/mk/generic_leaf_target.mk
