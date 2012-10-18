#!/bin/sh

# Copyright (c) 2002, Intel Corporation. All rights reserved.
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.


# A thread that has blocked on a barrier shall not prevent any unblocked
# thread that is eligible to use the same processing resources from
# eventually making forward progress in its execution.
# Eligibility for processing resources shall be determined by the scheduling policy.

# This is tested implicitly via assertion 1.

echo "Tested implicitly via assertion 1.  See output for status"
exit 0
