# Copyright (c) 2004, Intel Corporation. All rights reserved.
# Created by:  adam.li@intel.com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.

CFLAGS := -Wall -I../../../include -O2

# If you want date for plotting, uncommnent this flag
# CFLAGS += -DPLOT_OUTPUT

LDFLAGS := -lpthread -lrt -lm

TARGETS := s-c stress1 stress2

all: $(TARGETS)

graph: pthread_cond_timedwait.png

pthread_cond_timedwait.png: s-c.c
	$(CC) $(CFLAGS) -DPLOT_OUTPUT -o s-c s-c.c $(LDFLAGS)
	./s-c > data.plot
	./do-plot data.plot
	rm -f data.plot

clean:
	rm -f $(TARGETS)
