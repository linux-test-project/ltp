#!/bin/sh

DIGOPTS="+tcp +noadd +nosea +nostat +noquest +nocomm +nocmd"

status=0
ret=0
$DIG $DIGOPTS example. \
	@10.53.0.2 axfr -p 5300 > dig.out.ns2 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when do 10.53.0.2 example test" >> $stderr
fi
status=`expr $status + $ret`
grep ";" dig.out.ns2 >> $stdout

ret=0
$DIG $DIGOPTS example. \
	@10.53.0.3 axfr -p 5300 > dig.out.ns3 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when do 10.53.0.3 example test" >> $stderr
fi
status=`expr $status + $ret`
grep ";" dig.out.ns3 >> $stdout

$PERL ../digcomp.pl knowngood.dig.out dig.out.ns2 >$stdout 2>&1 || status=`expr $status + $ret`

$PERL ../digcomp.pl knowngood.dig.out dig.out.ns3 >$stdout 2>&1 || status=`expr $status + $ret`

ret=0
$DIG $DIGOPTS tsigzone. \
    	@10.53.0.2 axfr -y tsigzone.:1234abcd8765 -p 5300 \
	> dig.out.ns2 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when do 10.53.0.2 tsigzone test" >> $stderr
fi
status=`expr $status + $ret`
grep ";" dig.out.ns2 >> $stdout

ret=0
$DIG $DIGOPTS tsigzone. \
    	@10.53.0.3 axfr -y tsigzone.:1234abcd8765 -p 5300 \
	> dig.out.ns3 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when do 10.53.0.3 tsigzone test" >> $stderr
fi
status=`expr $status + $ret`
grep ";" dig.out.ns3 >> $stdout

$PERL ../digcomp.pl dig.out.ns2 dig.out.ns3 >$stdout 2>&1 || status=`expr $status + $ret`

tc_info "Exit status: $status"
exit $status
