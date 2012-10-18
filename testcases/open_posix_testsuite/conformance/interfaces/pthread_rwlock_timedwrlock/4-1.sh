#!/bin/sh

# Copyright (c) 2002, Intel Corporation. All rights reserved.
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.

# Test pthread_rwlock_timedwrlock()

#	If the Timers option is not supported, the timeout shall be based on the
#	system clock as returned by the time( ) function. The resolution of the timeout
#	shall be the resolution of the clock on which it is based. The timespec data type
#	is defined in the <time.h> header.

# This is tested implicitly via assertion 1.

echo "Tested implicitly via assertion 1.  See output for status"
exit 0
