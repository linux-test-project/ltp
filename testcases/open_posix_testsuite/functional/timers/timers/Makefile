# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#

CFLAGS = -g -O2 -Wall -Werror

INCLUDE = -I../../../include

LDPATH = -L/lib/libposixtime.so

LIB = -lposixtime

all:	twoevtimers.test twoptimers.test

%.test : %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ $(LDPATH) $(LIB)

clean :
	rm *.test
