# This doesn't work on m68k-motorola-sysv
# It also doesn't work on m88k-motorola-sysv3

istarget "m68k-motorola-sysv" && torture_compile_xfail="*"
istarget "m88k-motorola-sysv3"&& torture_compile_xfail="*"
istarget "i[3-6]86" && torture_execute_xfail="Os"
