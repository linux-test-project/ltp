# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#

CPPFLAGS+=	-I../../../include
LDLIBS=		-lrt

TARGETS=	twoevtimers.test twoptimers.test

all: $(TARGETS)

%.test: %.o
	$(LINK.o) $< $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGETS)
