#!/bin/sh
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# Test that clock_settime() sets errno=EINVAL if tp is outside the
# valid range for clock_id.
#
# This is tested implicitly via assertion 19 to the best of tester's
# knowledge.  Cannot find additional parameters for CLOCK_REALTIME
# in the POSIX specification

echo "Tested implicitly via assertion 19.  See output for status."
exit 0
