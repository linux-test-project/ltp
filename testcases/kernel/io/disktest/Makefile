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
# $Id: Makefile,v 1.4 2003/03/13 20:09:12 robbiew Exp $
# $Log: Makefile,v $
# Revision 1.4  2003/03/13 20:09:12  robbiew
# Applied Andreas Jaeger's patch for library linking:
# <<<<<<<<<<<<<<<<<<<<<<
# The ltp tests link files in the wrong order, this is rather important
# with static libraries - especially since glibc 2.3.2 contains a
# libpthread.so that is a linker script referencing a static library.
#
# The following is wrong:
# gcc -lm file.o -o file
# The order has to be: "file.o -lm"
#
# I'm appending a first round of patches to get this fixed.
#
# I noticed a number of failures with the threads tests and also
# problems when compiling in a static environment.
#
# I renamed LDFLAGS since those are  flags (and therefore come before
# the objects), LDLIBS should be used instead.
# >>>>>>>>>>>>>>>>>>>>>>>
#
# Revision 1.3  2003/03/13 15:34:18  robbiew
# Applied Vasan Sundar's patch:
# <<<<<<<<<<<<<<<<<<<<
# This fixes  2 problems we noticed with setting CFLAGS in the makefiles.
#
# 1. export CFLAGS from the top-level makefile so that it is available for
# the sub-makes. (We somehow missed this in our earlier patch in Jan. This
# is useful for cross-compilation)
# 2. In the lower-level makefiles, append to CFLAGS ( +=) instead of
# setting it.
# >>>>>>>>>>>>>>>>>>>>
#
# Revision 1.2  2002/05/28 14:45:38  plars
# MIPS fixes from Johannes Stezenbach:
# ltp-20020507 / current CVS does not compile on MIPS, because
# some i386 specifics were used in signal handlers in the
# kernel/mem/mtest06/mmap1.c and kernel/mem/mtest06/shmat1.c
# test cases. While I was at it I cleaned up some warnings
# regarding printf("%x") used for printing pointers,
# vs. printf("%p").
#
# Also, SIGSTKFLT and SIGUNUSED are not defined on MIPS
# (kernel/syscalls/signal/* test cases).
#
# And some test cases #include <curses.h>, which is not
# present on my (semi-embedded) MIPS system, and is unnecessary for
# the tests.
#
# Revision 1.1  2002/03/15 21:33:56  robbiew
# Forgot to add this.
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
CFLAGS+= -Wall -O -g -D"_LARGEFILE64_SOURCE" -D"_FILE_OFFSET_BITS=64"
#CC=gcc

all:	disktest
disktest: threading.o childmain.o globals.o main.o usage.o sfunc.o parse.o defs.h
		 $(CC) $(CFLAGS)  -o disktest threading.o main.o usage.o sfunc.o parse.o childmain.o globals.o -lpthread
main.o: main.c main.h sfunc.h usage.h defs.h
	$(CC) $(CFLAGS) -c -o main.o main.c
sfunc.o: sfunc.c sfunc.h main.h defs.h
	$(CC) $(CFLAGS) -c -o sfunc.o sfunc.c
usage.o: usage.c usage.h defs.h
	$(CC) $(CFLAGS) -c -o usage.o usage.c
parse.o: parse.c parse.h sfunc.h main.h defs.h
	$(CC) $(CFLAGS) -c -o parse.o parse.c
childmain.o: childmain.c childmain.h parse.h sfunc.h main.h defs.h
	$(CC) $(CFLAGS) -c -o childmain.o childmain.c
globals.o: globals.c globals.h main.h defs.h
	$(CC) $(CFLAGS) -c -o globals.o globals.c
threading.o: threading.c threading.h main.h defs.h globals.h sfunc.h childmain.h
	$(CC) $(CFLAGS) -c -o threading.o threading.c
clean:
	rm -f *~ *.o disktest 

install:
	ln -f disktest ../../../bin
