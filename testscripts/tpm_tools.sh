#!/bin/sh
#
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

# test_tpm_tools.sh - Run the tpm-tools test suite.

# Must be root to run the testsuite
#if [ $UID != 0 ]
#then
#	echo "FAILED: Must be root to execute this script"
#	exit 1
#fi

# Set the LTPROOT directory
cd `dirname $0`
export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]
then
	cd ..
	export LTPROOT=${PWD}
fi

# Set the PATH to include testcase/bin
# and the sbin directories
export LTPBIN=$LTPROOT/testcases/bin
export PATH=$PATH:/sbin:/usr/sbin:/usr/local/sbin:$LTPBIN

# We will store the logfiles in $LTPROOT/results, so make sure
# it exists.
if [ ! -d $LTPROOT/results ]
then
	mkdir $LTPROOT/results
fi

# Check for programs/daemons/groups...
USER="`whoami`"
RC=0
if [ -z "$LTPTMP" ] && [ -z "$TMPBASE" ]
then
	LTPTMP="/tmp"
else
	LTPTMP="$TMPBASE"
fi
export TPM_TMPFILE="$LTPTMP/tst_tpm.err"
rm -f $TPM_TMPFILE 1>/dev/null 2>&1

# Check for the expect command
rm -f $TPM_TMPFILE 1>/dev/null 2>&1
which expect 1>$TPM_TMPFILE 2>&1
if [ $? -ne 0 ]
then
	echo "The 'expect' command is not available.  Be sure the expect package has been installed properly"
	RC=1
fi

# Check for TrouSerS and that it is running
rm -f $TPM_TMPFILE 1>/dev/null 2>&1
which tcsd 1>$TPM_TMPFILE 2>&1
if [ $? -ne 0 ]
then
	echo "The trousers TSS stack is not available.  Be sure trousers has been installed properly"
	if [ -f $TPM_TMPFILE ]
	then
		cat $TPM_TMPFILE
	fi
	RC=1
else
	rm -f $TPM_TMPFILE 1>/dev/null 2>&1
	ps -ef 1>$TPM_TMPFILE
	grep tcsd $TPM_TMPFILE 1>/dev/null
	if [ $? -ne 0 ]
	then
		echo "The trousers TSS stack is not running.  Be sure to start the trousers daemon (tcsd)"
		RC=1
	fi
fi

# Make the opencryptoki testing optional
if [ -z "$TPM_NOPKCS11" ]
then

	# Check for the pkcs11 group and that the user is a member of it
	grep -q ^pkcs11: /etc/group
	if [ $? -ne 0 ]
	then
		echo "The 'pkcs11' group does not exist.  Be sure openCryptoki has been installed properly"
		RC=1
	fi

	groups | grep pkcs11 1>/dev/null 2>&1
	if [ $? -ne 0 ]
	then
		echo "User '$USER' is not a member of the 'pkcs11' group"
		RC=1
	fi

	# Check for openCryptoki and that it is running
	#   Additionally, delete the user's TPM token data store.
	rm -f $TPM_TMPFILE 1>/dev/null 2>&1
	which pkcsslotd 1>$TPM_TMPFILE 2>&1
	if [ $? -ne 0 ]
	then
		echo "The openCryptoki PKCS#11 slot daemon is not available.  Be sure openCryptoki has been installed properly"
		if [ -f $TPM_TMPFILE ]
		then
			cat $TPM_TMPFILE
		fi
		RC=1
	else
		rm -f $TPM_TMPFILE 1>/dev/null 2>&1
		ps -ef 1>$TPM_TMPFILE
		grep pkcsslotd $TPM_TMPFILE 1>/dev/null
		if [ $? -ne 0 ]
		then
			echo "The openCryptoki PKCS#11 slot daemon is not running.  Be sure to start the openCryptoki slot daemon (pkcsslotd)"
			RC=1
		else
			P11DIR=`which pkcsslotd | sed s-/sbin/pkcsslotd--`
			if [ "$P11DIR" = "/usr" ]
			then
				P11DIR=""
			fi

			grep libpkcs11_tpm $P11DIR/var/lib/opencryptoki/pk_config_data 1>/dev/null
			if [ $? -ne 0 ]
			then
				echo "The TPM PKCS#11 token is not active.  Be sure openCryptoki has been installed properly"
				RC=1
			fi
			if [ -d $P11DIR/var/lib/opencryptoki/tpm/$USER ]
			then
				rm -rf $P11DIR/var/lib/opencryptoki/tpm/$USER
			fi
		fi
	fi
fi

if [ $RC -ne 0 ]
then
	exit 1
fi

# Set known password values
export OWN_PWD="OWN PWD"
export NEW_OWN_PWD="NEW OWN PWD"
export SRK_PWD="SRK PWD"
export NEW_SRK_PWD="NEW SRK PWD"
export P11_SO_PWD="P11 SO PWD"
export NEW_P11_SO_PWD="NEW P11 SO PWD"
export P11_USER_PWD="P11 USER PWD"
export NEW_P11_USER_PWD="NEW P11 USER PWD"

echo "Running the tpm-tools testsuite..."
$LTPROOT/bin/ltp-pan -d 5 -S -a $LTPROOT/results/tpm_tools -n ltp-tpm-tools -l $LTPROOT/results/tpm_tools.logfile -o $LTPROOT/results/tpm_tools.outfile -p -f $LTPROOT/runtest/tpm_tools

echo "Done."
exit 0
