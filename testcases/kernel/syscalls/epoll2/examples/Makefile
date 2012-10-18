#
#  epoll ( Efficent event polling implementation )
#  Copyright (C) 2001,...,2002	 Davide Libenzi
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  Davide Libenzi <davidel@xmailserver.org>
#
#

OUTDIR = ../bin
SRCDIR = .
INCLUDE = -I- -I. -I../include -I/usr/src/linux/include
LIBS = -L../lib -lepoll -lpcl


ifeq ($(EPOLLCFG), release)
CFLAGS = -O3 $(INCLUDE) -DUNIX
else
CFLAGS = -g $(INCLUDE) -DUNIX -D_DEBUG
endif

SOURCES = $(SRCDIR)/epoll-test.c

MKDEP = mkdep -f


all: blaster httpd pipetest

.depend: $(SOURCES)
	$(MKDEP) $(CFLAGS) $(SOURCES)

blaster: $(SOURCES)
	$(CC) $(CFLAGS) -DHTTP_BLASTER -o $(OUTDIR)/epoll-blaster $(SOURCES) $(LIBS)

httpd: $(SOURCES)
	$(CC) $(CFLAGS) -DDPHTTPD -o $(OUTDIR)/epoll-httpd $(SOURCES) $(LIBS)

pipetest: $(SOURCES)
	$(CC) $(CFLAGS) -DPIPETESTER -o $(OUTDIR)/epoll-pipetest $(SOURCES) $(LIBS)

distclean: clean
	@rm -f .depend

clean:
	@rm -f *~ $(OUTDIR)/epoll-blaster $(OUTDIR)/epoll-httpd $(OUTDIR)/epoll-pipetest

#include .depend

