#! /bin/sh
#
# Test that the mlockall() checks that the flags argument is constructed from
# the bitwise-inclusive OR of one or more of the folowing symbolic constants,
# defined in sys/mman.h:
#	MCL_CURRENT
#	MCL_FUTURE
#
# This is tested implicitly via assertion 13.

echo "Tested implicitly via assertion 13."
exit 0
