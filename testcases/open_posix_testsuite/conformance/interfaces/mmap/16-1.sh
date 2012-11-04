#!/bin/sh

# Copyright (c) 2002, Intel Corporation. All rights reserved.
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.

# Upon successful completion, the mmap( ) function shall return
# the address at which the mapping was placed (pa);
# otherwise, it shall return a value of MAP_FAILED and set errno to indicate the
# error. The symbol MAP_FAILED is defined in the <sys/mman.h> header. No successful return
# from mmap( ) shall return the value MAP_FAILED.


# This is tested implicitly via assertion 1.

echo "Tested implicitly via assertion 1.  See output for status"
exit 0
