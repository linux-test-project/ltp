# The problem on Alpha at -O3 is that when dd is inlined, we have
# division by a constant, which gets converted to multiplication
# by a large constant, which gets turned into an induction variable.
# The problem is that the multiplication was unsigned SImode, and the
# induction variable is DImode, and we lose the truncation that
# should have happened.
istarget "alpha" && torture_compile_xfail="O3"
