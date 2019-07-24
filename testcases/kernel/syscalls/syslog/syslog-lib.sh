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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

##################################################################

readonly MAILLOG=/var/log/maillog

# Signals to trap.
readonly TRAP_SIGS="1 2 3 6 11 15"

# configuration file for syslog or syslog-ng
CONFIG_FILE=""

# rsyslogd .conf specific args.
RSYSLOG_CONFIG=

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
	tst_require_root

	trap '	disable_traps
		tst_resm TBROK "Testing is terminating due to a signal"
		cleanup 1' $TRAP_SIGS || exit 1

	if [ "$SYSLOG_DAEMON" = "syslog" ]; then
		CONFIG_FILE="/etc/syslog.conf"
	elif [ "$SYSLOG_DAEMON" = "syslog-ng" ]; then
		CONFIG_FILE="/etc/syslog-ng/syslog-ng.conf"
	elif [ "$SYSLOG_DAEMON" = "rsyslog" ]; then
		CONFIG_FILE="/etc/rsyslog.conf"
		# To cope with systemd-journal, we are looking for either:
		#   $ModLoad imjournal
		#   module(load="imjournal"...)
		# in rsyslog config, and using those settings.
		if grep -qri '^[^#]*modload.*imjournal' /etc/rsyslog.conf /etc/rsyslog.d/; then
			RSYSLOG_CONFIG=$(grep -Ehoi "^[^#].*(imjournal|workdirectory).*" -r /etc/rsyslog.conf /etc/rsyslog.d/;
				echo '$imjournalRatelimitInterval 0'; \
				echo '$ImjournalIgnorePreviousMessages on';)
		elif grep -qri '^[^#]*module.*load="imjournal"' /etc/rsyslog.conf /etc/rsyslog.d/; then
			RSYSLOG_CONFIG=$(grep -Ehoi "^[^#].*workdirectory.*" -r /etc/rsyslog.conf /etc/rsyslog.d/; \
				echo 'module(load="imjournal"'; \
				echo '       StateFile="imjournal.state"'; \
				echo '       Ratelimit.Interval="0"'; \
				echo '       IgnorePreviousMessages="on")')
		else
			RSYSLOG_CONFIG=$(echo '$ModLoad imuxsock.so'; \
				grep -ho "^\$SystemLogSocketName .*" -r /etc/rsyslog.conf /etc/rsyslog.d/ | head -1)
		fi
	else
		tst_resm TCONF "Couldn't find syslogd, syslog-ng or rsyslogd"
		cleanup 32
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

	tst_resm TINFO "restarting syslog daemon"

	if [ -n "$SYSLOG_DAEMON" ]; then
		restart_daemon $SYSLOG_DAEMON
		if [ $? -eq 0 ]; then
			# XXX: this really shouldn't exist; if *syslogd isn't
			# ready once the restart directive has been issued,
			# then it needs to be fixed.
			sleep 2
		else
			#
			# Some distributions name the service syslog even if
			# the package is syslog-ng or rsyslog, so try it once
			# more with just syslog.
			#
			restart_daemon "syslog"

			if [ $? -ne 0 ]; then
				$cleanup_command
			fi
		fi
	fi
}

export TST_TOTAL=${TST_TOTAL:=1}
export TST_COUNT=1
export TCID=${TCID:="$(basename "$0")"}
. cmdlib.sh
