#!/bin/bash

# Positive tests for cron, that means these tests have to pass

iam=`whoami`

tvar=${MACHTYPE%-*}
tvar=${tvar#*-}

if [ "$tvar" = "redhat" -o "$tvar" = "redhat-linux" ]
then
	CRON_ALLOW="/etc/cron.allow"
else
	CRON_ALLOW="/var/spool/cron/allow"
fi


if [ $iam = "root" ]; then
	if [ $# -lt 1 ] ; then
		echo Either do not run this script as root or start it like
		echo "  $0 <user>"
		exit 1
	fi

	mv $CRON_ALLOW $CRON_ALLOW.old >/dev/null 2>&1
	su $1 -c "$0 $*"
        RC=$?
	mv $CRON_ALLOW.old $CRON_ALLOW >/dev/null 2>&1
	exit $RC
fi

function restorecrontab () {
	test -e /tmp/crontab-save-$iam && \
		crontab /tmp/crontab-save-$iam && \
		rm -f /tmp/crontab-save-$iam && \
		echo restored old crontab
}

echo Running as user $iam...

# Save current users crontab

test -e /tmp/crontab-save-$iam && rm -f /tmp/crontab-save-$iam

if [ "0" -lt `crontab -l 2>/dev/null | wc -l` ]; then

	echo 'crontab of this user exists -> creating backup'
	crontab -l | grep '^[^#]' > /tmp/crontab-save-$iam
fi


# Do tests

# 1. Add new job

rm -rf /tmp/crontest >/dev/null 2>&1
mkdir -p /tmp/crontest

cat > /tmp/crontest/testjob_cron01 << EOF
echo Testjob running
date
EOF

chmod 755 /tmp/crontest/testjob_cron01

crontab - << EOF
`date '+%M' | awk '{ print ($1+2)%60 " * * * * "
}'` /tmp/crontest/testjob_cron01 >> /tmp/crontest/output_cron01 2>&1
EOF

rc=$?

if [ $rc = "1" ]; then
	echo Error while adding crontab for user $iam
	restorecrontab
	exit 1
fi

echo new job added successfully

# 2. Wait for execution of job

echo 'sleeping for 130 seconds...'
sleep 130

rc=1
test -e /tmp/crontest/output_cron01 && rc=0

if [ $rc = "1" ]; then
	echo Job has not been executed
	restorecrontab
	exit 1
fi

grep "Testjob running" /tmp/crontest/output_cron01
rc=$?
if [ $rc = "1" ]; then
	echo Job has not produced valid output
	restorecrontab
fi

echo 'job has been executed :-)'
echo "testjob's output:"
echo

rm -rf /tmp/crontest

# 3. Delete crontab

crontab -r

echo removed crontab

# Restore old crontab file

restorecrontab

exit $rc
