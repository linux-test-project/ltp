#
#   Copyright (c) International Business Machines  Corp., 2004
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#
#	Makefile
#

# dmapi_imp is platform for DMAPI implementation (currently supported: JFS)
dmapi_imp = JFS
# dmapi_lib is path of user space library for DMAPI implementation
dmapi_lib = /usr/lib/libjfsdm.a

std_cflags = -I. -g -Wall -D$(dmapi_imp)

#	Define USER_SPACE_FAULTS if DMAPI implementation handles user space
#	  faults (JFS has plenty of handle code in user space, which causes
#	  faults when invalid pointers are passed in)	
#	Define DIRECTORY_LINKS if DMAPI FS supports links on directories (JFS
#	  does not)
#	Define INTERIOR_HOLES if DMAPI implementation supports probing/punching
#	  interior holes (JFS does); if not defined, only probing/punching 
#	  holes to end-of-file is supported
#	Define MULTIPLE_REGIONS if DMAPI implementation supports multiple
#	  interior regions (JFS does); if not defined, only one region
#	  covering entire file is supported
#extra_cflags = -DUSER_SPACE_FAULTS
#extra_cflags = -DDIRECTORY_LINKS
#extra_cflags = -DINTERIOR_HOLES
#extra_cflags = -DMULTIPLE_REGIONS
extra_cflags = -DINTERIOR_HOLES -DMULTIPLE_REGIONS

cflags = $(std_cflags) $(extra_cflags)

lflags = $(dmapi_lib)
lflags_thd = $(dmapi_lib) -lpthread

progs = handle session event_sn event_sd event_an pmr_pre pmr_post event_am \
	  hole invis attr event_us disp config objref mount token right mmap \
	  mmapfile event

all	: $(progs)

dm_test.h	: dm_vars.h Makefile
	touch dm_test.h

dm_test.o	: dm_test.c dm_test.h dm_impl.h Makefile
	cc -c $(cflags) dm_test.c

handle	: handle.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o handle handle.c dm_test.o $(lflags)

session	: session.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o session session.c dm_test.o $(lflags)

event_sn: event_sn.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o event_sn event_sn.c dm_test.o $(lflags_thd)

event_sd: event_sd.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o event_sd event_sd.c dm_test.o $(lflags_thd)

event_an: event_an.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o event_an event_an.c dm_test.o $(lflags_thd)

pmr_pre : pmr_pre.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o pmr_pre pmr_pre.c dm_test.o $(lflags_thd)

pmr_post: pmr_post.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o pmr_post pmr_post.c dm_test.o $(lflags_thd)

event_am: event_am.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o event_am event_am.c dm_test.o $(lflags_thd)

hole	: hole.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o hole hole.c dm_test.o $(lflags)

invis	: invis.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o invis invis.c dm_test.o $(lflags_thd)

attr	: attr.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o attr attr.c dm_test.o $(lflags)

event_us: event_us.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o event_us event_us.c dm_test.o $(lflags_thd)

disp	: disp.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o disp disp.c dm_test.o $(lflags_thd)

config	: config.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o config config.c dm_test.o $(lflags)

objref	: objref.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o objref objref.c dm_test.o $(lflags_thd)

mount	: mount.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o mount mount.c dm_test.o $(lflags_thd)

token	: token.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o token token.c dm_test.o $(lflags_thd)

right	: right.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o right right.c dm_test.o $(lflags_thd)

mmap	: mmap.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o mmap mmap.c dm_test.o $(lflags_thd)

mmapfile: mmapfile.c dm_test.h Makefile
	cc $(cflags) -o mmapfile mmapfile.c $(lflags)

event	: event.c dm_test.h Makefile dm_test.o
	cc $(cflags) -o event event.c dm_test.o $(lflags_thd)

clean	: 
	rm -f *.o
	rm -f $(progs)
