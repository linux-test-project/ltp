#!/bin/sh

iam=`whoami`

tvar=${MACHTYPE%-*}
tvar=${tvar#*-}

setup_path() {
	# Support paths used by older distributions of RHEL or SLES.
	export CRON_DENY="$(man crontab 2>/dev/null | grep -m 1 -o '/[\/a-z.]*deny$' || echo "/etc/cron.deny")"
	export CRON_ALLOW="$(man crontab 2>/dev/null | grep -m 1 -o '/[\/a-z.]*allow$' || echo "/etc/cron.allow")"
}

setup_path

