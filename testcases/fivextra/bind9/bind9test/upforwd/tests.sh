#!/bin/sh

# ns1 = stealth master
# ns2 = slave with update forwarding disabled; not currently used
# ns3 = slave with update forwarding enabled


if test -f /etc/resolv.conf
then
    mv /etc/resolv.conf /etc/resolv.conf.orig
fi

echo "nameserver 10.10.10.10" > /etc/resolv.conf

status=0

tc_info "I:fetching master copy of zone before update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.1 axfr -p 5300 > dig.out.ns1 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching master copy of zone before update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching slave 1 copy of zone before update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.2 axfr -p 5300 > dig.out.ns2 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching slave 1 copy of zone before update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching slave 2 copy of zone before update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.3 axfr -p 5300 > dig.out.ns3 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching slave 2 copy of zone before update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:comparing pre-update copies to known good data"
$PERL ../digcomp.pl knowngood.before dig.out.ns1 >>$stdout 2>&1 || status=`expr $status + $ret`
$PERL ../digcomp.pl knowngood.before dig.out.ns2 >>$stdout 2>&1 || status=`expr $status + $ret`
$PERL ../digcomp.pl knowngood.before dig.out.ns3 >>$stdout 2>&1 || status=`expr $status + $ret`

tc_info "I:updating zone (signed)"
ret=0
$NSUPDATE -y update.example:c3Ryb25nIGVub3VnaCBmb3IgYSBtYW4gYnV0IG1hZGUgZm9yIGEgd29tYW4K -- - <<EOF || ret=1
server 10.53.0.3 5300
update add updated.example. 600 A 10.10.10.1
update add updated.example. 600 TXT Foo
send
EOF

if [ $ret != 0 ]
then
echo "I:failed when updating zone (signed)" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:sleeping 3 seconds for server to incorporate changes"
sleep 3

tc_info "I:fetching master copy of zone after update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.1 axfr -p 5300 > dig.out.ns1 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching master copy of zone after update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching slave 1 copy of zone after update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.2 axfr -p 5300 > dig.out.ns2 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching slave 1 copy of zone after update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching slave 2 copy of zone after update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.3 axfr -p 5300 > dig.out.ns3 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching slave 2 copy of zone after update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:comparing post-update copies to known good data"
$PERL ../digcomp.pl knowngood.after1 dig.out.ns1 >>$stdout 2>&1 || status=`expr $status + $ret`
$PERL ../digcomp.pl knowngood.after1 dig.out.ns2 >>$stdout 2>&1 || status=`expr $status + $ret`
$PERL ../digcomp.pl knowngood.after1 dig.out.ns3 >>$stdout 2>&1 || status=`expr $status + $ret`


tc_info "I:updating zone (unsigned)"
ret=0
$NSUPDATE -- - <<EOF || ret=1
server 10.53.0.3 5300
update add unsigned.example. 600 A 10.10.10.1
update add unsigned.example. 600 TXT Foo
send
EOF
if [ $ret != 0 ]
then
echo "I:failed when updating zone (unsigned)" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:sleeping 3 seconds for server to incorporate changes"
sleep 3

tc_info "I:fetching master copy of zone after update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.1 axfr -p 5300 > dig.out.ns1 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching master copy of zone after update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching slave 1 copy of zone after update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.2 axfr -p 5300 > dig.out.ns2 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching slave 1 copy of zone after update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:fetching slave 2 copy of zone after update"
ret=0
$DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd example.\
	@10.53.0.3 axfr -p 5300 > dig.out.ns3 || ret=1
if [ $ret != 0 ]
then
echo "I:failed when fetching slave 2 copy of zone after update" >> $stderr
fi
status=`expr $status + $ret`

tc_info "I:comparing post-update copies to known good data"
$PERL ../digcomp.pl knowngood.after2 dig.out.ns1 >>$stdout 2>&1 || status=`expr $status + $ret`
$PERL ../digcomp.pl knowngood.after2 dig.out.ns2 >>$stdout 2>&1 || status=`expr $status + $ret`
$PERL ../digcomp.pl knowngood.after2 dig.out.ns3 >>$stdout 2>&1 || status=`expr $status + $ret`

rm -f /etc/resolv.conf
if test -f /etc/resolv.conf.orig
then
    mv /etc/resolv.conf.orig  /etc/resolv.conf
fi


tc_info "I:exit status: $status"
exit $status
