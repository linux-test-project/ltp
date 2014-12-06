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
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
################################################################################
#
# File :        ima_measurements.sh
#
# Description:  This file verifies measurements are added to the measurement
# 		list based on policy.
#
# Author:       Mimi Zohar, zohar@ibm.vnet.ibm.com
################################################################################
export TST_TOTAL=3
export TCID="ima_measurements"

init()
{
	tst_check_cmds sha1sum

	# verify using default policy
	if [ ! -f "$IMA_DIR/policy" ]; then
		tst_resm TINFO "not using default policy"
	fi
}

# Function:     test01
# Description   - Verify reading a file causes a new measurement to
#		  be added to the IMA measurement list.
test01()
{
	# Create file test.txt
	cat > test.txt <<-EOF
	$(date) - this is a test file
	EOF
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "Unable to create test file"
	fi

	# Calculating the sha1sum of test.txt should add
	# the measurement to the measurement list.
	# (Assumes SHA1 IMA measurements.)
	hash=$(sha1sum "test.txt" | sed 's/  -//')

	# Check if the file is measured
	# (i.e. contained in the ascii measurement list.)
	cat /sys/kernel/security/ima/ascii_runtime_measurements > measurements
	sleep 1
	$(grep $hash measurements > /dev/null)
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "TPM ascii measurement list does not contain sha1sum"
	else
		tst_resm TPASS "TPM ascii measurement list contains sha1sum"
	fi
}

# Function:     test02
# Description	- Verify modifying, then reading, a file causes a new
# 		  measurement to be added to the IMA measurement list.
test02()
{
	# Modify test.txt
	echo $($date) - file modified >> test.txt

	# Calculating the sha1sum of test.txt should add
	# the new measurement to the measurement list
	hash=$(sha1sum test.txt | sed 's/  -//')

	# Check if the new measurement exists
	cat /sys/kernel/security/ima/ascii_runtime_measurements > measurements
	$(grep $hash measurements > /dev/null)

	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Modified file not measured"
		tst_resm TINFO "iversion not supported; or not mounted with iversion"
	else
		tst_resm TPASS "Modified file measured"
	fi
}

# Function:     test03
# Description 	- Verify files are measured based on policy
#		(Default policy does not measure user files.)
test03()
{
	# create file user-test.txt
	mkdir -m 0700 user
	chown nobody.nobody user
	cd user
	hash=0

	# As user nobody, create and cat the new file
	# (The LTP tests assumes existence of 'nobody'.)
	sudo -n -u nobody sh -c "echo $(date) - create test.txt > ./test.txt;
				 cat ./test.txt > /dev/null"

	# Calculating the hash will add the measurement to the measurement
	# list, so only calc the hash value after getting the measurement
	# list.
	cat /sys/kernel/security/ima/ascii_runtime_measurements > measurements
	hash=$(sha1sum test.txt | sed 's/  -//')
	cd - >/dev/null

	# Check if the file is measured
	grep $hash measurements > /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TPASS "user file test.txt not measured"
	else
		tst_resm TFAIL "user file test.txt measured"
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
