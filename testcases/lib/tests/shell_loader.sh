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

. tst_loader.sh

tst_res TPASS "Shell loader works fine!"
case "$PWD" in
	/tmp/*)
		tst_res TPASS "We are running in temp directory in $PWD";;
	*)
		tst_res TFAIL "We are not running in temp directory but $PWD";;
esac
