# This test fails under hpux 9.X and 10.X because HUGE_VAL is DBL_MAX
# instead of +Infinity.
istarget "hpux" && torture_execute_xfail="*"

