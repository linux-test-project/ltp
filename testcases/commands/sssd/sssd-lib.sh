#!/bin/sh
#
#  Copyright (c) 2012 FUJITSU LIMITED
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
##################################################################

export TST_TOTAL=${TST_TOTAL:=1}
export TST_COUNT=1
export TCID=${TCID:="$(basename "$0")"}

if [ -z "$LTPTMP" -a -z "$TMPBASE" ]; then
	LTPTMP=/tmp
else
	LTPTMP=$TMPBASE
fi

if ! which sss_useradd >/dev/null 2>&1; then
	tst_brkm TCONF NULL \
		 "sss_useradd does not exist. Skipping all testcases."
	exit 0
fi

# Signals to trap.
readonly TRAP_SIGS="2 3 6 11 15"

CONFIG_FILE="/etc/sssd/sssd.conf"
NSS_CONFIG_FILE="/etc/nsswitch.conf"

# number of seconds to wait for another sssd test to complete
WAIT_COUNT=30

cleanup()
{
	disable_traps
	exit_code=$1

	# Restore the previous sssd daemon state.
	if [ -f "$CONFIG_FILE.ltpback" ]; then
		if mv "$CONFIG_FILE.ltpback" "$CONFIG_FILE"; then
			mv $NSS_CONFIG_FILE.ltpback $NSS_CONFIG_FILE
			# Make sure that restart_sssd_daemon doesn't loop
			# back to cleanup again.
			if [ $SSSD_STARTED -eq 1 ]; then
				stop_daemon sssd
			else
				restart_sssd_daemon "return 1"
			fi
			# Maintain any nonzero exit codes
			if [ $exit_code -ne $? ]; then
				exit_code=1
			fi
		else
			exit_code=1
		fi
	fi

	exit $exit_code
}

setup()
{
	tst_require_root

	trap '	disable_traps
		tst_resm TBROK "Testing is terminating due to a signal"
		cleanup 1' $TRAP_SIGS || exit 1

	# Check to see if sssd exists
	if [ ! -e /usr/sbin/sssd ]; then
		tst_resm TCONF "couldn't find sssd"
		cleanup	0
	fi

	# Check to see if nscd exists
	if [ ! -e /usr/sbin/nscd ]; then
		tst_resm TCONF "couldn't find nscd"
		cleanup	0
	fi

	# Back up configuration file
	if [ -f "$CONFIG_FILE" ]; then
		# Pause if another LTP sssd test is running
		while [ -f "$CONFIG_FILE.ltpback" -a $WAIT_COUNT -gt 0 ]; do
			: $(( WAIT_COUNT -= 1 ))
			sleep 1
		done
		# Oops -- $CONFIG_FILE.ltpback is still there!
		if [ $WAIT_COUNT -eq 0 ]; then
			tst_resm TBROK "another sssd test is stuck"
			cleanup 1
		elif ! cp "$CONFIG_FILE" "$CONFIG_FILE.ltpback"; then
			tst_resm TBROK "failed to backup $CONFIG_FILE"
			cleanup 1
		fi

		cp $NSS_CONFIG_FILE $NSS_CONFIG_FILE.ltpback
		grep "passwd:     files sss" $NSS_CONFIG_FILE > /dev/null
		if [ $? -ne 0 ]; then
			sed -i "s/passwd:     files/passwd:     files sss/" \
				$NSS_CONFIG_FILE
		fi
	else
		tst_resm TWARN "$CONFIG_FILE not found!"
		touch $CONFIG_FILE
	fi
	chmod 0600 $CONFIG_FILE
	if [ $? -ne 0 ]; then
		tst_brkm TBROK NULL "fail to modify the permission of $CONFIG_FILE"
	fi
}

disable_traps()
{
	trap - $TRAP_SIGS
}

restart_sssd_daemon()
{
	# Default to running `cleanup 1' when dealing with error cases.
	if [ $# -eq 0 ]; then
		cleanup_command="cleanup 1"
	else
		cleanup_command=$1
	fi

	tst_resm TINFO "restarting sssd daemon"
	restart_daemon sssd
	if [ $? -eq 0 ]; then
		# wait sssd restart success.
		sleep 1
	else
		$cleanup_command
	fi
}

# sssd.conf should contain:
# [sssd]
# config_file_version = 2
# services = nss, pam
# domains = LOCAL
#
#[nss]
#
#[pam]
#
#[domain/LOCAL]
#id_provider = local
make_config_file()
{
	printf "[sssd]\nconfig_file_version = 2\n" > $CONFIG_FILE
	printf "services = nss, pam\ndomains = LOCAL\n" >> $CONFIG_FILE
	printf "\n[nss]\n\n[pam]\n\n" >> $CONFIG_FILE
	printf "[domain/LOCAL]\nid_provider = local\n" >> $CONFIG_FILE
}

. cmdlib.sh

SSSD_STARTED=0
status_daemon sssd
if [ $? -ne 0 ]; then
	SSSD_STARTED=1
fi

# determine sssd.conf can support override_gid?
setup
make_config_file
sed -i -e "/\[domain\/LOCAL\]/ a\override_gid = error" $CONFIG_FILE
# make sure config file is OK
sleep 1
restart_daemon sssd

if [ $? -ne 1 ]; then
	tst_resm TCONF "override_gid does not exist. Skipping all testcases"
	cleanup 0
fi
