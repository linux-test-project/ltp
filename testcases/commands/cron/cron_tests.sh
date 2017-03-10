#!/bin/sh

# Copyright (c) International Business Machines Corp., 2001
# Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Manoj Iyer <manjo@mail.utexas.edu>

TST_ID="cron_tests01"
TST_CNT=3
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_SETUP=setup
TST_CLEANUP=cleanup
. tst_test.sh

. daemonlib.sh

SYSLOG_STARTED=
CROND_STARTED=
LOGS=

grep_logs()
{
	local pattern="$1"
	local fail_msg="$2"
	local pass_msg="${3:-}"
	local n="${4:-10}"

	local lines=10
	local out=out.$$
	local err=err.$$
	local i ret

	for i in $(seq 1 $n); do
		if [ "$LOGS" ]; then
			tail -n $lines $LOGS | grep "$pattern" > $out 2> $err
		else
			journalctl -n $lines | grep "$pattern" > $out 2> $err
		fi
		ret=$?
		[ $ret -eq 0 ] && break
		sleep 1
	done

	if [ $ret -ne 0 ]; then
		tst_res TFAIL "$fail_msg: `cat $err`"
	else
		[ "$pass_msg" ] && tst_res TPASS "$pass_msg"
	fi
}

create_crontab()
{
	local crontab=cronjob.cron
	local script=$1
	local out=out.$$

	tst_res TINFO "creating crontab: $script"

	cat > $crontab <<EOF
* * * * * $script
EOF

	tst_res TINFO "installing crontab file"
	crontab $crontab > $out 2>&1
	if [ $? -ne 0 ]; then
		tst_brk TBROK "crontab: error while installing crontab file: `cat $out`"
		return 1
	fi
	return 0
}

remove_crontab()
{
	local out=out.$$
	tst_res TINFO "removing crontab file"
	crontab -r > $out 2>&1
	if [ $? -ne 0 ]; then
		tst_brk TBROK "crontab: error while removing crontab file `cat $out`"
		return 1
	fi
	return 0
}

create_hello_script()
{
	local script=$1

	cat > $script <<EOF
#!/bin/sh
echo "Hello Hell"
exit 0
EOF
	chmod +x $script
}

install_cron_test()
{
	local script=$PWD/cronprg.sh
	local cron_out=$PWD/tst1_cron.out
	local err=err.log
	local sleep_sec
	local ts_min1 ts_min2 fail

	tst_res TINFO "test install cron job"

	cat > $script <<EOF
#! /bin/sh
DATE=\`LC_ALL=C date\`
echo "Hello Hell today is \$DATE" > $cron_out 2>&1
exit 0
EOF
	chmod +x $script

	create_crontab $script 2> $err

	if [ $? -ne 0 ]; then
		tst_brk TBROK "crontab: error while creating cron job: `cat $err`"
	else
		tst_res TINFO "cron job installed successfully"
	fi

	grep_logs 'crontab.*REPLACE' \
		"cron activity not recorded" \
		"cron activity logged"

	# Sleep 3s after next minute since the loop below sleeps for 62 seconds, we
	# should start this 5-iteration loop closely following the start of a
	# minute.
	sleep_sec=$((123-`date +%-S`))
	tst_res TINFO "sleep for ${sleep_sec}s"
	sleep $sleep_sec

	# $script executed by the cron job will record the date and time into file
	# $cron_out. Get the minute recorded by the program, sleep to allow the cron
	# job to update file after 1m, and check if the value is advanced by 1.
	for i in $(seq 1 5); do
		tst_res TINFO "loop: $i: start"

		if [ ! -f "$cron_out" ]; then
			tst_res TFAIL "loop $i: file $cron_out doesn't exist"
			fail=1
			break
		fi

		ts_min1=$(awk '{print $8}' $cron_out | awk -F: '{printf("%d", $2);}')

		# wait for the cron job to update output file
		sleep 62

		# Check the time recorded in output file, this should be 1 minute ahead of
		# what was recored earlier.
		ts_min2=$(awk '{print $8}' $cron_out | awk -F: '{printf("%d", $2);}')

		if [ "x${ts_min1}" = "x" ] || [ "x${ts_min2}" = "x" ]; then
			tst_res TFAIL "loop $i: failed to get time: ts_min1: $ts_min1, ts_min2: $ts_min2"
			fail=1
			break
		fi

		[ $ts_min1 -eq 59 ] && ts_min1=0 || ts_min1=$(( $ts_min1+1 ))

		if [ $ts_min2 -ne $ts_min1 ]; then
			tst_res TFAIL "loop $i: failed to update every minute: expected: $ts_min1, received: $ts_min2"
			fail=1
			break
		fi
	done

	if [ ! "$fail" ]; then
		grep_logs "CMD ($script)" \
			"failed to install cron job installed and execute it" \
			"cron job installed and executed" 1
	fi

	remove_crontab
}

remove_cron_job_test()
{
	local script=$PWD/cronprg.sh

	tst_res TINFO "test remove cron job"

	create_hello_script $script
	create_crontab $script

	grep_logs 'crontab.*REPLACE' \
		"crontab activity not recorded"

	remove_crontab && grep_logs DELETE \
		"crontab activity not recorded" \
		"crontab removed the cron job" 1
}

list_cron_jobs_test()
{
	local script=$PWD/cronprg.sh
	local out=cron.out

	tst_res TINFO "test list installed cron jobs"

	create_hello_script $script
	create_crontab $script

	tst_res TINFO "crontab: listing cron jobs"
	crontab -l | grep "$script" > $out 2>&1 || \
		tst_brk TBROK "crontab failed while listing installed cron jobs: `cat $out`"

	remove_crontab

	crontab -l > $out 2>&1
	if [ $? -ne 0 ]; then
		grep -q "no crontab for" $out
		if [ $? -ne 0 ]; then
			tst_res TFAIL "crontab failed removing cron job: `cat $out`"
		else
			tst_res TPASS "crontab did not list any cron jobs"
		fi
	else
		tst_res TFAIL "crontab failed removing cron job: `cat $out`"
	fi
}

setup()
{
	if [ "$SYSLOG_DAEMON" ]; then
		status_daemon $SYSLOG_DAEMON
		if [ $? -ne 0 ]; then
			restart_daemon $SYSLOG_DAEMON
			SYSLOG_STARTED=1
		fi
	fi

	if [ "$CROND_DAEMON" ]; then
		status_daemon $CROND_DAEMON
		if [ $? -ne 0 ]; then
			restart_daemon $CROND_DAEMON
			CROND_STARTED=1
		fi
	fi

	for f in /var/log/syslog /var/log/messages /var/log/cron /var/log/cron.log; do
		[ -f "$f" ] && LOGS="$f $LOGS"
	done
}

cleanup()
{
	[ "$SYSLOG_STARTED" = "1" ] && stop_daemon $SYSLOG_DAEMON
	[ "$CROND_STARTED" = "1" ] && stop_daemon $CROND_DAEMON
}

do_test()
{
	case $1 in
	1) install_cron_test;;
	2) remove_cron_job_test;;
	3) list_cron_jobs_test;;
	esac
}

tst_run
