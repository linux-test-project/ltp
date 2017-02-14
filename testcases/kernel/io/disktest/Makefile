#
# Disktest Makefile
# Copyright (c) International Business Machines Corp., 2001
#
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#  Please send e-mail to yardleyb@us.ibm.com if you have
#  questions or comments.
#
#  Project Website:  TBD
#
#
# $Id: Makefile,v 1.13 2009/11/06 20:07:30 vapier Exp $
# $Log: Makefile,v $
# Revision 1.13  2009/11/06 20:07:30  vapier
# drop adding of -O/-W type flags that are already handled in common .mk files
#
# for the few makefiles that arent yet converted to the .mk infrastructure:
# Lately gcc developers introduced -Wextra flag that does the same as -W but is
# more descriptive. According to this using -W flag should be safe (as gcc is
# backward compatlible) but using -Wextra is limited to newer gcc releases.
#
# Attached patch replaces all -Wextra occurences with -W and thus fixes
# compliation failures with older gcc (mine was gcc-3.3.3 on sles).
#
# patch by Cyril Hrubis
#
# Revision 1.12  2009/10/09 17:55:48  yaberauneya
# 1. Please see README.mk-devel for a full description of the changes
# from a Make perspective.
# 2. Several files were changed to accomodate correct installation
# practices, most notably ones in testcases/network/{ipv6,tcp_cmds},
# testcases/kernel/sched/hyperthreading/ht_enabled/..., and some items
# in tools/..., and also to avoid collisions as far as installed
# testcases (scripts, compiled C apps) were concerned.
# 3. Several apps weren't autoconf safe and some autoconf tests and
# conditional statements have been placed in sourcecode and in Makefiles
# to either a) prevent the tests from being built / installed or b) turn
# the tests into dummy apps which print out TCONF messages due to
# lack-of-build support.
#
# Signed-off-by: Ngie Cooper <yaneurabeya@gmail.com>
#
# Revision 1.11  2008/02/14 08:22:22  subrata_modak
# Disktest application update to version 1.4.2, by, Brent Yardley <yardleyb@us.ibm.com>
#
# Revision 1.31  2008/02/07 15:43:37  yardleyb
# Applied patch from Mike Frysinger to ensure Makefile properly respect CFLAGS/LDFLAGS
#
# Revision 1.30  2006/10/19 20:01:48  yardleyb
# changes made to support rpm builds
#
# Revision 1.29  2006/10/19 17:30:27  yardleyb
# Added basic signal handling.
#
# Revision 1.28  2006/05/12 19:44:47  yardleyb
# added CHANGELOG to distro
# uninstall does not force a build
#
# Revision 1.27  2005/10/12 23:13:35  yardleyb
# Updates to code to support new function in disktest version 1.3.x.
# Actual changes are recorded in the README
#
# Revision 1.26  2004/12/18 06:13:03  yardleyb
# Updated timer schema to more accurately use the time options.  Added
# fsync on write option to -If.
#
# Revision 1.25  2004/12/17 06:34:56  yardleyb
# removed -mf -ml.  These mark options cause to may issues when using
# random block size transfers.  Fixed -ma option for endian-ness.  Fixed
# false data misscompare during multiple cycles.
#
# Revision 1.24  2004/11/20 04:43:42  yardleyb
# Minor code fixes.  Checking for alloc errors.
#
# Revision 1.23  2004/11/19 21:45:12  yardleyb
# Fixed issue with code added for -F option.  Cased disktest
# to SEG FAULT when cleaning up threads.
#
# Revision 1.22  2004/11/19 03:47:45  yardleyb
# Fixed issue were args data was not being copied from a
# clean source.
#
# Revision 1.21  2004/11/02 20:47:13  yardleyb
# Added -F functions.
# lots of minor fixes. see README
#
# Revision 1.20  2003/09/12 21:23:56  yardleyb
# Updated version to 1.12
#
# Revision 1.19  2003/01/13 21:33:31  yardleyb
# Added code to detect AIX volume size.
# Updated mask for random LBA to use start_lba offset
# Updated version to 1.1.10
#
# Revision 1.18  2002/05/31 18:48:50  yardleyb
# Updated version to 1.1.9
#
# Revision 1.17  2002/04/24 01:59:45  yardleyb
# Update to version v1.1.3
#
# Revision 1.16  2002/04/02 00:11:04  yardleyb
# Corrected -D for each OS type
#
# Revision 1.15  2002/04/01 20:05:26  yardleyb
# Modified Makefiles for linux,
# Created Makefiles for windows/aix
#
# Revision 1.14  2002/03/07 03:38:52  yardleyb
# Added dump function from command
# line.  Created formatted dump output
# for Data miscomare and command line.
# Can now leave off filespec the full
# path header as it will be added based
# on -I.
#
# Revision 1.13  2002/02/28 04:25:45  yardleyb
# reworked threading code
# made locking code a macro.
#
# Revision 1.12  2002/02/26 19:35:59  yardleyb
# Updates to parsing routines for user
# input.  Added multipliers for -S and
# -s command line arguments. Forced
# default seeks to default if performing
# a diskcache test.
#
# Revision 1.11  2002/02/21 21:42:15  yardleyb
# Updated distro for man1
#
# Revision 1.10  2002/02/21 21:34:16  yardleyb
# Cleaned up make dependancies
# added install and uninstall
#
# Revision 1.9  2002/02/21 01:00:50  yardleyb
# Added README and directory
# structure to distro
#
# Revision 1.8  2002/02/19 02:46:37  yardleyb
# Added changes to compile for AIX.
# Update getvsiz so it returns a -1
# if the ioctl fails and we handle
# that fact correctly.  Added check
# to force vsiz to always be greater
# then stop_lba.
#
# Revision 1.7  2001/12/04 19:00:33  yardleyb
# Updated to add new files and
# filename changes
#
# Revision 1.6  2001/10/10 00:17:14  yardleyb
# Added Copyright and GPL license text.
# Miner bug fixes throughout text.
#
# Revision 1.5  2001/09/22 03:29:51  yardleyb
# Added dependence on main.o for sfunc.h usage.h header files
#
# Revision 1.4  2001/09/10 22:14:27  yardleyb
# Added clean up for tar file. Included man page in distro
#
# Revision 1.3  2001/09/06 18:23:30  yardleyb
# Added duty cycle -D.  Updated usage. Added
# make option to create .tar.gz of all files
#
# Revision 1.2  2001/09/05 22:44:42  yardleyb
# Split out some of the special functions.
# added O_DIRECT -Id.  Updated usage.  Lots
# of clean up to functions.  Added header info
# to pMsg.
#
# Revision 1.1  2001/09/04 19:28:07  yardleyb
# Split usage out. Split header out.  Added usage text.
# Made signal handler one function. code cleanup.
#

