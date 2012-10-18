#!/bin/sh
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.

#  Test pthread_create() fails and an error number is returned if:

#  - [EAGAIN]	The system lacked the resources to create another thread,
#               	or the system-imposed limit on the total number of threads
#		in a process {PTHREAD_THREADS_MAX} would be exceeded.
#  - [EINVAL]	The value specified by 'attr' is invalid.
#  - [EPERM]	The caller does not have the appropriate permission to set
#		the required scheduling parameters or scheduling policy.

# This is tested implicitly via assertion 10. Note: only EINVAL is tested because the other
# 2 conditions are hard to emmulate.

echo "Tested implicitly via assertion 10.  See output for status"
exit 0
