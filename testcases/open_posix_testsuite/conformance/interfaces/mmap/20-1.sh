#!/bin/sh

# Copyright (c) 2002, Intel Corporation. All rights reserved.
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.

# The mmap( ) function shall fail if:
# [EINVAL] The addr argument (if MAP_FIXED was specified) or off is not a multiple of
# the page size as returned by sysconf( ), or is considered invalid by the
# implementation.

# This is tested implicitly via assertion 11.

echo "Tested implicitly via assertion 11.  See output for status"
exit 0
