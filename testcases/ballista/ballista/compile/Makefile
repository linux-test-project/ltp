# Makefile: Makefile for Ballista Datatype Template Compiler
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

dFLAGS=-g -DDEBUG
FLAGS= -O -Wall
PFLAGS= -p -g3

CPPCOMP= ${CXX}

#
# The following is "boilerplate" to set up the standard compilation
# commands:
.SUFFIXES:
.SUFFIXES: .cpp .c .cc .h .o
.c.o:  ; ${CC} $(FLAGS) -c $*.c
.cc.o:  ; $(CPPCOMP) $(FLAGS) -c $*.cc
.cpp.o:  ; $(CPPCOMP) $(FLAGS) $(INC) -c $*.cpp
.cxx.o:  ; $(CPPCOMP) $(FLAGS) $(INC) -c $*.cxx

all: blexer bparser
	cp blexer ../blexer
	cp blexer ../templates/blexer
	cp bparser ../bparser
	cp bparser ../templates/bparser
	cp do_parse ../do_parse
	cp do_parse ../templates/do_parse
	chmod 700 ../templates/do_parse

blexer: jlist.o butil.o lex.yy.o blexer.o
	${CC} $(FLAGS)  -o blexer jlist.o butil.o lex.yy.o blexer.o 

bparser: bparser.o 
	${CXX} $(FLAGS) -o bparser bparser.o butil.c

jlist.o: jlist.c butil.h jlist.h

butil.o: butil.c butil.h

lex.yy.o: lex.yy.c

lex.yy.c: ballista.l tokens.h
	flex ballista.l

blexer.o: blexer.c blexer.h jlist.h butil.h

bparser.o: bparser.cpp tokens.h butil.h

clean:
	rm -f *.o
	rm -f lex.yy.c
	rm -f blexer
	rm -f bparser
	rm -f ../blexer ../bparser ../templates/blexer ../templates/bparser ../do_parse ../templates/do_parse