# 1. -D_LARGE_FILES is used in AIX to support 64bit functions and data types.
# 2. For Linux:
#     i.  -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 is used in to support
#         64-bit functions and data types.
#     ii. -D_GNU_SOURCE is to support Linux O_DIRECT.
#
#    i. and ii. are typically taken from rpm, but, if not, are defined here.
#

top_srcdir	?= ../../../..

include $(top_srcdir)/include/mk/env_pre.mk

VER		:= `grep VER_STR $(abs_srcdir)/main.h | awk -F\" '{print $$2}'`

CPPFLAGS	+= -DLINUX -D_THREAD_SAFE -D_GNU_SOURCE -D_LARGE_FILES -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 $(RPM_OPT_FLAGS)

DISTRO_FILES	:= Makefile* *.[ch] LICENSE README CHANGELOG

LDLIBS		+= -lpthread

MAKE_TARGETS	:= disktest

OBJS		:= $(patsubst $(abs_srcdir)/%.c,%.o,$(wildcard $(abs_srcdir)/*.c))

disktest-$(VER):
	mkdir -p "$@"

disktest: $(OBJS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

all-clean: clean
	$(RM) -rf *~ *tar* *zip* disktest-$(VER)

distro: all-clean | disktest-$(VER)
	mkdir -p disktest-$(VER)/man1
	cp $(addprefix $(abs_srcdir)/,$(DISTRO_FILES)) disktest-$(VER)/.
	install -m 00444 $(abs_srcdir)/man1/disktest.1 disktest-$(VER)/man1
	gzip disktest-$(VER)/man1/disktest.1
	tar -cvzf disktest-$(VER).tar.gz disktest-$(VER)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
