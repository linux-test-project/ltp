#!/bin/sh

# Copyright (c) 2002, Intel Corporation. All rights reserved.
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.


# Upon successful completion, the pthread_barrier_wait( ) function shall return
# PTHREAD_BARRIER_SERIAL_THREAD for a single (arbitrary) thread synchronized at the
# barrier and zero for each of the other threads. Otherwise,
# an error number shall be returned to indicate the error.

# This is tested implicitly via assertion 1.

echo "Tested implicitly via assertion 1.  See output for status"
exit 0
