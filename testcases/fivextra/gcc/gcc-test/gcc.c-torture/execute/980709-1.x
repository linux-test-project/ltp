# pow() is not available on m6811/m6812 target, this test will not link.
istarget "m6811" && continue
istarget "m6812" && continue

# XFAIL this test for AIX using -msoft-float.
# This test calls the system libm.a function pow.
# A false failure is reported if -msoft-float is used.
# AIX expects the the parameters to be passed in fp regs. 

istarget "powerpc*aix" && torture_compile_xfail="msoft-float" 
istarget "rs6000*aix" && torture_compile_xfail="msoft-float"
