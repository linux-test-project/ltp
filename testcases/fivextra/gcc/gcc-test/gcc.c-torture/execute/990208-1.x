# Doesn't work at -O3 because of ifcvt.c optimizations which
# cause the 2 inlined labels to be at the same location.

istarget "ia64" && torture_compile_xfail="O3" 
