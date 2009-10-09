#
#  Copyright (c) International Business Machines  Corp., 2001
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

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

# ptrace06 is a broken test ; it hangs the target consistently, chewing up CPU.
# See: `Issue 3 - ptrace06 hung for 7 hours' --
# http://sourceforge.net/mailarchive/forum.php?thread_name=364299f40910062300s65c43c93w9cccdfe8835c2334%40mail.gmail.com&forum_name=ltp-list

MAKE_TARGETS		:= $(filter-out ptrace06,$(patsubst $(abs_srcdir)/%.c,%,$(wildcard $(abs_srcdir)/*.c)))

include $(top_srcdir)/include/mk/generic_leaf_target.mk
