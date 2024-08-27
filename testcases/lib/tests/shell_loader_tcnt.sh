#!/bin/sh
#
# The script should be executed tcnt times and the iteration number should be in $1
#
# ---
# env
# {
#  "tcnt": 2
# }
# ---
#

. tst_loader.sh

tst_res TPASS "Iteration $1"
