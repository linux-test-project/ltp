#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :       man.sh
#
# Description: This testcase tests the following 8 commands: 
#
#               
# /usr/bin/apropos 	/usr/bin/catman 	/usr/bin/man
# /usr/bin/mandb	/usr/bin/manpath 	/usr/bin/whatis
# /usr/bin/zsoelim 	/usr/sbin/accessdb
#
# Author:       Andrew Pham, apham@us.ibm.com
#
# History:      Mar 10 2003 - Created -Andrew Pham.
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		08 Mar 2004 (rcp) cleanup. BUG 5894
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source 

DATADIR=$LTPBIN/Mandir123

MANCACHE=/var/cache/man
MANSHARE=/usr/share/man

REQUIRED="grep chown mkdir cp rm"
COMMANDS="mandb man catman apropos whatis manpath zsoelim accessdb"

FIVCMD=fivcmd$$

################################################################################
# utility functions
################################################################################

function tc_local_setup()
{
	tc_exec_or_break $REQUIRED || return
	tc_root_or_break || return

	# put a manpage in place
	cp $DATADIR/tmanpage.150x.gz $MANSHARE/man1/$FIVCMD.1.gz
	chmod a+r $MANSHARE/man1/$FIVCMD.1.gz

	# save original index.db
	mv $MANCACHE/index.db $MANCACHE/index.db-$$

	# save original /etc/manpath.config
	mv /etc/manpath.config /etc/manpath.config-$$
	cp /etc/manpath.config-$$ /etc/manpath.config
}

function tc_local_cleanup()
{
	# restore saved files
	[ -f  /etc/manpath.config-$$ ] && mv /etc/manpath.config-$$ /etc/manpath.config
	[ -f $MANCACHE/index.db-$$ ] && mv $MANCACHE/index.db-$$ $MANCACHE/index.db

	# remove generated files
	rm -f $MANSHARE/man1/$FIVCMD.1.gz
	rm -f $MANCACHE/cat1/$FIVCMD.1.gz
}

################################################################################
# the testcase functions
################################################################################

function TC_man()
{
	local cmd="man $FIVCMD"
	echo "" | $cmd &>$stdout
	tc_fail_if_bad $? "\"$cmd\" failed" || return

	grep -q xxyyzz $stdout 2>$stderr
	tc_pass_or_fail $? "expected to see \"xxyyzz\" in stdout"
}

function TC_mandb()
{
	local cmd="mandb -c"
	$cmd >$stdout 2>/dev/null
	tc_fail_if_bad $? "\"$cmd\" failed" || return

	grep -q $FIVCMD $MANCACHE/index.db
	tc_pass_or_fail $? "expected to see \"$FIVCMD\" in \"$MANCACHE/index.db\""
}

function TC_apropos()
{
	local cmd="apropos xxyyzz"
	echo "" | $cmd >$stdout 2>$stderr
	tc_fail_if_bad $? "\"$cmd\" failed" || return

	grep $FIVCMD $stdout >& /dev/null
	tc_pass_or_fail $? "expected to see \"$FIVCMD\" in stdout" \
			"for command \"$cmd\"" || return
}

function TC_whatis()
{
	local cmd="whatis $FIVCMD"
	echo "" | $cmd >$stdout 2>$stderr
	tc_fail_if_bad $? "\"$cmd\" failed" || return

	grep xxyyzz $stdout >& /dev/null
	tc_pass_or_fail $? "expected to see \"xxyyzz\"  in stdout" \
			"for command \"$cmd\""
}

function TC_catman ()
{

	# sync up the man_db
	echo "" | man -u 1 $FIVCMD &>/dev/null
	
	echo "" | catman 1 >$stdout 2>$stderr
	tc_fail_if_bad $? "\"catman 1\" failed" || return
	
	ls -la $MANCACHE/cat1 > $stdout 2>$stderr
	grep -q "$FIVCMD.1.gz" $stdout
	tc_pass_or_fail $? "expected to see $FIVCMD.1.gz in directory $MANCACHE/cat1" \
		"$(ls -la $MANCACHE/cat1)"
}

function TC_manpath()
{
	export MANPATH=""

	cat > /etc/manpath.config <<-EOF
		MANPATH_MAP     /bin                    $MANSHARE
		MANPATH_MAP     /usr/bin                $MANSHARE
	EOF

	manpath >$stdout 2>$stderr
	tc_fail_if_bad $? "command \"manpath\" failed" || return

	grep -q share $stdout 
	tc_fail_if_bad $? "expected to see \"share\" in stdout" \
			"resulting from \"manpath\" command" || return

	manpath -g >&/dev/null
	rc=$?
	[ $rc -ne 0 ]
	tc_pass_or_fail $? "command \"manpath -g\" returned rc=0 but non-zero expected"
}	

function TC_zsoelim()
{
	zsoelim $DATADIR/tmanpage.150x.gz >$stdout 2>$stderr
	tc_fail_if_bad $? "zsoelim command failed" || return
	
	grep -q xxyyzz123 $stdout 
	tc_pass_or_fail $? "expected to see \"xxyyzz123\" in stdout"
}

function TC_accessdb()
{
	if [ -s $MANCACHE/index.db ]; then
		accessdb $MANCACHE/index.db >$stdout 2>$stderr
		tc_fail_if_bad $? "command \"accessdb $MANCACHE/index.db\" failed" || return

		grep -q $FIVCMD $stdout
		tc_fail_if_bad $? "expected to see \"$FIVCMD\" in stdout" \
				"resulting from command \"accessdb $MANCACHE/index.db\"" || return
	elif [ -s $MANSHARE/index.db ]; then
		accessdb $MANSHARE/index.db >$stdout 2>$stderr
		tc_fail_if_bad $? "command\"accessdb $MANSHARE/index.db\" failed" || return

		grep -q $FIVCMD $stdout
		tc_fail_if_bad $? "expected to see \"$FIVCMD\" in stdout" \
				"resulting from command \"accessdb $MANSHARE/index.db\"" || return
	else
		tc_break_if_bad 1 "NO db found to access" || return
	fi		
}

function TC_install_check()
{
	tc_executes $COMMANDS
	tc_pass_or_fail $? "man package not installed properly"
}

################################################################################
# main
################################################################################
set install_check $COMMANDS
TST_TOTAL=$#
tc_setup

tc_root_or_break || exit
for cmd in install_check $COMMANDS 
do
	tc_register $cmd
	TC_$cmd || exit
done
