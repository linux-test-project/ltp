#!/bin/sh
#
# eval_onescript.sh SCRIPT
#
# Evaluates one test program, and helps it out by doing a bit of setup
# for it.  It does this by sourcing some configuration files for it
# first, and if it exited without calling FINISHED, call it.
#
# Modified for LTP
# Robb Romans <robb@austin.ibm.com>
# July 18, 2003

source TESTCONF.sh
source eval_tools.sh

source ./$1

# We shouldn't get here...
# If we do, it means they didn't exit properly.
# So we will.
tst_resm TBROK "$1 FAILED to execute"
STOPAGENT      # Just in case.
FINISHED
