BEGIN {
	FS="@";
	n = 0;
	printf ("local -i status\n");
	printf ("TST_TOTAL=0\n");
}

$0 ~ /^#/  { next; }

NF == 3 {
	printf ("tc_register \"test%02d %s\"\n", ++n, $0);
	printf ("let TST_TOTAL+=1\n");
	printf ("status=`echo '%s' | { grep -e '%s' > /dev/null 2>&1; echo $?; cat >/dev/null; }`\n",$3, $2);
	printf ("[ $status -eq %s ]\n", $1);
	printf ("tc_pass_or_fail $? \"expected status=%s, actual status=$status\"\n", $1);
	print ("");
}
