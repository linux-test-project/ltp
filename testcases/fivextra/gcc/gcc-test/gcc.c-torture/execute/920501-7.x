# This doesn't work on sparc's with -mflat.
istarget "sparc" && torture_execute_xfail="mflat"
