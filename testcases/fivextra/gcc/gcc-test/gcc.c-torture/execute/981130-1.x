# This test is known to fail on targets that use the instruction scheduler
# at optimisation levels of 2 or more because the alias analysis is confused
# by the reassignment of a variable structure to a fixed structure.  The 
# failure could be suppressed by preventing instruction scheduling:
#
# set additional_flags "-fno-schedule-insns2";
#
# but this would disguise the fact that there is a problem.  Instead we use
# we generate an xfail result and explain that it is alias analysis that
# is at fault.
istarget "m32r" && torture_compile_xfail="O2 O1 O0 Os"
