#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2004						      ##
##									      ##
## This program is free software;  you can redistribute it or modify          ##
## it under the terms of the GNU General Public License as published by       ##
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
#
# File :	texinfo.sh
#
# Description:  test the functionality of texinfo
#
# Author:	CSDL
#

##############################################
#             VERSION 1.1(revised)           #
##############################################
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
REQIRED="troff diff"
INSTALLED="makeinfo texi2dvi texi2html texi2index texi2roff"

TEXIFILE="texinfo_files/sample.texi"
INFOFILE="texinfo_files/sample.info"
TEXTFILE="texinfo_files/sample.txt"
HTMLFILE="texinfo_files/sample.html"
DVIFILE="texinfo_files/sample.dvi"
PDFFILE="texinfo_files/sample.pdf"
XMLFILE="texinfo_files/sample.xml"
DOCBOOKFILE="texinfo_files/sample-docbook.xml"
HTMLFILE1="texinfo_files/sample-1.html"
TROFFFILE="texinfo_files/sample.troff"
INDEXFILE="texinfo_files/sample.index"

MAKEINFO=`which makeinfo`
TEXI2DVI=`which texi2dvi`
TEXI2HTML=`which texi2html`
TEXI2INDEX=`which texi2index`
TEXI2ROFF=`which texi2roff`
TROFF=`which troff`
DIFF=`which diff`
DIFFOPTS="-iwE"

################################################################################
# utility functions
################################################################################

#
# Setup specific to this test
#
function tc_local_setup()
{
	tc_exec_or_break $REQUIRED  || exit
	tc_exec_or_break $INSTALLED || exit
}

################################################################################
# testcase functions
################################################################################

#
# test01: convert texi to info file
#
function test01()
{
	tc_register "makeinfo: texi --> info"
	$MAKEINFO --fill-column 1000 $TEXIFILE -o $TCTMP/sample.info >$stdout 2>$stderr
	tc_fail_if_bad  $? "error occured during convert" || return

	$DIFF $DIFFOPTS -I ".*produced by makeinfo version .* from.*" -I "Node:.*" \
		$INFOFILE $TCTMP/sample.info >$stdout 2>$stderr
	tc_pass_or_fail $? "output info is different from expected"
}

#
# test02: convert texi to plain text file
#
function test02()
{
	tc_register "makeinfo: texi --> plain text"
	$MAKEINFO --fill-column 1000 --no-headers $TEXIFILE -o $TCTMP/sample.txt >$stdout 2>$stderr
	tc_fail_if_bad  $? "error occured during convert" || return

	$DIFF $DIFFOPTS $TEXTFILE $TCTMP/sample.txt >$stdout 2>$stderr
	tc_pass_or_fail $? "output text is different from expected"
}

#
# test03: convert texi to html files
#
function test03()
{
	tc_register "makeinfo: texi --> html"
	$MAKEINFO --html --no-split $TEXIFILE -o $TCTMP/sample.html >$stdout 2>$stderr
	tc_fail_if_bad  $? "error occured during convert" || return

	$DIFF $DIFFOPTS $HTMLFILE $TCTMP/sample.html >$stdout 2>$stderr
	tc_pass_or_fail $? "output html is different from expected"
}

#
# test04: convert texi to DVI file
#

#
# test05: convert texi to PDF file
#

#
# test06: convert texi to xml file
#
function test06()
{
	tc_register "makeinfo: texi --> xml"
	$MAKEINFO --xml $TEXIFILE -o $TCTMP/sample.xml >$stdout 2>$stderr
	tc_fail_if_bad  $? "error occured during convert" || return

	$DIFF $DIFFOPTS -I "<setfilename>.*</setfilename>" \
		$XMLFILE $TCTMP/sample.xml >$stdout 2>$stderr
	tc_pass_or_fail $? "output xml is different from expected"
}

#
# test07: convert texi to docbook file
#
function test07()
{
	tc_register "makeinfo: texi --> docbook"
	$MAKEINFO --docbook $TEXIFILE -o $TCTMP/sample-docbook.xml >$stdout 2>$stderr
	tc_fail_if_bad  $? "error occured during convert" || return

	$DIFF $DIFFOPTS -I "<!.*>" \
		$DOCBOOKFILE $TCTMP/sample-docbook.xml >$stdout 2>$stderr
	tc_pass_or_fail $? "output docbook is different from expected"
}

#
# test08: convert texi to html file
#
function test08()
{
	tc_register "texi2html: texi --> html"
	$TEXI2HTML $TEXIFILE -o $TCTMP/sample-1.html >$stdout 2>$stderr
	tc_fail_if_bad  $? "error occured during convert" || return

	$DIFF $DIFFOPTS -I "<!.*>" -I "by <I>.*</I>" \
		$HTMLFILE1 $TCTMP/sample-1.html >$stdout 2>$stderr
	tc_pass_or_fail $? "output html is different from expected"
}

#
# test09: convert texi to troff and index file
#
function test09()
{
	tc_register "texi2roff: texi --> troff"
	$TEXI2ROFF -i -ms $TEXIFILE 2>$stderr | $TROFF -ms >$TCTMP/sample.troff 2>$TCTMP/index
	[ -s $stderr ] && \
	{
		tc_warn "mistakes occured during convert";
		echo "================== stderr follows ==================";
		cat $stderr;
		echo "=================== stderr above ===================";
	}
	$DIFF $DIFFOPTS $TROFFFILE $TCTMP/sample.troff >$stdout 2>$stderr
	tc_pass_or_fail $? "output index is different from expected"

	tc_register "texi2index: texi --> index"
	$TEXI2INDEX $TCTMP/index | $TROFF -ms >$TCTMP/sample.index 2>$stderr
	[ -s $stderr ] && \
	{
		tc_warn "mistakes occured during convert";
		echo "================== stderr follows ==================";
		cat $stderr;
		echo "=================== stderr above ===================";
	}
	$DIFF $DIFFOPTS $INDEXFILE $TCTMP/sample.index >$stdout 2>$stderr
	tc_pass_or_fail $? "output index is different from expected"
}

################################################################################
# MAIN
################################################################################
TST_TOTAL=8
tc_setup

test01
test02
test03
tc_warn "test04 texi2dvi: texi --> DVI, not implemented"
tc_warn "test05 texi2dvi: texi --> PDF, not implemented"
test06
test07
test08
# note there are two tests in test09
test09
