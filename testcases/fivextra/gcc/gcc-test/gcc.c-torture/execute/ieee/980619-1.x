# This used to fail on ia32, with or without -ffloat-store.
# It works now, but some people think that's a fluke, so I'm
# keeping this around just in case.

istarget "i[3-6]86" && torture_execute_xfail="O3 O2 O1 Os"
