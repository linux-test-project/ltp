#!/bin/sh

# Copyright (c) 2002, Intel Corporation. All rights reserved.
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.

# The mmap( ) function shall fail if:
# [EACCES] The fildes argument is not open for read,
# regardless of the protection specified,
# or fildes is not open for write and PROT_WRITE was specified for a
# MAP_SHARED type mapping.


# This is tested implicitly via assertion 6.

echo "Tested implicitly via assertion 6.  See output for status"
exit 0
