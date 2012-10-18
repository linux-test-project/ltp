#!/bin/sh
# Copyright (c) 2003, Intel Corporation. All rights reserved.
# Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.

# mq_timedreceive() will fail with EINTR if mq_receive() is interrupted by a signal.

# This is tested implicitly via assertion 5.

echo "Tested implicitly via assertion 5.  See output for status"
exit 0
