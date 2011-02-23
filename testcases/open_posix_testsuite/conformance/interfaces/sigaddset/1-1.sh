#!/bin/sh
#   
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this 
# source tree.
#

#   Test that sigaddset() will add signo to the set signal set.
#   This test initializes an empty signal set first.
#   Test steps:
#   1)  Initialize an empty signal set.
#   2)  Add the SIGALRM signal to the empty signal set.
#   3)  Verify that SIGALRM is a member of the signal set.
# 

# Calling 1-core with a 0 parameter initializes to an
# empty set.
./1-core 0
