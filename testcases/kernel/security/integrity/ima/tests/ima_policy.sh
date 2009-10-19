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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        ima_policy.sh
#
# Description:  This file tests replacing the default integrity measurement
#		policy.
#
# Author:       Mimi Zohar, zohar@ibm.vnet.ibm.com
#
# Return        - zero on success
#               - non zero on failure. return value from commands ($RC)
################################################################################
init()
{
	export TST_TOTAL=3
	export TCID="init"
	export TST_COUNT=0
	RC=0

	# verify using default policy
	IMA_POLICY=$IMA_DIR/policy
	if [ ! -f $IMA_POLICY ]; then
		tst_res TINFO $LTPTMP/imalog.$$ \
		 "$TCID: default policy already replaced"
		  RC=1
	fi

	VALID_POLICY=`dirname $0`\/..\/policy/measure.policy
	if [ ! -f $VALID_POLICY ]; then
		tst_res TINFO $LTPTMP/imalog.$$ \
		 "$TCID: missing $VALID_POLICY"
		  RC=1
	fi

	INVALID_POLICY=`dirname $0`\/..\/policy/measure.policy-invalid
	if [ ! -f $INVALID_POLICY ]; then
		tst_res TINFO $LTPTMP/imalog.$$ \
		 "$TCID: missing $INVALID_POLICY"
		  RC=1
	fi
	return $RC
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
				RC=1
				return $RC
			fi
		fi
	}
	done
}


# Function:     test01
# Description   - Verify invalid policy doesn't replace default policy.
test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	load_policy $INVALID_POLICY & p1=$!
	wait "$p1"; RC=$?
	if [ $RC -ne 0 ]; then
		RC=0
		tst_res TPASS $LTPTMP/imalog.$$ \
		 "$TCID: didn't load invalid policy"
	else
		RC=1
		tst_res TFAIL $LTPTMP/imalog.$$ \
		 "$TCID: loaded invalid policy"
	fi
	return $RC
}

# Function:     test02
# Description	- Verify policy file is opened sequentially, not concurrently
#		  and install new policy
test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	load_policy $VALID_POLICY & p1=$!  # forked process 1
	load_policy $VALID_POLICY & p2=$!  # forked process 2
	wait "$p1"; RC1=$?
	wait "$p2"; RC2=$?
	if [ $RC1 -eq 0 ] && [ $RC2 -eq 0 ]; then
		tst_res TFAIL $LTPTMP/imalog.$$ \
		 "$TCID: measurement policy opened concurrently"
	elif [ $RC1 -eq 0 ] || [ $RC2 -eq 0 ]; then
		RC=0
		tst_res TPASS $LTPTMP/imalog.$$ \
		 "$TCID: replaced default measurement policy"
	else
		tst_res TFAIL $LTPTMP/imalog.$$ \
		 "$TCID: problems opening measurement policy"
	fi
	return 0
}

# Function:     test03
# Description 	- Verify can't load another measurement policy.
test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	load_policy $INVALID_POLICY & p1=$!
	wait "$p1"; RC=$?
	if [ $RC -ne 0 ]; then
		RC=0
		tst_res TPASS $LTPTMP/imalog.$$ \
		 "$TCID: didn't replace valid policy"
	else
		RC=1
		tst_res TFAIL $LTPTMP/imalog.$$ "$TCID: replaced valid policy"
	fi
	return $RC
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, init, and test functions.
EXIT_VAL=0

. $(dirname "$0")/ima_setup.sh
setup || exit $?
init || exit $?
test01 || EXIT_VAL=$RC
test02 || EXIT_VAL=$RC
test03 || EXIT_VAL=$RC
exit $EXIT_VAL
