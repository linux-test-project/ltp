#! /bin/bash

echo "## Start Test"
date
if [ -d ./00 ] ; then
    echo -n "Clear old files..."
    /bin/rm -fr ./00 >& /dev/null
    echo "done"
fi
STARTT=`date +%s`
echo $STARTT
echo ""
echo "## Create files "
time ~/fs-bench/cr

echo ""
echo "## tar all "
MAXFILE=`tar cBf - 00 | tar tvBf - 2>&1 | tail -n 1 | awk '{print $6;}'| awk -F'/' '{print $4;}'`

echo ""
echo "## Change owner"
time chown -R $USER  ./00

echo ""
echo "## random access"
time ~/fs-bench/ra  $MAXFILE

echo ""
echo "## Change mode "
time chmod -R go+rw  ./00

echo ""
echo "## Random delete and create"
time ~/fs-bench/radc  $MAXFILE

echo ""
echo "## Change mode again"
time chmod -R go-rw  ./00

echo ""
echo "## Remove all files and directories"
time /bin/rm -fr ./00
echo ""
echo "## Finish test"
ENDT=`date +%s`
echo $ENDT
date

echo -n 'TOTAL(seconds): '
expr $ENDT - $STARTT
