#!/bin/sh

status=0

tc_info "I:fetching a.example from ns2's initial configuration"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd +noauth \
	a.example. @10.53.0.2 any -p 5300 > dig.out.ns2.1 || ret=1
grep ";" dig.out.ns2.1 >> $stderr	# XXXDCL why is this here?
if [ $ret != 0 ]
then
echo "I:failed when fetching a.example from ns2's initial configuration" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching a.example from ns3's initial configuration"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd +noauth \
	a.example. @10.53.0.3 any -p 5300 > dig.out.ns3.1 || ret=1
grep ";" dig.out.ns3.1	>> $stderr # XXXDCL why is this here?
if [ $ret != 0 ]
then
echo "I:failed when fetching a.example from ns3's initial configuration" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:copying in new configurations for ns2 and ns3"
rm -f ns2/named.conf ns3/named.conf ns2/example.db
cp ns2/named2.conf ns2/named.conf
cp ns3/named2.conf ns3/named.conf
cp ns2/example2.db ns2/example.db

tc_info "I:sleeping two seconds then reloading ns2 and ns3 with rndc"
sleep 2
rndc_result=`$RNDC -c ./rndc.conf -s 10.53.0.2 -p 9953 reload 2>&1 | sed 's/^/I:ns2 /'`
#tc_info "$rndc_result"
rndc_result=`$RNDC -c ./rndc.conf -s 10.53.0.3 -p 9953 reload 2>&1 | sed 's/^/I:ns3 /'`
#tc_info "$rndc_result"

tc_info "I:sleeping five seconds."
sleep 5

tc_info "I:fetching a.example from ns2's 10.53.0.4, source address 10.53.0.4"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd +noauth \
	-b 10.53.0.4 a.example. @10.53.0.4 any -p 5300 > dig.out.ns4.2 \
	|| ret=1
grep ";" dig.out.ns4.2 >> $stderr	# XXXDCL why is this here?
if [ $ret != 0 ]
then
echo "I:failed when fetching a.example from ns2's 10.53.0.4, source address 10.53.0.4" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching a.example from ns2's 10.53.0.2, source address 10.53.0.2"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd +noauth \
	-b 10.53.0.2 a.example. @10.53.0.2 any -p 5300 > dig.out.ns2.2 \
	|| ret=1
grep ";" dig.out.ns2.2 >> $stderr	# XXXDCL why is this here?
if [ $ret != 0 ]
then
echo "I:failed when fetching a.example from ns2's 10.53.0.2, source address 10.53.0.2" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching a.example from ns3's 10.53.0.3, source address defaulted"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd +noauth \
	@10.53.0.3 a.example. any -p 5300 > dig.out.ns3.2 || ret=1
grep ";" dig.out.ns3.2 >> $stderr	# XXXDCL why is this here?
if [ $ret != 0 ]
then
echo "I:failed when fetching a.example from ns3's 10.53.0.3, source address defaulted" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:comparing ns3's initial a.example to one from reconfigured 10.53.0.2"
$PERL ../digcomp.pl dig.out.ns3.1 dig.out.ns2.2 >>$stdout 2>&1 || status=1

tc_info "I:comparing ns3's initial a.example to one from reconfigured 10.53.0.3"
$PERL ../digcomp.pl dig.out.ns3.1 dig.out.ns3.2 >>$stdout 2>&1 || status=1

tc_info "I:comparing ns2's initial a.example to one from reconfigured 10.53.0.4"
$PERL ../digcomp.pl dig.out.ns2.1 dig.out.ns4.2 >>$stdout 2>&1 || status=1

tc_info "I:comparing ns2's initial a.example to one from reconfigured 10.53.0.3"
tc_info "I:(should be different)"
if $PERL ../digcomp.pl dig.out.ns2.1 dig.out.ns3.2 >/dev/null
then
      echo "I:no differences found.  something's wrong." >> $stderr
      status=`expr $status + 1`
fi

tc_info "I:exit status: $status"
exit $status
