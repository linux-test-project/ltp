#!/bin/sh
#
# ---
# doc
#
# [Description]
#
# This is a simple shell test loader example.
# ---
#
# ---
# env
# {
#  "needs_tmpdir": true
# }
# ---
#
# ---
# inv
#
# This is an invalid block that breaks the test.
# ---

. tst_loader.sh

tst_res TPASS "This should pass!"
