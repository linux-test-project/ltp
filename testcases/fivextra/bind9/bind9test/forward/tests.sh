#!/bin/sh

root=10.53.0.1
hidden=10.53.0.2
f1=10.53.0.3
f2=10.53.0.4

status=0

tc_info "I:checking that a forward zone overrides global forwarders"
ret=0
$DIG txt.example1. txt @$hidden -p 5300 > dig.out.hidden || ret=1
$DIG txt.example1. txt @$f1 -p 5300 > dig.out.f1 || ret=1
$PERL ../digcomp.pl dig.out.hidden dig.out.f1 >>$stdout 2>&1 || ret=1
if [ $ret != 0 ]
then 
echo "I:failed when checking that a forward zone overrides global forwarders" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:checking that a forward first zone no forwarders recurses"
ret=0
$DIG txt.example2. txt @$root -p 5300 > dig.out.root || ret=1
$DIG txt.example2. txt @$f1 -p 5300 > dig.out.f1 || ret=1
$PERL ../digcomp.pl dig.out.root dig.out.f1 >>$stdout 2>&1 || ret=1
if [ $ret != 0 ] 
then 
echo "I:failed when checking that a forward first zone no forwarders recurses" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:checking that a forward only zone no forwarders fails"
ret=0
$DIG txt.example2. txt @$root -p 5300 > dig.out.root || ret=1
$DIG txt.example2. txt @$f1 -p 5300 > dig.out.f1 || ret=1
$PERL ../digcomp.pl dig.out.root dig.out.f1 >>$stdout 2>&1 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when checking that a forward only zone no forwarders fails" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:checking that global forwarders work"
ret=0
$DIG txt.example4. txt @$hidden -p 5300 > dig.out.hidden || ret=1
$DIG txt.example4. txt @$f1 -p 5300 > dig.out.f1 || ret=1
$PERL ../digcomp.pl dig.out.hidden dig.out.f1 >>$stdout 2>&1 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when checking that global forwarders work" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:checking that a forward zone works"
ret=0
$DIG txt.example1. txt @$hidden -p 5300 > dig.out.hidden || ret=1
$DIG txt.example1. txt @$f2 -p 5300 > dig.out.f2 || ret=1
$PERL ../digcomp.pl dig.out.hidden dig.out.f2 >>$stdout 2>&1 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when checking that a forward zone works" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:checking that forwarding doesn't spontaneously happen"
ret=0
$DIG txt.example2. txt @$root -p 5300 > dig.out.root || ret=1
$DIG txt.example2. txt @$f2 -p 5300 > dig.out.f2 || ret=1
$PERL ../digcomp.pl dig.out.root dig.out.f2 >>$stdout 2>&1 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when checking that forwarding doesn't spontaneously happen" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:checking that a forward zone with no specified policy works"
ret=0
$DIG txt.example3. txt @$hidden -p 5300 > dig.out.hidden || ret=1
$DIG txt.example3. txt @$f2 -p 5300 > dig.out.f2 || ret=1
$PERL ../digcomp.pl dig.out.hidden dig.out.f2 >>$stdout 2>&1 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when checking that a forward zone with no specified policy works" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:exit status: $status"
exit $status
