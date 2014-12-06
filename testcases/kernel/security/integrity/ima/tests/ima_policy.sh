#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (C) 2009 IBM Corporation                                         ##
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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
################################################################################
#
# File :        ima_policy.sh
#
# Description:  This file tests replacing the default integrity measurement
#		policy.
#
# Author:       Mimi Zohar, zohar@ibm.vnet.ibm.com
################################################################################
export TST_TOTAL=3
export TCID="ima_policy"

init()
{
	# verify using default policy
	IMA_POLICY=$IMA_DIR/policy
	if [ ! -f $IMA_POLICY ]; then
		tst_resm TINFO "default policy already replaced"
	fi

	VALID_POLICY=$LTPROOT/testcases/data/ima_policy/measure.policy
	if [ ! -f $VALID_POLICY ]; then
		tst_resm TINFO "missing $VALID_POLICY"
	fi

	INVALID_POLICY=$LTPROOT/testcases/data/ima_policy/measure.policy-invalid
	if [ ! -f $INVALID_POLICY ]; then
		tst_resm TINFO "missing $INVALID_POLICY"
	fi
}

load_policy()
{
	exec 2>/dev/null 4>$IMA_POLICY
	if [ $? -ne 0 ]; then
		exit 1
	fi

	cat $1 |
	while read line ; do
	{
		if [ "${line#\#}" = "${line}" ] ; then
			echo $line >&4 2> /dev/null
			if [ $? -ne 0 ]; then
				exec 4>&-
				return 1
			fi
		fi
	}
	done
}


# Function:     test01
# Description   - Verify invalid policy doesn't replace default policy.
test01()
{
	load_policy $INVALID_POLICY & p1=$!
	wait "$p1"
	if [ $? -ne 0 ]; then
		tst_resm TPASS "didn't load invalid policy"
	else
		tst_resm TFAIL "loaded invalid policy"
	fi
}

# Function:     test02
# Description	- Verify policy file is opened sequentially, not concurrently
#		  and install new policy
test02()
{
	load_policy $VALID_POLICY & p1=$!  # forked process 1
	load_policy $VALID_POLICY & p2=$!  # forked process 2
	wait "$p1"; RC1=$?
	wait "$p2"; RC2=$?
	if [ $RC1 -eq 0 ] && [ $RC2 -eq 0 ]; then
		tst_resm TFAIL "measurement policy opened concurrently"
	elif [ $RC1 -eq 0 ] || [ $RC2 -eq 0 ]; then
		tst_resm TPASS "replaced default measurement policy"
	else
		tst_resm TFAIL "problems opening measurement policy"
	fi
}

# Function:     test03
# Description 	- Verify can't load another measurement policy.
test03()
{
	load_policy $INVALID_POLICY & p1=$!
	wait "$p1"
	if [ $? -ne 0 ]; then
		tst_resm TPASS "didn't replace valid policy"
	else
		tst_resm TFAIL "replaced valid policy"
	fi
}

. ima_setup.sh

setup
TST_CLEANUP=cleanup

init
test01
test02
test03

tst_exit
