#!./perl

# $RCSfile: if.t,v $$Revision: 1.1 $$Date: 2005/03/22 17:51:42 $

print "1..2\n";

# first test to see if we can run the tests.

$x = 'test';
if ($x eq $x) { print "ok 1\n"; } else { print "not ok 1\n";}
if ($x ne $x) { print "not ok 2\n"; } else { print "ok 2\n";}
