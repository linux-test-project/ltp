# This doesn't work on MIPS Irix.
# See http://gcc.gnu.org/ml/gcc-patches/2002-04/msg00473.html

istarget "mips" && torture_execute_xfail="*"
istarget "sgi" && torture_execute_xfail="*"
istarget "irix6" && torture_execute_xfail="*"
