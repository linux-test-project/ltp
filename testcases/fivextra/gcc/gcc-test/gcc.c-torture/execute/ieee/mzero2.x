# freebsd sets up the fpu with a different precision control which causes
# this test to "fail".
istarget "i[3-6]86*freebsd" && torture_execute_xfail="*"
