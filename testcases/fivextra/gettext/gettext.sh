#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	gettext.sh
#
# Description:	Test gettext package
#
# Author:	Andrew Pham (apham@us.ibm.com)	
#
# History:	Mar 13 2003 - Created - RR
#		Mar 19 2003 - Added more tests
#		Aug 28 2003 - Set TC_TOTAL up front.
#
#		05 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

################################################################################
# all commands to be tested:

commands=" xgettext msgmerge msgfmt msgunfmt ngettext \
	   gettext msgcmp msgcomm gettextize " 

REQUIRED="grep ls cat diff touch mkdir"
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=9

# Initialize output messages
ErrMsg="Failed: Not available."
ErrMsg1="Failed: Unexpected output.  Expected:"

################################################################################
# testcase functions
################################################################################
#
# Modify "make check" plural-1 test
#

function TC_xgettext()
{
	cat > $TCTMP/myprog.c <<-EOF
	#ifdef HAVE_CONFIG_H
	# include <config.h>
	#endif

	#include <stdlib.h>
	#include <stdio.h>
	#include <locale.h>

	/* Make sure we use the included libintl, not the system's one. */
	#define textdomain textdomain__
	#define bindtextdomain bindtextdomain__
	#define ngettext ngettext__
	#undef _LIBINTL_H
	#include "libgnuintl.h"

	int main (argc, argv)
	  	int argc;
	    	char *argv[];
		{
			int n = atoi (argv[1]);

			if (setlocale (LC_ALL, "") == NULL)
			return 1;

			textdomain ("cake");
			bindtextdomain ("cake", ".");
			printf (ngettext ("a piece of cake", \
				"%d pieces of cake", n), n);
			printf ("\n");
			return 0;
		}
	EOF
	
	xgettext -o $TCTMP/cake.pot --omit-header $TCTMP/myprog.c \
	>/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$ErrMsg" || return 1

	grep -q "%d pieces of cake" $TCTMP/cake.pot &&
        grep -q "a piece of cake" $TCTMP/cake.pot
	tc_pass_or_fail $?  "got: `cat $TCTMP/cake.pot`" \
		|| return 1
	return 0
}

function TC_msgmerge()
{
	cat > $TCTMP/fr.po <<-EOF
	msgid "a piece of cake"
	msgid_plural "%d pieces of cake"
	msgstr[0] "un morceau de gateau"
	msgstr[1] "%d morceaux de gateau"
	EOF
	
	msgmerge -q -o $TCTMP/fr.po.new $TCTMP/fr.po $TCTMP/cake.pot \
		>/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$ErrMsg" || return 1
	
	grep -q "un morceau de gateau" $TCTMP/fr.po.new &&
        grep -q "%d morceaux de gateau" $TCTMP/fr.po.new
	tc_pass_or_fail $?  "$ErrMsg1 `cat $TCTMP/fr.po` got: `cat $TCTMP/fr.po.new`" \
		|| return 1
	return 0
}

function TC_msgfmt()
{
	msgfmt -o $TCTMP/cake.mo $TCTMP/fr.po >/dev/null 2>>$stderr
	tc_fail_if_bad $? "$ErrMsg" || return 1
		
	msgunfmt $TCTMP/cake.mo -o $TCTMP/fr.po.tmp >/dev/null 2>>$stderr
	tc_fail_if_bad $? "$ErrMsg" || return 1
	
	diff $TCTMP/fr.po $TCTMP/fr.po.tmp >& /dev/null
	tc_pass_or_fail $?  "$ErrMsg1 `cat $TCTMP/fr.po` got: `cat $TCTMP/fr.po.tmp`" \
		|| return 1

	return 0
}

function TC_msgcmp()
{
	msgcmp $TCTMP/fr.po  $TCTMP/fr.po  >/dev/null 2>$stderr
	tc_fail_if_bad $? "$ErrMsg" || return 1
		
	echo "msgid \"red\"" >> $TCTMP/cake.pot
	echo 'msgstr "mau ddo"' >> $TCTMP/cake.pot

	msgcmp $TCTMP/fr.po  $TCTMP/cake.pot >&/dev/null
	if [ $? -ne 0 ]; then
		tc_pass_or_fail 0 "$ErrMsg1" || return 0
	else
		tc_pass_or_fail 1 "$ErrMsg1" || return 1
	fi
}

function TC_msgcomm()
{
	msgcomm -o $TCTMP/message.po $TCTMP/cake.pot  $TCTMP/fr.po  \
	>/dev/null 2>$stderr
	tc_fail_if_bad $? "$ErrMsg" || return 1
		
	cat $TCTMP/message.po | grep cake >&/dev/null
	tc_pass_or_fail $? "$ErrMsg1" || return 1

	return 0
}

function TC_msgunfmt()
	{
	tc_info "$TCNAME: See TC_msgfmt testcase."
	return 0
}

function TC_ngettext()
{	
	local RC=0
	
	My_Lang=$LANGUAGE
	My_all=$LC_ALL
	My_msg=$LC_MESSAGES
	My_La=$LANG

	LANGUAGE=
	LC_ALL=fr
	LC_MESSAGES=
	LANG=
	export LANGUAGE LC_ALL LC_MESSAGES LANG
	
	[ -d /tmp/gettext ] || mkdir /tmp/gettext
	[ -d /tmp/gettext/fr ] || mkdir /tmp/gettext/fr
	[ -d /mp/gettext/fr/LC_MESSAGES ] || mkdir /tmp/gettext/fr/LC_MESSAGES
	cp $TCTMP/cake.mo /tmp/gettext/fr/LC_MESSAGES/cake.mo
	cp $TCTMP/fr.po /tmp/gettext/fr/LC_MESSAGES/fr.po

	cd $LTPBIN
	./test_gettext 2 | grep pieces >&/dev/null
	tc_pass_or_fail $? "$ErrMsg" || RC=1
	
	LANGUAGE=$My_Lang
	LC_ALL=$My_all
	LC_MESSAGES=$My_msg
	LANG=$My_La
	export LANGUAGE LC_ALL LC_MESSAGES LANG
	
	return $RC
}

function TC_gettext()
{	
	local RC=0

	My_Lang=$LANGUAGE
	My_all=$LC_ALL
	My_msg=$LC_MESSAGES
	My_La=$LANG

	LANGUAGE=
	LC_ALL=fr
	LC_MESSAGES=
	LANG=
	export LANGUAGE LC_ALL LC_MESSAGES LANG
	
	cd $LTPBIN
	./test_gettext -1 | grep piece >&/dev/null
	tc_pass_or_fail $? "$ErrMsg" || RC=1
	
	LANGUAGE=$My_Lang
	LC_ALL=$My_all
	LC_MESSAGES=$My_msg
	LANG=$My_La
	export LANGUAGE LC_ALL LC_MESSAGES LANG
	
	rm -rf /tmp/gettext >&/dev/null
	return $RC
}

function TC_gettextize()
{	
	mkdir $TCTMP/getize
	touch $TCTMP/getize/configure.in $TCTMP/getize/configure.ac
	
	gettextize -c $TCTMP/getize > /dev/null 2>$stderr
	tc_fail_if_bad $? "$ErrMsg" || return 1

	ls $TCTMP/getize | grep NLS >& /dev/null
	tc_pass_or_fail $? "$ErrMsg1 BOUT-NLS in output.\
		got: `ls $TCTMP/getize`"
	return 
}
################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break $REQUIRED || exit 
E_value=0
for cmd in $commands
do
	tc_register $cmd 
	TC_$cmd || E_value=1 
done
exit $E_value
