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
# $Id: Makefile,v 1.1 2002/03/15 21:33:56 robbiew Exp $
# $Log: Makefile,v $
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
CFLAGS= -Wall -O -g -D"_LARGEFILE64_SOURCE" -D"_FILE_OFFSET_BITS=64"
#CC=gcc

all:	threading.o childmain.o globals.o main.o usage.o sfunc.o parse.o defs.h
	$(CC) $(CFLAGS) -lpthread -o disktest threading.o main.o usage.o sfunc.o parse.o childmain.o globals.o
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
