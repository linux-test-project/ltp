# MakefileHost.dist: Makefile for Ballista Test Harness
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
# Entry Conditions:
# 	The following environment variables are set:
#	 ${TARGET} : the name of the os specific Test Manager
#
########################

# compiler info for the host
CC = g++ -Wno-deprecated
CFLAGS += -w ${TARGET_DEF}
CLIBS = -lpthread -ldl -lnsl -rdynamic
TEST_MAN_FILE = selfHost
TARGET_DEF    = -DB_SELFHOST

OBJS = match.o line.o hack.o ballistaRPC.o ballistaXDR.o ${TEST_MAN_FILE}.o \
	ballistaUtil.o marshal.o testCaseIterator.o\
	serverCommunication.o parseArguments.o ballistaError.o
	

# build everything by default
all : callGen callGen_standAlone genCodeCreator replacer ballista

# run-of-of-the-mill rules for utility programs
callGen: callGen.cpp
	${CC} ${CFLAGS} callGen.cpp -o callGen

callGen_standAlone: callGen_standAlone.cpp
	${CC} ${CFLAGS} callGen_standAlone.cpp -o callGen_standAlone

genCodeCreator: genCodeCreator.cpp
	${CC} ${CFLAGS} genCodeCreator.cpp -o genCodeCreator

replacer: replacer.cpp
	${CC} ${CFLAGS} replacer.cpp -o replacer

# run-of-of-the-mill rules for harness components
hack.o : hack.cpp
	${CC} ${CFLAGS} -c $?

line.o : line.cpp
	${CC} ${CFLAGS} -c $?

ballistaRPC.o : ballistaRPC.c
	${CC} ${CFLAGS} -c $?

ballistaXDR.o : ballistaXDR.c
	${CC} ${CFLAGS} -c $?

ballistaUtil.o : ballistaUtil.cpp
	${CC} ${CFLAGS} -c $?

testCaseIterator.o : testCaseIterator.cpp 
	${CC} ${CFLAGS} -c $?

serverCommunication.o : serverCommunication.cpp 
	${CC} ${CFLAGS} -c $?

parseArguments.o : parseArguments.cpp
	${CC} ${CFLAGS} -c $?

ballistaError.o : ballistaError.cpp
	${CC} ${CFLAGS} -c $?

marshal.o : marshal.cpp
	${CC} ${CFLAGS} -c $?

match.o : match.cpp
	${CC} ${CFLAGS} -c $?

# the Test Manager is OS dependant, so is built from the ${TEST_MAN_FILE}
#  environment variable
${TEST_MAN_FILE}.o : ${TEST_MAN_FILE}.h ${TEST_MAN_FILE}.cpp
	${CC} ${CFLAGS} -c ${TEST_MAN_FILE}.cpp

# ballista is built from several static components, but the Test Manager
#  (${TEST_MAN_FILE}) is platform dependant
ballista : ballista.cpp ${OBJS}
	${CC} ${CFLAGS} ${CLIBS} ballista.cpp ${OBJS} -o ballista

# rules that are only used for testing purposes
unmarshal.o : unmarshal.cpp
	${CC} ${CFLAGS} -c $?

testmarshal.o : testmarshal.cpp 
	${CC} ${CFLAGS} -c $?

testmarshal : marshal.o unmarshal.o testmarshal.o
	${CC} marshal.o unmarshal.o testmarshal.o -o testmarshal

test_parse_arguments.o : test_parse_arguments.cpp
	${CC} ${CC_OPTS} -c $?

test_parse_arguments : test_parse_arguments.o parseArguments.o \
	ballistaUtil.o
	${CC} test_parse_arguments.o parseArguments.o ballistaUtil.o \
	-o test_parse_arguments

pause_harness : pause_harness.cpp ${OBJS}
	${CC} -Itemplates ${CFLAGS} pause_harness.cpp ${OBJS} -o pause_harness 


# clean up everything that we build
clean :
	$(MAKE) -C templates clean
	rm -f *.o ballista callGen callGen_standAlone genCodeCreator replacer testmarshal

