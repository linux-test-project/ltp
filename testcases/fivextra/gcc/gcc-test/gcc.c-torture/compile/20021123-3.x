istarget "powerpc"  &&  torture_compile_xfail="O1 O2 O3 Os"
istarget "i[3-6]86" && [ "$gcc_version" = "3.2" ]  &&  torture_compile_xfail="O1 O2 O3 Os"
