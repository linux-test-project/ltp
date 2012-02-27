#! /bin/sh
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
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
SSSD_INIT_SCRIPT="/etc/init.d/sssd"

# Command to restart sssd daemon.
SSSD_RESTART_CMD=

# number of seconds to wait for another sssd test to complete
WAIT_COUNT=30

# running under systemd?
if command -v systemctl >/dev/null 2>&1; then
	HAVE_SYSTEMCTL=1
else
	HAVE_SYSTEMCTL=0
fi

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
			restart_sssd_daemon "return 1"
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
	is_root || exit 1

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

	SVCNAME=$(basename $SSSD_INIT_SCRIPT)
	if [ $HAVE_SYSTEMCTL == 1 ]; then
		for svc in "$SVCNAME" "sssd"; do
			if systemctl is-enabled $svc.service >/dev/null 2>&1
			then
				SSSD_RESTART_CMD="systemctl restart $svc.service"
				break
			fi
		done
	else
		for SSSD_INIT_SCRIPT in "$SSSD_INIT_SCRIPT" "/etc/init.d/sssd"
		do
			if [ -x "$SSSD_INIT_SCRIPT" ]; then
				SSSD_RESTART_CMD="$SSSD_INIT_SCRIPT restart"
				break
			fi
		done
	fi

	if [ -z "$SSSD_RESTART_CMD" ]; then
		tst_resm TBROK "Don't know how to restart $SVCNAME"
		cleanup 1
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
		tst_brkm TBROK NULL "$CONFIG_FILE not found!"
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

	tst_resm TINFO "restarting sssd daemon via $SSSD_RESTART_CMD"
	$SSSD_RESTART_CMD > /dev/null 2>&1
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
	echo -e "[sssd]\nconfig_file_version = 2" > $CONFIG_FILE
	echo -e "services = nss, pam\ndomains = LOCAL" >> $CONFIG_FILE
	echo -e "\n[nss]\n\n[pam]\n" >> $CONFIG_FILE
	echo -e "[domain/LOCAL]\nid_provider = local" >> $CONFIG_FILE
}

. cmdlib.sh

# determine sssd.conf can support override_gid?
setup
make_config_file
sed -i -e "/\[domain\/LOCAL\]/ a\override_gid = error" $CONFIG_FILE
# make sure config file is OK
sleep 1
$SSSD_RESTART_CMD > /dev/null 2>&1

if [ $? -ne 1 ]; then
	tst_resm TCONF "override_gid does not exist. Skipping all testcases"
	cleanup 0
fi
