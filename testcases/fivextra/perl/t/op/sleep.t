#!./perl

print "1..1\n";

$x = sleep 3;
if ($x >= 2 && $x <= 10) {print "ok 1\n";} else {print "not ok 1 $x\n";}
