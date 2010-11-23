#! /bin/sh
#
#  Copyright (c) Linux Test Project, 2010 
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

##################################################################

readonly MAILLOG=/var/log/maillog

# Signals to trap.
readonly TRAP_SIGS="1 2 3 6 11 15"

# configuration file for syslog or syslog-ng
CONFIG_FILE=""

# rsyslogd .conf specific args.
RSYSLOG_CONFIG=

# Command for syslog daemon.
SYSLOG_COMMAND=

# Args to pass to $SYSLOG_COMMAND when restarting the daemon.
SYSLOG_RESTART_COMMAND_ARGS=

# number of seconds to wait for another syslog test to complete
WAIT_COUNT=60

cleanup()
{
	# Reentrant cleanup -> bad. Especially since rsyslogd on Fedora 13
	# seems to get stuck FOREVER when not running as root. Lame...
	disable_traps
	exit_code=$1

	# Restore the previous syslog daemon state.
	if [ -f "$CONFIG_FILE.ltpback" ]; then
		if mv "$CONFIG_FILE.ltpback" "$CONFIG_FILE"; then
			# Make sure that restart_syslog_daemon doesn't loop
			# back to cleanup again.
			restart_syslog_daemon "return 1"
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

	# Check to see if syslogd or syslog-ng exists
	if [ -e /sbin/syslogd ]; then
		CONFIG_FILE="/etc/syslog.conf"
		SYSLOG_INIT_SCRIPT="/etc/init.d/sysklogd"
	elif command -v syslog-ng >/dev/null 2>&1; then
		CONFIG_FILE="/etc/syslog-ng/syslog-ng.conf"
		SYSLOG_INIT_SCRIPT="/etc/init.d/syslog-ng"
	elif command -v rsyslogd >/dev/null 2>&1; then
		CONFIG_FILE="/etc/rsyslog.conf"
		SYSLOG_INIT_SCRIPT="/etc/init.d/rsyslog"
		RSYSLOG_CONFIG='$ModLoad imuxsock.so'
	#elif <detect-upstart-jobfile-here>; then
		# TODO: upstart
	else
		tst_resm TCONF "couldn't find syslogd, syslog-ng, or rsyslogd"
		cleanup	0
	fi

	if [ -z "$SYSLOG_INIT_SCRIPT" ]; then
		# TODO: upstart
		:
	else
		# Fallback to /etc/init.d/syslog if $SYSLOG_INIT_SCRIPT
		# doesn't exist.
		for SYSLOG_INIT_SCRIPT in "$SYSLOG_INIT_SCRIPT" "/etc/init.d/syslog"; do 
			if [ -x "$SYSLOG_INIT_SCRIPT" ]; then
				break
			fi
		done
		if [ ! -x "$SYSLOG_INIT_SCRIPT" ]; then
			tst_resm TBROK "$SYSLOG_INIT_SCRIPT - no such script"
			cleanup 1
		fi

		SYSLOG_COMMAND=$SYSLOG_INIT_SCRIPT
		SYSLOG_RESTART_COMMAND_ARGS=restart
	fi

	# Back up configuration file
	if [ -f "$CONFIG_FILE" ]; then
		# Pause if another LTP syslog test is running
		while [ -f "$CONFIG_FILE.ltpback" -a $WAIT_COUNT -gt 0 ]; do
			: $(( WAIT_COUNT -= 1 ))
			sleep 1
		done
		# Oops -- $CONFIG_FILE.ltpback is still there!
		if [ $WAIT_COUNT -eq 0 ]; then
			tst_resm TBROK "another syslog test is stuck"
			cleanup 1
		elif ! cp "$CONFIG_FILE" "$CONFIG_FILE.ltpback"; then
			tst_resm TBROK "failed to backup $CONFIG_FILE"
			cleanup 1
		fi
	else
		tst_resm TBROK "$CONFIG_FILE not found!"
	fi

}

disable_traps()
{
	trap - $TRAP_SIGS
}

# For most cases this isn't exotic. If you're running upstart however, you
# might have fun here :).
restart_syslog_daemon()
{
	# Default to running `cleanup 1' when dealing with error cases.
	if [ $# -eq 0 ]; then
		cleanup_command="cleanup 1"
	else
		cleanup_command=$1
	fi

	tst_resm TINFO "restarting syslog daemon via $SYSLOG_COMMAND $SYSLOG_RESTART_COMMAND_ARGS"

	if $SYSLOG_COMMAND $SYSLOG_RESTART_COMMAND_ARGS >/dev/null 2>&1; then
		# XXX: this really shouldn't exist; if *syslogd isn't ready
		# once the restart directive has been issued, then it needs to
		# be fixed.
		sleep 2
	else
		$cleanup_command
	fi
}

export TST_TOTAL=${TST_TOTAL:=1}
export TST_COUNT=1
export TCID=${TCID:="$(basename "$0")"}
. cmdlib.sh
