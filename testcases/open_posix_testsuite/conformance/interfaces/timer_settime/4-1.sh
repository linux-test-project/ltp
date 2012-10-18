#!/bin/sh
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.


# Test that if TIMER_ABSTIME is not set in flags, timer_settime()
# times relatively.
#
# This is tested implicitly via assertion 1, since a relative timer is used.

echo "Tested implicitly via assertion 1.  Set output for status."
exit 0
