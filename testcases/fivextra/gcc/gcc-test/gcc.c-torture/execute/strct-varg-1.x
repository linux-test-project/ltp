# This doesn't work on mn10200

istarget "mn10200" && torture_execute_xfail="*"
istarget "h8300" && torture_execute_xfail "*"
