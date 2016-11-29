#!/bin/bash
########################################################
#
# CHANGE ACTIVITY
#
#    10/01/04  Kris Wilson    RHEL4 only allows super user
#                               to use crontab.
#                               to use crontab.
#    12/03/04  Marty Ridgeway Pull RHEl4 tests out from script
########################################################

iam=`whoami`

if [ $iam = "root" ]; then
	if [ $# -lt 1 ] ; then
		echo Either do not run this script as root or start it like
		echo "  $0 <user>"
		exit 1
	fi

	su $1 -c "$0 $*"
	exit $?
fi

#
# 1. root einen cronjob unterjubeln
#

finalrc=0


crontab -u root - << EOF
0 * * * * true
EOF

rc=$?

if [ $rc = "0" ]; then
	echo root has now an interesting cron job
	echo "crontab has a severe security breach (FAIL)"
	echo
	finalrc=1
else
	echo "Editing a crontab of another user failed successfully (PASS)"
	echo
fi


#
# 2. write some illegal crontabs
#

# Save crontab

#crontab -l > /dev/null 2> /dev/null
#if [ $? = "0" ]; then
#	echo Saving current crontab...
#	echo
#	crontab -l > /tmp/save-crontab-`whoami`
#	savedcrontab=1
#	crontab -r
#fi

#for line in `cat cron_illegal_cron_lines | grep '^[^#]' | sed -e 's/[ \t][ \t]*/_/g'` ; do
#	line=`echo $line | sed -e 's/_/ /g'`
	# echo Line: "$line"
#	cronconf=`echo "$line" | cut -f 1 -d '|'`
#	desc=`echo "$line" | cut -f 2 -d '|'`

#	echo "Test: $desc"
#	echo "$cronconf true" | crontab -
	# echo "$cronconf"
#	if [ $? = "0" ]; then
#		echo 'Test FAILED (or crontab returned wrong exit code)'
#		echo 'crontab -l:'
#		crontab -l
#		finalrc=1
#	fi
#	echo
#done


# Test whether cron uses setuid correctly

echo
echo setuid test
echo

tmpscript=cron_neg01_test
rm -rf $tmpscript.out >/dev/null 2>&1


cat > /tmp/$tmpscript << EOF
touch /root/halloichwarhier
sleep 1
cat /root/halloichwarhier ; echo "res:$?"
rm /root/halloichwarhier
EOF

chmod 755 /tmp/$tmpscript

#
cronline=`date '+%M' | awk '{print ($1+2)%60 " * * * * "}'`
(echo "$cronline /tmp/$tmpscript >> /tmp/$tmpscript.out 2>> /tmp/$tmpscript.out" ; \
 echo "$cronline /tmp/$tmpscript >> /$tmpscript.out 2>> /$tmpscript.out") \
 | crontab -

echo "sleeping 130 secs..."
sleep 130

echo
echo "Results:"
if [ "1" = `cat /tmp/$tmpscript.out | grep "res:0" | wc -l` ]; then
	echo "setuid test part 1 successfully failed (PASS)"
else
	echo "cron executed scripts have root privileges! (FAIL)"
	finalrc=1
fi

CODE=0
test -e /tmp/$tmpscript.out && CODE=1
if [ $CODE = "1" ]; then
	echo "setuid test part 2 successfully failed (PASS)"
else
	echo "cron writes script output with root privileges! (FAIL)"
	finalrc=1
fi
echo

rm /tmp/$tmpscript* >/dev/null 2>&1
crontab -r

# Restore crontab

if [ "$savedcrontab" = "1" ]; then
	echo "Restoring crontab..."
	cat /tmp/save-crontab-`whoami` | grep '^[^#]' | crontab -
	# rm -r /tmp/save-crontab-`whoami`
fi

exit $finalrc
