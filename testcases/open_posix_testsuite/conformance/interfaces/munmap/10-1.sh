#!/bin/sh

# Copyright (c) 2002, Intel Corporation. All rights reserved.
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.

# The munmap( ) function shall fail if:
# [EINVAL] The addr argument is not a multiple of the page size
# as returned by sysconf( ).

# This is tested implicitly via assertion 3.

echo "Tested implicitly via assertion 3.  See output for status"
exit 0
