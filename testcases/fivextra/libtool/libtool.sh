#! /bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2004						      ##
##									      ##
## This program is free software;  you can redistribute it or modify	      ##
## it under the terms of the GNU General Public License as published by	      ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
# File :	libtool.sh
#
# Description: shell script for libtool package test
#
# Author:	CSDL
#
#
# History:	2004-6-30  CREATE	CSDL
#		2004-7-1   ENHANCED CSDL  enhance test07, add test08, test09 test10
#
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################
REQUIRED="mkdir ldd grep"
INSTALLED="libtool"

CURDIR=`pwd`

LIBTOOL=`which libtool`
GREP=`which grep`
LDD=`which ldd`
MKDIR=`which mkdir`

GCC=`< FIV_CC_NAME.txt`
echo GCC=$GCC

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_exec_or_break $REUIRED
	tc_fail_if_bad $? "require tools to do test" || exit

	tc_exec_or_break $INSTALLED
	tc_fail_if_bad $? "package libtool not properly installed" || exit

	cp -f libtool_demo/install-sh $TCTMP/
	cp -f libtool_demo/foo.h $TCTMP/
	cp -f libtool_demo/foo.c $TCTMP/
	cp -f libtool_demo/hello.c $TCTMP/
	cp -f libtool_demo/main.c $TCTMP/

	cd $TCTMP
	[ -f install-sh ] && [ -f foo.h ] && [ -f foo.c ] && \
	[ -f hello.c ] && [ -f main.c ]
	tc_pass_or_fail $? "source files for test not found"
}

################################################################################
# local utility functions
################################################################################

#
# test01: creating objects
#
function test01(){
	TST_TOTAL=1
	tc_register "libtool: creating object: foo.c --> foo.o"
	$LIBTOOL --mode=compile $GCC -g -O -c foo.c >$stdout 2>$stderr
	tc_pass_or_fail $? "compile foo.c failed"

	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: creating object: hello.c --> hello.o"
	$LIBTOOL --mode=compile $GCC -g -O -c hello.c >$stdout 2>$stderr
	tc_pass_or_fail $? "compile hello.c failed"

	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: creating object: main.c --> main.o"
	$LIBTOOL --mode=compile $GCC -g -O -c main.c >$stdout 2>$stderr
	tc_pass_or_fail $? "compile main.c failed"
}

#
# test02: linking libraries
#
function test02(){
	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: linking library: foo.lo, hello.lo --> libhello.la"
	$LIBTOOL --mode=link $GCC -g -O -o libhello.la foo.lo hello.lo -rpath $TCTMP/libs -lm >$stdout 2>$stderr
	tc_pass_or_fail $? "linking library libhello.la failed"
}

#
# test03: linking executables
#
function test03(){
	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: linking executable: main.o, libhello.la --> hell"
	$LIBTOOL --mode=link $GCC -g -O -o hell main.o libhello.la -lm >$stdout 2>$stderr
	tc_fail_if_bad $? "linking executable hell failed"

	./hell >$stdout 2>$stderr
	$GREP ".*This is not GNU Hello.*" $stdout >/dev/null 2>&1
	tc_fail_if_bad $? "excute hell result in unexpected output"
}

#
# test05: installing libraries
#
function test05(){
	$MKDIR -p $TCTMP/libs

	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: installing library: libhello.la --> libs/libhello.la"
	$LIBTOOL --mode=install cp libhello.la $TCTMP/libs/libhello.so >$stdout 2>$stderr
	tc_pass_or_fail $? "install library libs/libhello.la failed"
}

#
# test06: installing executables
#
function test06(){
	$MKDIR -p $TCTMP/bin

	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: installing executable: hell --> bin/hell"
	$LIBTOOL install -c hell $TCTMP/bin/hell >$stdout 2>$stderr
	tc_fail_if_bad $? "install executable bin/hell failed"

	$LDD $TCTMP/bin/hell >$stdout 2>$stderr
	$GREP ".*libhello.so.*=>.*$TCTMP/libs/libhello.so.*" $stdout >/dev/null 2>&1
	tc_pass_or_fail $? "excutable bin/hell linking error"
}

#
# test07: linking static libraries
#
function test07(){
	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: linking static library: libhello.la --> libs/libhello.a"
	$LIBTOOL --mode=install ./install-sh -c libhello.la $TCTMP/libs/libhello.a >$stdout 2>$stderr
	tc_fail_if_bad $? "linking static library failed" || return

	$LIBTOOL --mode=link $GCC -o hell-static main.o libs/libhello.a -lm >$stdout 2>$stderr
	tc_fail_if_bad $? "linking static library to executable failed" || return

	$LDD $TCTMP/hell-static >$stdout 2>$stderr
	$GREP ".*libhello.*=>.*$TCTMP/libs/libhello.*" $stdout >/dev/null 2>&1
	[ $? -ne 0 ]
	tc_pass_or_fail $? "libhello should not appear as a shared object"
}

#
# test08: complete library installing
#
function test08(){
	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: finish library installing"
	$LIBTOOL --mode=finish $TCTMP/libs >$stdout 2>$stderr
	tc_fail_if_bad $? "complete library installing failed" || return

	$GREP ".*ldconfig.*$TCTMP/libs.*" $stdout >/dev/null 2>&1
	tc_pass_or_fail $? "$TCTMP/libs failed to be configured to dynamic linker run time bindings"
}

#
# test09: uninstall objects
#
function test09(){
	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: uninstall foo.o hello.o main.o"
	$LIBTOOL --mode=uninstall rm -f foo.o hello.o main.o >$stdout 2>$stderr
	tc_pass_or_fail $? "uninstall obsolete objects failed"
}

#
# test10: clean libraries
#
function test10(){
	let TST_TOTAL=$TST_TOTAL+1
	tc_register "libtool: clean libhello.la"
	$LIBTOOL --mode=clean rm -f libhello.la >$stdout 2>$stderr
	tc_pass_or_fail $? "clean up failed"
}

################################################################################
# main
################################################################################
tc_setup
test01 &&
test02 &&
test03 &&
test05 &&
test06 &&
test07 &&
{ test08; test09; test10; }
