#!/usr/bin/awk -f

$1 == "PASS" {
	pass += $2;
	next
}
$1 == "FAIL" {
	fail += $2
}
END {
	print "PASS", pass;
	print "FAIL", fail;
}
