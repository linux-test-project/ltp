# Makefile: Makefile for Ballista Datatype Templates
# Copyright (C) 1998-2001  Carnegie Mellon University
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
########################
#
# File:         Makefile
#
# Description:  manage generation of datatype object files
#
# Author:       Keith Arner
#
# Reviews:
#
########################
#
# Entry Conditions: 
# 	The following environment variables are set:
#	 ${CC} : <<appropriate C compiler for target platform>>
#	 ${CFLAGS} : <<appropriate C compiler flags for target platform>>
#
#	Any other env. vars. specific to the target platform compiler
#	 should also be set.
#
# Notes:
#	The datatype object files are built from template files.  These
#	 template files are listed in the 'dataTypes' file.  All datatypes
#	 listed in that file will be built and linked into the
#	 execute_test_case executeable.  The ordering of the entries in the
#	 dataTypes file is important.  See the Design Document for details.
#
########################

#  Define suffixes to be recognized by make
.SUFFIXES: .o .tpl

# Suffix rule to build <datatype>.o from <datatype>.tpl
#  as a side effect, build <datatype>.cpp and <datatype>.h,
#  and <datatype>.param
.tpl.o :
	../paramGen $<
	../bdc_standalone $<
	${CC} ${CFLAGS} -O -c $*.cpp

# Special case rule to build bTypes.o, as there is no bTypes.tpl
bTypes.o : bTypes.cpp
	${CC} ${CFLAGS} -O -c bTypes.cpp

# remove all the object files, and all .cpp and .h files *except*
#  bTypes.cpp, bTypes.h, ptr_gen.cpp and ptr_gen.h
clean :
	rm -f *.o `ls *.cpp *.h *.param | grep -v 'bTypes\.' | grep -v 'paramAccess\.param'`
