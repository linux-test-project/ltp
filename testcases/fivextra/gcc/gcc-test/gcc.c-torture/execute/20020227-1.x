# This test reportedly fails on all 64-bit targets, see PR6221.  It's
# been observed to fail on at least mips-irix6, alpha, ia64, hppa64,
# sparc64/sparcv9 and mmix during April 2002.

istarget "sparc"  && additional_flags="-m64"

istarget "cross*powerpc64" && torture_execute_xfail="*"
istarget "alpha" && torture_execute_xfail="*"
istarget "mmix" && torture_execute_xfail="*"
istarget "sparcv9" && torture_execute_xfail="*"
istarget "mips" && torture_execute_xfail="*"
istarget "irix6" && torture_execute_xfail="*"

