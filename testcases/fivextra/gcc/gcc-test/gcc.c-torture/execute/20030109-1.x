istarget "powerpc" && torture_execute_xfail="*"
[ "$gcc_version" = "3.2" ] &&  istarget "i[3-6]86" && torture_execute_xfail="*"
