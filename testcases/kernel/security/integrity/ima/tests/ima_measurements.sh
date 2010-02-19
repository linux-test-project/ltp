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
# File :        ima_measurements.sh
#
# Description:  This file verifies measurements are added to the measurement
# 		list based on policy.
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

	exists sha1sum

	# verify using default policy
	if [ ! -f "$IMA_DIR/policy" ]; then
		tst_res TINFO $LTPTMP/imalog.$$ \
		 "$TCID: not using default policy"
	fi
	return $RC
}

# Function:     test01
# Description   - Verify reading a file causes a new measurement to
#		  be added to the IMA measurement list.
test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Create file test.txt
	cat > $LTPIMA/test.txt <<-EOF || RC=$?
	`date` - this is a test file
	EOF
	if [ $RC -ne 0 ]; then
		tst_res TBROK $LTPTMP/imalog.$$ "" \
		 "$TCID: Unable to create test file"
		return $RC
	fi

	# Calculating the sha1sum of $LTPTMP/test.txt should add
	# the measurement to the measurement list.
	# (Assumes SHA1 IMA measurements.)
	hash=$(sha1sum < "$LTPIMA/test.txt" | sed 's/  -//')

	# Check if the file is measured
	# (i.e. contained in the ascii measurement list.)
	cat /sys/kernel/security/ima/ascii_runtime_measurements > \
		 $LTPIMA/measurements
	sleep 1
	`grep $hash $LTPIMA/measurements > /dev/null` || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/imalog.$$ \
		 "$TCID: TPM ascii measurement list does not contain sha1sum"
		return $RC
	else
		tst_res TPASS $LTPTMP/imalog.$$ \
		 "$TCID: TPM ascii measurement list contains sha1sum"
	fi
	return $RC
}

# Function:     test02
# Description	- Verify modifying, then reading, a file causes a new
# 		  measurement to be added to the IMA measurement list.
test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Modify test.txt
	echo `$date` - file modified >> $LTPIMA/test.txt || RC=$?

	# Calculating the sha1sum of $LTPTMP/test.txt should add
	# the new measurement to the measurement list
	hash=`cat $LTPIMA/test.txt | sha1sum | sed 's/  -//'`

	# Check if the new measurement exists
	cat /sys/kernel/security/ima/ascii_runtime_measurements > \
		$LTPIMA/measurements
	`grep $hash $LTPIMA/measurements > /dev/null` || RC=$?

	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/imalog.$$ \
		 "$TCID: Modified file not measured"
		tst_res TINFO $LTPTMP/imalog.$$ \
		 "$TCID: iversion not supported; or not mounted with iversion"
		return $RC
	else
		tst_res TPASS $LTPTMP/imalog.$$ \
		 "$TCID: Modified file measured"
	fi
	return $RC
}

# Function:     test03
# Description 	- Verify files are measured based on policy
#		(Default policy does not measure user files.)
test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# create file user-test.txt
	mkdir -m 0700 $LTPIMA/user
	chown nobody.nobody $LTPIMA/user
	cd $LTPIMA/user
	hash=0

	# As user nobody, create and cat the new file
	# (The LTP tests assumes existence of 'nobody'.)
	sudo -n -u nobody sh -c "echo `date` - create test.txt > ./test.txt;
				 cat ./test.txt > /dev/null"

	# Calculating the hash will add the measurement to the measurement
	# list, so only calc the hash value after getting the measurement
	# list.
	cat /sys/kernel/security/ima/ascii_runtime_measurements > \
		 $LTPIMA/measurements
	hash=`cat ./test.txt | sha1sum | sed 's/  -//'`
	cd - >/dev/null

	# Check if the file is measured
	grep $hash $LTPIMA/measurements > /dev/null || RC=$?
	if [ $RC -ne 0 ]; then
		RC=0
		tst_res TPASS $LTPTMP/imalog.$$ \
		 "$TCID: user file test.txt not measured"
	else
		RC=1
		tst_res TFAIL $LTPTMP/imalog.$$ \
		 "$TCID: user file test.txt measured"
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
RC=0
EXIT_VAL=0

. $(dirname "$0")/ima_setup.sh
setup || exit $?
init || exit $?
test01 || EXIT_VAL=$RC
test02 || EXIT_VAL=$RC
test03 || EXIT_VAL=$RC
exit $EXIT_VAL
