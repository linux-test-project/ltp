#!/bin/sh

# Copyright (c) 2002, Intel Corporation. All rights reserved.
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.



#The pthread_rwlock_tryrdlock( ) function shall fail if:
#[EBUSY] The read-write lock could not be acquired for reading because a writer holds
#the lock or a writer with the appropriate priority was blocked on it.

# This is tested implicitly via assertion 1.

echo "Tested implicitly via assertion 1.  See output for status"
exit 0
